#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_mapi.h"
#include "cvi_mode.h"
#include "cvi_media_init.h"
#include "cvi_modeinner.h"
#include "ui_common.h"


//#error "Unimplemented"

//UsbMenu MODE
int32_t CVI_MODE_OpenUsbMenuMode(void)
{
    int32_t s32Ret = 0;

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    CVI_LOGD("CVI_MODE_OpenUsbMenuMode\n");

    //screen init if not screen not init
    s32Ret = CVI_MEDIA_DispInit(true);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Disp init");

    // s32Ret = CVI_UIAPP_Start();
    // MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"CVI_UIAPP_Start");

    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = CVI_WORK_MODE_USBMENU;
    CVI_EVENTHUB_Publish(&stEvent);

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_USBMENU;

    return 0;
}

int32_t CVI_MODE_CloseUsbMenuMode(void)
{
    int32_t s32Ret = 0;

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_BUTT;

    s32Ret = CVI_MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_DispDeInit fail");

    return 0;
}


int32_t CVI_MODEMNG_UsbMenuModeMsgProc(CVI_MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
    //int32_t s32Ret = 0;
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return CVI_PROCESS_MSG_RESULTE_OK;
    }

    /** check parameters */
    MODEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstModeMngCtx->bInProgress);

    CVI_STATE_S* pstStateAttr = (CVI_STATE_S*)pvArg;
    CVI_LOGD("UsbMenu current mode(%s)\n\n", pstStateAttr->name);
    CVI_LOGD("UsbMenu mode will process message topic(%x) \n\n", pstMsg->topic);

    return CVI_PROCESS_MSG_RESULTE_OK;
}

int32_t CVI_MODEMNG_UsbMenuStatesInit(const CVI_STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static CVI_STATE_S stUsbMenuState =
    {
        CVI_WORK_MODE_USBMENU,
        MODEEMNG_STATE_USBMENU,
        CVI_MODE_OpenUsbMenuMode,
        CVI_MODE_CloseUsbMenuMode,
        CVI_MODEMNG_UsbMenuModeMsgProc,
        NULL
    };
    stUsbMenuState.argv = &stUsbMenuState;
    s32Ret = CVI_HFSM_AddState(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stUsbMenuState,
                              (CVI_STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "HFSM add UsbMenu state");
    CVI_LOGD("CVI_MODEMNG_UsbMenuStatesInit\n");

    return s32Ret;
}

/** deinit Uvc mode */
int32_t CVI_MODEMNG_UsbMenuStatesDeinit(void)
{
    int32_t s32Ret = 0;
    CVI_MODE_CloseUsbMenuMode();
    return s32Ret;
}