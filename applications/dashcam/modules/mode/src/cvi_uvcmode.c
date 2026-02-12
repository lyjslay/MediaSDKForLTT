#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_mapi.h"
#include "cvi_mode.h"
#include "cvi_media_init.h"
#include "cvi_usbctrl.h"
#include "cvi_modeinner.h"
#include "cvi_uvc.h"

static int32_t CVI_MODE_SetUvcCfg(void)
{
    int32_t z = 0;
    int32_t ret = 0;
    CVI_PARAM_USB_MODE_S UsbModeParam = {0};
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_MEDIA_SPEC_S params = {0};
    CVI_UVC_CFG_S UvcCfg = {0};

    CVI_PARAM_GetUsbParam(&UsbModeParam);
    CVI_PARAM_GetMediaMode(UsbModeParam.UvcParam.VcapId, &params);

    memcpy(&UvcCfg.attr, &UsbModeParam.UvcParam.UvcCfg, sizeof(CVI_UVC_CFG_ATTR_S));
    /* get vprochdl */
    for (z = 0; z < MAX_VPROC_CNT; z++) {
        if ((MediaParams->SysHandle.vproc[z] != NULL) && 
            (UsbModeParam.UvcParam.VprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(MediaParams->SysHandle.vproc[z]))) {
            UvcCfg.stDataSource.VprocHdl = MediaParams->SysHandle.vproc[z];
            break;
        }
    }
    UvcCfg.stDataSource.VprocChnId = UsbModeParam.UvcParam.VprocChnId;
    /* set usb uvc configure */
    
    ret = CVI_USB_SetUvcCfg(&UvcCfg);
    if(ret != 0) {
        CVI_LOGE("CVI_USB_SetUvcCfg faile !\n");
        return -1;
    }

    return 0;
}

//UVC MODE
int32_t CVI_MODEMNG_OpenUvcMode(void)
{
    int32_t s32Ret = 0;

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    s32Ret = CVI_MEDIA_VbInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Vb init");

    s32Ret = CVI_MEDIA_VideoInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Video init");

    //screen init if not screen not init
    s32Ret = CVI_MEDIA_DispInit(true);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Disp init");

    s32Ret = CVI_MODE_SetUvcCfg();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MODE_SetUvcCfg fail");

    CVI_USB_CFG_S stUsbCfg = {0};
    CVI_USB_GetUvcCfg(&stUsbCfg);

    if (UVC_Init(&stUsbCfg.stUvcCfg.attr.stDevCap, &stUsbCfg.stUvcCfg.stDataSource, &stUsbCfg.stUvcCfg.attr.stBufferCfg) != 0) {
        printf("UVC_Init Failed !");
        return -1;
    }

    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = CVI_WORK_MODE_UVC;
    CVI_EVENTHUB_Publish(&stEvent);

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_UVC;

    return 0;
}

int32_t CVI_MODEMNG_CloseUvcMode(void)
{
    int32_t s32Ret = 0;

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_BUTT;

    s32Ret = UVC_Deinit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "UVC_Deinit fail");

    s32Ret = CVI_MEDIA_VcapDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_VcapDeInit fail");

    s32Ret = CVI_MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_DispDeInit fail");

    s32Ret = CVI_MEDIA_VbDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_VbDeInit fail");

    return 0;
}

int32_t CVI_MODEMNG_UvceModeMsgProc(CVI_MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
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
    CVI_LOGD("uvc current mode(%s)\n\n", pstStateAttr->name);
    CVI_LOGD("UVC mode will process message topic(%x) \n\n", pstMsg->topic);
    switch (pstMsg->topic)
    {
        case CVI_EVENT_MODEMNG_UVC_MODE_START:
        {
            CVI_USB_CFG_S stUsbCfg = {0};
            CVI_USB_GetUvcCfg(&stUsbCfg);
            char uvc_devname[32] = "/dev/video0";
            if (UVC_Start(stUsbCfg.stUvcCfg.attr.szDevPath) != 0) {
                CVI_LOGE("UVC_Start Failed, patch = %s, try open = %s!", stUsbCfg.stUvcCfg.attr.szDevPath, uvc_devname);
                if (UVC_Start(uvc_devname) != 0) {
                    CVI_LOGE("UVC_Start Failed, patch = %s!", uvc_devname);
                }
            }
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_MODEMNG_UVC_MODE_STOP:
        {
            if (UVC_Stop() != 0) {
                CVI_LOGE("UVC_Stop Failed !");
            }
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        default:
            return CVI_PROCESS_MSG_UNHANDLER;
            break;
    }

    return CVI_PROCESS_MSG_RESULTE_OK;
}

int32_t CVI_MODEMNG_UvcStatesInit(const CVI_STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static CVI_STATE_S stUvcState =
    {
        CVI_WORK_MODE_UVC,
        MODEEMNG_STATE_UVC,
        CVI_MODEMNG_OpenUvcMode,
        CVI_MODEMNG_CloseUvcMode,
        CVI_MODEMNG_UvceModeMsgProc,
        NULL
    };
    stUvcState.argv = &stUvcState;
    s32Ret = CVI_HFSM_AddState(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stUvcState,
                              (CVI_STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "HFSM add NormalRec state");

    return s32Ret;
}

/** deinit Uvc mode */
int32_t CVI_MODEMNG_UvcStatesDeinit(void)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_CloseUvcMode();
    return s32Ret;
}