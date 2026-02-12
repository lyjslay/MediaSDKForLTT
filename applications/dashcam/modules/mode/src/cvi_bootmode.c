#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_mode.h"
#include "cvi_media_init.h"
#include "cvi_modeinner.h"
#include "ui_common.h"


//#error "Unimplemented"

//BootFirst MODE
int32_t CVI_MODE_OpenBootFirstMode(void)
{
    int32_t s32Ret = 0;

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    CVI_LOGD("#########################CVI_MODE_OpenBootFirstMode#############################\n");

    //screen init if not screen not init
    s32Ret = CVI_MEDIA_DispInit(true);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Disp init");

    // s32Ret = CVI_UIAPP_Start();
    // MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"CVI_UIAPP_Start");

    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = CVI_WORK_MODE_BOOT;
    CVI_EVENTHUB_Publish(&stEvent);

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_BOOT;

    return 0;
}

int32_t CVI_MODE_CloseBootFirstMode(void)
{
    int32_t s32Ret = 0;

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_BUTT;

    s32Ret = CVI_MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_DispDeInit fail");

    /** close movie ui */
    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODECLOSE;
    stEvent.arg1 = CVI_WORK_MODE_MOVIE;
    s32Ret = CVI_EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Publish");


    return 0;
}

int32_t CVI_MODEMNG_BootFirstModeMsgProc(CVI_MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return CVI_PROCESS_MSG_RESULTE_OK;
    }

    /** check parameters */
    MODEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstModeMngCtx->bInProgress);

    CVI_STATE_S* pstStateAttr = (CVI_STATE_S*)pvArg;
    CVI_LOGD("BootFirst current mode(%s)\n\n", pstStateAttr->name);
    CVI_LOGD("BootFirst mode will process none message, transfer to modebase, topic(%x) \n\n", pstMsg->topic);

    return CVI_PROCESS_MSG_UNHANDLER;
}

int32_t CVI_MODEMNG_BootFirstStatesInit(const CVI_STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static CVI_STATE_S stBootFirstState =
    {
        CVI_WORK_MODE_BOOT,
        MODEEMNG_STATE_BOOTFIRST,
        CVI_MODE_OpenBootFirstMode,
        CVI_MODE_CloseBootFirstMode,
        CVI_MODEMNG_BootFirstModeMsgProc,
        NULL
    };
    stBootFirstState.argv = &stBootFirstState;
    s32Ret = CVI_HFSM_AddState(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stBootFirstState,
                              (CVI_STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "HFSM add BootFirst state");
    CVI_LOGD("CVI_MODEMNG_BootFirstStatesInit\n");

    return s32Ret;
}

/** deinit Uvc mode */
int32_t CVI_MODEMNG_BootFirstStatesDeinit(void)
{
    int32_t s32Ret = 0;
    CVI_MODE_CloseBootFirstMode();
    return s32Ret;
}