#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_mode.h"
#include "cvi_media_init.h"
#include "cvi_modeinner.h"
#include "cvi_usb_storage.h"

//storage MODE
int32_t CVI_MODE_OpenStorageMode(void)
{
    int32_t s32Ret = 0;

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    s32Ret = CVI_MEDIA_VbInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Vb init");

    //screen init if not screen not init
    s32Ret = CVI_MEDIA_DispInit(true);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Disp init");

    CVI_USB_STORAGE_CFG_S StpstCfg = {0};
    CVI_USB_GetStorageCfg(&StpstCfg);

    if (USB_STORAGE_Init(StpstCfg.szDevPath) != 0) {
        printf("USB_STORAGE_Init Failed !");
        return -1;
    }

    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = CVI_WORK_MODE_STORAGE;
    CVI_EVENTHUB_Publish(&stEvent);

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_STORAGE;

    return 0;
}

int32_t CVI_MODE_CloseStorageMode(void)
{
    int32_t s32Ret = 0;

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_BUTT;

    s32Ret = USB_STORAGE_Deinit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "USB_STORAGE_Deinit fail");

    s32Ret = CVI_MEDIA_VcapDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_VcapDeInit fail");

    s32Ret = CVI_MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_DispDeInit fail");

    s32Ret = CVI_MEDIA_VbDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_VbDeInit fail");

    return 0;
}

int32_t CVI_MODEMNG_StorageModeMsgProc(CVI_MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
    // int32_t s32Ret = 0;
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return CVI_PROCESS_MSG_RESULTE_OK;
    }

    /** check parameters */
    MODEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstModeMngCtx->bInProgress);

    CVI_STATE_S* pstStateAttr = (CVI_STATE_S*)pvArg;
    CVI_LOGD("storage current mode(%s)\n\n", pstStateAttr->name);
    CVI_LOGD("storage mode will process message topic(%x) \n\n", pstMsg->topic);
    switch (pstMsg->topic)
    {
        case CVI_EVENT_MODEMNG_STORAGE_MODE_PREPAREDEV:
        {
            CVI_USB_STORAGE_CFG_S StpstCfg = {0};
            CVI_USB_GetStorageCfg(&StpstCfg);
            if (USB_STORAGE_PrepareDev(&StpstCfg) != 0) {
                CVI_LOGE("USB_STORAGE_PrepareDev");
            }
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        default:
            return CVI_PROCESS_MSG_UNHANDLER;
            break;
    }

    return CVI_PROCESS_MSG_RESULTE_OK;
}

int32_t CVI_MODEMNG_StorageStatesInit(const CVI_STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static CVI_STATE_S stStorageState =
    {
        CVI_WORK_MODE_STORAGE,
        MODEEMNG_STATE_USB_STORAGE,
        CVI_MODE_OpenStorageMode,
        CVI_MODE_CloseStorageMode,
        CVI_MODEMNG_StorageModeMsgProc,
        NULL
    };
    stStorageState.argv = &stStorageState;
    s32Ret = CVI_HFSM_AddState(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stStorageState,
                              (CVI_STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "HFSM add NormalRec state");

    return s32Ret;
}

/** deinit Uvc mode */
int32_t CVI_MODEMNG_StorageStatesDeinit(void)
{
    int32_t s32Ret = 0;
    CVI_MODE_CloseStorageMode();
    return s32Ret;
}