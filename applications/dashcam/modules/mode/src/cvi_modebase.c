#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_modeinner.h"
#ifdef SERVICES_LIVEVIEW_ON
#include "cvi_volmng.h"
#endif
#include "cvi_usbctrl.h"
#include "cvi_timedtask.h"
#ifdef CONFIG_SCREEN_ON
#include "cvi_hal_screen.h"
#endif
#ifdef SERVICES_SPEECH_ON
#include "cvi_speechmng.h"
#endif

#ifdef SERVICES_SPEECH_ON
int32_t CVI_MODEMNG_StartEventSpeech()
{
    int32_t s32Ret = 0;
    s32Ret = CVI_SPEECHMNG_StartSpeech();
    return s32Ret;
}

int32_t CVI_MODEMNG_StopEventSpeech()
{
    int32_t s32Ret = 0;
    s32Ret = CVI_SPEECHMNG_StopSpeech();
    return s32Ret;
}
#endif

static int32_t CVI_MODEMNG_BaseModeProcessMessage(CVI_MESSAGE_S *pstMsg, void* pvArg, uint32_t *pStateID)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return CVI_PROCESS_MSG_RESULTE_OK;
    }

    /** check parameters */
    MODEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstModeMngCtx->bInProgress);

    CVI_STATE_S* pstStateAttr = (CVI_STATE_S*)pvArg;
    CVI_LOGD("base mode(%s)\n\n", pstStateAttr->name);
    CVI_LOGD(" will process message topic(%x) \n\n", pstMsg->topic);
    switch (pstMsg->topic)
    {
        case CVI_EVENT_MODEMNG_MODESWITCH:
            {
                if (pstMsg->arg1 < 0 || pstMsg->arg1 > CVI_WORK_MODE_BUTT) {
                    CVI_LOGE("Switch mode is illegal! modeid(%d)\n", pstMsg->arg1);
                }
                *pStateID = pstMsg->arg1;
                /*if (CVI_WORK_MODE_LAPSE == pstMsg->arg1) {
                    CVI_MODEMNG_SetMediaLapseTime(pstMsg->arg2);
                }*/
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_POWEROFF:
            {
                /*TODO*/
                CVI_LOGI("try to process message (%x) to poweroff device\n\n", pstMsg->topic);
                pstModeMngCtx->bSysPowerOff = true;
                pstModeMngCtx->stModemngCfg.pfnExitCB(CVI_MODEMNG_EXIT_MODE_POWEROFF);
                /** no need publish result event and reset g_bModeMngInProgress to false */
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_SCREEN_DORMANT:
            {
                #ifdef CONFIG_SCREEN_ON
                if (pstMsg->arg1 == true) {
                    CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_0, CVI_HAL_SCREEN_STATE_OFF);
                } else {
                    CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_0, CVI_HAL_SCREEN_STATE_ON);
                }
                #endif
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_CARD_FORMAT:
            {
                CVI_EVENT_S stEvent = {0};
                stEvent.topic = CVI_EVENT_MODEMNG_CARD_FORMATING;
                CVI_EVENTHUB_Publish(&stEvent);
                s32Ret = CVI_MODEMNG_Format((char *)pstMsg->aszPayload);
                if(s32Ret != 0){
                    CVI_LOGE("CVI_MODEMNG_Format Faild!, s32Ret: %d\n", s32Ret);
                    CVI_MODEMNG_SetCardState(CVI_CARD_STATE_UNAVAILABLE);
                    stEvent.topic = CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED;
                    CVI_EVENTHUB_Publish(&stEvent);
                }else{
                    CVI_LOGD("CVI_MODEMNG_Format Succes!\n");
                    CVI_MODEMNG_SetCardState(CVI_CARD_STATE_FORMATED);
                    CVI_MODEMNG_InitFilemng();
                    stEvent.topic = CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED;
                    CVI_EVENTHUB_Publish(&stEvent);
                }
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_SWITCH_LIVEVIEW:
            {
                CVI_MODEMNG_LiveViewSwitch(pstMsg->arg1);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            {
                if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_REMOVE)) {
                    CVI_MODEMNG_MonitorStatusNotify(pstMsg);
                    s32Ret = CVI_FILEMNG_SetDiskState(false);
                    MODEMNG_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "CVI_FILEMNG_SetDiskState");
                    return CVI_PROCESS_MSG_RESULTE_OK;
                }
            }
            break;
        case CVI_EVENT_STORAGEMNG_DEV_CONNECTING:
            {
                if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_CHECKING))
                    CVI_MODEMNG_MonitorStatusNotify(pstMsg);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        case CVI_EVENT_STORAGEMNG_DEV_ERROR:
            {
                if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_ERROR))
                    CVI_MODEMNG_MonitorStatusNotify(pstMsg);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        case CVI_EVENT_STORAGEMNG_MOUNT_FAILED:
            {
                if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_MOUNT_FAILED))
                    CVI_MODEMNG_MonitorStatusNotify(pstMsg);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        case CVI_EVENT_STORAGEMNG_MOUNT_READ_ONLY:
            {
                if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_READ_ONLY))
                    CVI_MODEMNG_MonitorStatusNotify(pstMsg);

                return CVI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        case CVI_EVENT_STORAGEMNG_FS_CHECKING:
            {
                if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_CHECKING))
                    CVI_MODEMNG_MonitorStatusNotify(pstMsg);

                return CVI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        case CVI_EVENT_STORAGEMNG_MOUNTED:
            {
                CVI_MODEMNG_InitFilemng();
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        case CVI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
        case CVI_EVENT_STORAGEMNG_FS_EXCEPTION:
            {
                if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_FSERROR))
                    CVI_MODEMNG_MonitorStatusNotify(pstMsg);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
            break;
        case CVI_EVENT_USB_UVC_READY:
        {
            *pStateID = CVI_WORK_MODE_USBCAM;
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_RECMNG_STOPREC:
        case CVI_EVENT_RECMNG_SPLITREC:
        case CVI_EVENT_RECMNG_EVENTREC_END:
        {
            if (pstMsg->topic == CVI_EVENT_RECMNG_EVENTREC_END) {
                CVI_MODEMNG_SetEmrState(false);
            }
        }
        case CVI_EVENT_RECMNG_EMRREC_END:
        case CVI_EVENT_RECMNG_PIV_END:
                CVI_MODEMNG_MonitorStatusNotify(pstMsg);
#ifdef SERVICES_PHOTO_ON
        case CVI_EVENT_PHOTOMNG_PIV_START:
        case CVI_EVENT_PHOTOMNG_PIV_END:
#endif
            {
                if (CVI_CARD_STATE_AVAILABLE == CVI_MODEMNG_GetCardState()) {
                    CVI_FILEMNG_RecoverRemoveFileName((char *)pstMsg->aszPayload);
                }
                if (pstMsg->topic == CVI_EVENT_RECMNG_EMRREC_END) {
                    CVI_FILEMNG_RenameMovToEmr((char *)pstMsg->aszPayload);
                } else {
                    CVI_FILEMNG_AddFile((char *)pstMsg->aszPayload);
                }
                // cvi_async();
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_RECMNG_STARTREC:
        case CVI_EVENT_RECMNG_STARTEVENTREC:
        case CVI_EVENT_RECMNG_STARTEMRREC:
                CVI_MODEMNG_MonitorStatusNotify(pstMsg);
        case CVI_EVENT_RECMNG_SPLITSTART:
            {
                CVI_FILEMNG_RecoverAddFileName((char *)pstMsg->aszPayload);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_RECMNG_OPEN_FAILED:
        case CVI_EVENT_RECMNG_WRITE_ERROR:
            {
                CVI_MODEMNG_StopRec();
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_RECMNG_PIV_START:
            CVI_MODEMNG_MonitorStatusNotify(pstMsg);
            break;
#ifdef SERVICES_SPEECH_ON
        case CVI_EVENT_SPEECHMNG_STARTREC:
        case CVI_EVENT_SPEECHMNG_STOPREC:
        case CVI_EVENT_SPEECHMNG_OPENFRONT:
        case CVI_EVENT_SPEECHMNG_OPENREAR:
        case CVI_EVENT_SPEECHMNG_CLOSESCREEN:
        case CVI_EVENT_SPEECHMNG_OPENSCREEN:
        case CVI_EVENT_SPEECHMNG_EMRREC:
        case CVI_EVENT_SPEECHMNG_PIV:
        case CVI_EVENT_SPEECHMNG_CLOSEWIFI:
        case CVI_EVENT_SPEECHMNG_OPENWIFI:
        CVI_MODEMNG_MonitorStatusNotify(pstMsg);
            break;
        case CVI_EVENT_MODEMNG_START_SPEECH:
        {
            CVI_MODEMNG_StartEventSpeech();
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_MODEMNG_STOP_SPEECH:
        {
            CVI_MODEMNG_StopEventSpeech();
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
#endif
        default:
            break;
    }

    return CVI_PROCESS_MSG_RESULTE_OK;
}

static int32_t CVI_MODEMNG_OpenBaseMode(void)
{
    int32_t s32Ret = 0;

    s32Ret = CVI_USBCTRL_Init();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"usbctrl init");

    /** init timedtask */
    s32Ret = CVI_TIMEDTASK_Init();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"timetask init");

    s32Ret = CVI_MEDIA_SensorDet();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"sensor detect");
    return 0;
}

static int32_t CVI_MODEMNG_CloseBaseMode(void)
{
    return 0;
}

int32_t CVI_MODEMNG_BaseStateInit(void)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_S *pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    pstModeMngCtx->stBase.stateID = CVI_WORK_MODE_BUTT;
    snprintf(pstModeMngCtx->stBase.name, CVI_STATE_NAME_LEN, "%s", MODEEMNG_STATE_BASE);
    pstModeMngCtx->stBase.open = CVI_MODEMNG_OpenBaseMode;
    pstModeMngCtx->stBase.close = CVI_MODEMNG_CloseBaseMode;
    pstModeMngCtx->stBase.processMessage = CVI_MODEMNG_BaseModeProcessMessage;
    pstModeMngCtx->stBase.argv = &(pstModeMngCtx->stBase);

    s32Ret = CVI_HFSM_AddState(pstModeMngCtx->pvHfsmHdl,
                              &(pstModeMngCtx->stBase),
                              NULL);
    return s32Ret;
}

/** deinit Base mode */
int32_t CVI_MODEMNG_BaseStatesDeinit(void)
{
    int32_t s32Ret = 0;

    return s32Ret;
}

