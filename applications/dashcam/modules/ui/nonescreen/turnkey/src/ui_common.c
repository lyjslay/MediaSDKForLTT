#include <stdio.h>
#include "ui_common.h"
//#include "event_recorder_player.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static CVI_UI_MESSAGE_CONTEXT s_stMessageCtx = {.bMsgProcessed = true, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
static uint32_t type;
static bool key_power_off = false;
int32_t  CVI_PowerButton_Event(void);

#ifndef CONFIG_SERVICES_LIVEVIEW_ON
#define CVI_VOICE_MAX_SEGMENT_CNT (5)
typedef struct _CVI_VOICEPLAY_VOICE_S
{
    uint32_t volume;
    uint32_t u32VoiceCnt;
    uint32_t au32VoiceIdx[CVI_VOICE_MAX_SEGMENT_CNT];
    bool bDroppable;
} CVI_VOICEPLAY_VOICE_S;
#endif

static int32_t  CVI_UICOMM_MessageResult(CVI_EVENT_S *pstEvent)
{
    int32_t  s32Ret = 0;
    CVI_MUTEX_LOCK(s_stMessageCtx.MsgMutex);

    if (!s_stMessageCtx.bMsgProcessed) {
        CVI_LOGD("event(%x)\n\n", pstEvent->topic);
        if ((s_stMessageCtx.stMsg.topic == pstEvent->topic)
            && (s_stMessageCtx.stMsg.arg1 == pstEvent->arg1)
            && (s_stMessageCtx.stMsg.arg2 == pstEvent->arg2)) {
            if (s_stMessageCtx.pfnMsgResultProc != NULL) {
                s32Ret = s_stMessageCtx.pfnMsgResultProc(pstEvent);
                if (0 != s32Ret) {
                    CVI_LOGE("pfnMsgResultProc() Error:%#x\n", s32Ret);
                }
            }
            s_stMessageCtx.bMsgProcessed = true;
        }
    }

    CVI_MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);

    return s32Ret;
}

bool ui_common_cardstatus(void)
{
    uint32_t CurMode = CVI_WORK_MODE_BUTT;

    switch(CVI_MODEMNG_GetCardState()) {
        case CVI_CARD_STATE_REMOVE:
            type = EVT_NO_SDCARD;
            break;
        case CVI_CARD_STATE_AVAILABLE:
            return true;
            break;
        case CVI_CARD_STATE_ERROR:
            type = EVT_SDCARD_ERROR;
            break;
        case CVI_CARD_STATE_FSERROR:
        case CVI_CARD_STATE_UNAVAILABLE:
            type = EVT_SDCARD_NEED_FORMAT;
            break;
        case CVI_CARD_STATE_SLOW:
            type = EVT_SDCARD_SLOW;
            break;
        case CVI_CARD_STATE_CHECKING:
            type = EVT_SDCARD_CHECKING;
            break;
        case CVI_CARD_STATE_READ_ONLY:
            type = EVT_SDCARD_READ_ONLY;
            break;
        case CVI_CARD_STATE_MOUNT_FAILED:
            type = EVT_SDCARD_MOUNT_FAILED;
            break;
        default:
            CVI_LOGE("value is invalid\n");
            break;
    }
    CVI_MODEMNG_SetParkingRec(false);

    CVI_MODEMNG_GetCurMode(&CurMode);
    if (CurMode == CVI_WORK_MODE_MOVIE) {
    }

    return false;
}

static bool ui_close_flag = false;
int32_t  CVI_PowerButton_Event(void)
{
    CVI_MESSAGE_S Msg = {0};
    u_int32_t s32Ret = 0;
    if (key_power_off == true) {
        Msg.topic = CVI_EVENT_MODEMNG_POWEROFF;
        s32Ret = CVI_MODEMNG_SendMessage(&Msg);
        if (0 != s32Ret) {
            CVI_LOGI("CVI_MODEMNG_SendMessage fail\n");
            return -1;
        }
    } else {
        if (ui_close_flag == false) {
            CVI_LOGE("pannel is going close\n");
            ui_close_flag = true;
        } else {
            CVI_LOGE("pannel is going open\n");
            ui_close_flag = false;
        }

    }
    return 0;
}

int32_t  ui_common_eventcb(void *argv, CVI_EVENT_S *msg)
{
    CVI_MESSAGE_S Msg = {0};
    int32_t  s32Ret = 0;
    /*receive message result*/
    s32Ret = CVI_UICOMM_MessageResult(msg);
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "MessageResult");

    if (!(CVI_EVENT_MODEMNG_RECODER_STARTSTATU  == msg->topic || CVI_EVENT_MODEMNG_RECODER_STOPSTATU ==msg->topic))
    {

    }

    switch(msg->topic) {
        case CVI_EVENT_MODEMNG_CARD_REMOVE:
            {
                if (CVI_MODEMNG_GetCurWorkMode() == CVI_WORK_MODE_PLAYBACK) {
                    Msg.topic = CVI_EVENT_MODEMNG_MODESWITCH;
                    Msg.arg1 = CVI_WORK_MODE_MOVIE;
                    CVI_MODEMNG_SendMessage(&Msg);
                } else {
                    ui_common_cardstatus();
                }
            }
            break;
        case CVI_EVENT_MODEMNG_CARD_AVAILABLE:
        case CVI_EVENT_MODEMNG_RESET:
            {
                uint32_t u32ModeState = 0;
                CVI_MODEMNG_GetModeState(&u32ModeState);
                CVI_LOGD("u32ModeState == %d\n", u32ModeState);
                if (ui_common_cardstatus() == true && u32ModeState != CVI_MEDIA_MOVIE_STATE_MENU) {
                    Msg.topic = CVI_EVENT_MODEMNG_START_REC;
                    CVI_MODEMNG_SendMessage(&Msg);
                }
            }
            break;
        case CVI_EVENT_MODEMNG_CARD_FSERROR:
        case CVI_EVENT_MODEMNG_CARD_SLOW:
        case CVI_EVENT_MODEMNG_CARD_CHECKING:
        case CVI_EVENT_MODEMNG_CARD_UNAVAILABLE:
        case CVI_EVENT_MODEMNG_CARD_ERROR:
        case CVI_EVENT_MODEMNG_CARD_READ_ONLY:
        case CVI_EVENT_MODEMNG_CARD_MOUNT_FAILED:
            ui_common_cardstatus();
            break;
        case CVI_EVENT_MODEMNG_MODEOPEN:
            {
                CVI_LOGD("CVI_EVENT_MODEMNG_MODEOPEN\n");
                switch(msg->arg1) {
                    case CVI_WORK_MODE_MOVIE:
                        break;
                    case CVI_WORK_MODE_PLAYBACK:
                        break;
                    case CVI_WORK_MODE_USBCAM:
                        break;
                    case CVI_WORK_MODE_USB:
                        break;
                    case CVI_WORK_MODE_UVC:
                        break;
                    case CVI_WORK_MODE_STORAGE:
                        break;
                    default:
                        break;
                }
            }
            break;
        case CVI_EVENT_MODEMNG_MODECLOSE:
            {
                CVI_LOGD("CVI_EVENT_MODEMNG_MODECLOSE\n");
                switch(msg->arg1) {
                    case CVI_WORK_MODE_MOVIE:
                        break;
                    case CVI_WORK_MODE_PLAYBACK:
                        break;
                    case CVI_WORK_MODE_USBCAM:
                        break;
                    case CVI_WORK_MODE_USB:
                        break;
                    default:
                        break;
                }
            }
            break;
        case CVI_EVENT_MODEMNG_RECODER_STARTSTATU:
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        case CVI_EVENT_MODEMNG_RECODER_STOPSTATU:
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        case CVI_EVENT_MODEMNG_RECODER_SPLITREC:
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        case CVI_EVENT_MODEMNG_RECODER_STARTEVENTSTAUE:
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        case CVI_EVENT_MODEMNG_RECODER_STOPEVENTSTAUE:
        {
            // CVI_MODEMNG_SetEmrState(false);
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        }
        case CVI_EVENT_MODEMNG_CARD_FORMATING:
        {
            type = EVT_FORMAT_PROCESS;
            CVI_LOGD("CVI_EVENT_MODEMNG_CARD_FORMATING\n");
            break;
        }
        case CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED:
            type = EVT_FORMAT_SUCCESS;
            CVI_LOGD("CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED\n");
            break;
        case CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED:
        {
            type = EVT_FORMAT_FAILED;
            CVI_LOGD("CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED\n");
            break;
        }
        case CVI_EVENT_MODEMNG_PLAYBACK_FINISHED:
            CVI_LOGD("CVI_EVENT_MODEMNG_PLAYBACK_FINISHED\n");
            break;
        case CVI_EVENT_MODEMNG_PLAYBACK_PROGRESS:
            CVI_LOGD("CVI_EVENT_MODEMNG_PLAYBACK_PROGRESS\n");
            break;
        case CVI_EVENT_MODEMNG_PLAYBACK_PAUSE:
            CVI_LOGD("CVI_EVENT_MODEMNG_PLAYBACK_PAUSE\n");
            break;
        case CVI_EVENT_MODEMNG_PLAYBACK_RESUME:
            CVI_LOGD("CVI_EVENT_MODEMNG_PLAYBACK_RESUME\n");
            break;
        case CVI_EVENT_MODEMNG_PLAYBACK_ABNORMAL:
        {
            type = EVT_FILE_ABNORMAL;
            CVI_LOGD("CVI_EVENT_PLAYBACKMODE_FILE_ABNORMAL\n");
            break;
        }
        case CVI_EVENT_MODETEST_START_RECORD:
        {
            CVI_LOGD("CVI_EVENT_MODETEST_START_RECORD\n");
            CVI_LOGE(" noscreen not suport !\n");
            //event_recorder_player_start_record("/mnt/system/ui_record.txt");
            break;
        }
        case CVI_EVENT_MODETEST_STOP_RECORD:
        {
            CVI_LOGD("CVI_EVENT_MODETEST_STOP_RECORD\n");
            CVI_LOGE(" noscreen not suport !\n");
            // event_recorder_player_stop_record();
            break;
        }
        case CVI_EVENT_MODETEST_PLAY_RECORD:
        {
            CVI_LOGE(" noscreen not suport !\n");
            CVI_LOGD("CVI_EVENT_MODETEST_PLAY_RECORD\n");
            // event_recorder_player_start_play("/mnt/system/ui_record.txt", msg->arg1);
            break;
        }
#ifdef SERVICES_IMAGE_VIEWER_ON
        case CVI_EVENT_AHDMNG_PLUG_STATUS:
        {
            if (msg->arg1 == CVI_AHDMNG_PLUG_OUT) {
                CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_VIEW_WIN_STATUS, CVI_MEDIA_VIEW_WIN_FRONT);
            } else if (msg->arg1 == CVI_AHDMNG_PLUG_IN) {
                CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_VIEW_WIN_STATUS, CVI_MEDIA_VIEW_WIN_DOUBLE);
            }
            break;
        }
#endif
        case CVI_EVENT_KEYMNG_LONG_CLICK:
        {
            break;
        }
        case CVI_EVENT_KEYMNG_SHORT_CLICK:
        {
            break;
        }
        case CVI_EVENT_MODEMNG_RECODER_STARTPIVSTAUE:
        {
#ifdef SERVICES_LIVEVIEW_ON
            if (msg->arg1 == 0)  {
                CVI_VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_PHOTO_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=false,
                };
                CVI_VOICEPLAY_Push(&stVoice, 0);
            }
#endif
            break;
        }
#ifdef SERVICES_SPEECH_ON
        case CVI_EVENT_MODEMNG_SPEECHMNG_STARTREC:
        {
            uint32_t u32ModeState = 0;
            CVI_MODEMNG_GetModeState(&u32ModeState);
            if (ui_common_cardstatus() == true &&
               (u32ModeState != CVI_MEDIA_MOVIE_STATE_REC) &&
               (u32ModeState != CVI_MEDIA_MOVIE_STATE_LAPSE_REC)) {
                Msg.topic = CVI_EVENT_MODEMNG_START_REC;
                CVI_MODEMNG_SendMessage(&Msg);
            }
            break;
        }
        case CVI_EVENT_MODEMNG_SPEECHMNG_STOPREC:
        {
            uint32_t u32ModeState = 0;
            CVI_MODEMNG_GetModeState(&u32ModeState);
            if ((u32ModeState == CVI_MEDIA_MOVIE_STATE_REC) ||
               (u32ModeState == CVI_MEDIA_MOVIE_STATE_LAPSE_REC)) {
                Msg.topic = CVI_EVENT_MODEMNG_STOP_REC;
                CVI_MODEMNG_SendMessage(&Msg);
            }
            break;
        }
        case CVI_EVENT_MODEMNG_SPEECHMNG_OPENFRONT:
        {
            uint32_t curWind = (uint32_t)CVI_PARAM_Get_View_Win();
            if (curWind == 0) {
                break;
            }
            Msg.topic = CVI_EVENT_MODEMNG_SWITCH_LIVEVIEW;
            uint32_t enWind = (curWind >> 16) & 0xFFFF;
            uint32_t enSns = (curWind & 0xFFFF);
            if (enWind == 0x1) {
                break;
            }
            Msg.arg1 = ((0x1 << 16) | enSns);
            CVI_MODEMNG_SendMessage(&Msg);
            break;
        }
        case CVI_EVENT_MODEMNG_SPEECHMNG_OPENREAR:
        {
            uint32_t curWind = (uint32_t)CVI_PARAM_Get_View_Win();
            if (curWind == 0) {
                break;
            }
            Msg.topic = CVI_EVENT_MODEMNG_SWITCH_LIVEVIEW;
            uint32_t enWind = (curWind >> 16) & 0xFFFF;
            uint32_t enSns = (curWind & 0xFFFF);
            if (enWind == 0x10 || (enSns >> 1) != 0x1) {
                break;
            }
            Msg.arg1 = ((0x1 << 17) | enSns);
            CVI_MODEMNG_SendMessage(&Msg);
            break;
        }
#ifdef CONFIG_SCREEN_ON
        case CVI_EVENT_MODEMNG_SPEECHMNG_CLOSESCREEN:
        {
            s32Ret = CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_0, CVI_HAL_SCREEN_STATE_OFF);
            if (s32Ret != 0){
                CVI_LOGI("Close screen fail\n");
            }
            break;
        }
        case CVI_EVENT_MODEMNG_SPEECHMNG_OPENSCREEN:
        {
            s32Ret = CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_0, CVI_HAL_SCREEN_STATE_ON);
            if (s32Ret != 0){
                CVI_LOGI("Open screen fail\n");
            }
            break;
        }
#endif
        case CVI_EVENT_MODEMNG_SPEECHMNG_EMRREC:
        {
            uint32_t u32ModeState = 0;
            CVI_MODEMNG_GetModeState(&u32ModeState);
            if (u32ModeState != CVI_MEDIA_MOVIE_STATE_LAPSE_REC && ui_common_cardstatus()) {
                Msg.topic = CVI_EVENT_MODEMNG_START_EMRREC;
                CVI_MODEMNG_SendMessage(&Msg);
            }
            break;
        }
        case CVI_EVENT_MODEMNG_SPEECHMNG_PIV:
        {
            if (ui_common_cardstatus() == true) {
                Msg.topic = CVI_EVENT_MODEMNG_START_PIV;
                CVI_MODEMNG_SendMessage(&Msg);
            }
            break;
        }
#ifdef CONFIG_WIFI_ON
        case CVI_EVENT_MODEMNG_SPEECHMNG_CLOSEWIFI:
        {
            CVI_PARAM_WIFI_S WifiParam = {0};
            CVI_MESSAGE_S Msg = {0};
            CVI_PARAM_GetWifiParam(&WifiParam);
            if (WifiParam.Enable) {
                Msg.topic = CVI_EVENT_MODEMNG_SETTING;
                Msg.arg1 = CVI_PARAM_MENU_WIFI_STATUS;
                Msg.arg2 = 0;
                CVI_MODEMNG_SendMessage(&Msg);
                widget_t* topwin = window_manager_get_top_main_window(window_manager());
                if (topwin != NULL) {
                    widget_t* wifi_image_widget = widget_lookup(topwin, "wifi_btm_image", TRUE);
                    image_base_set_image(wifi_image_widget, "wifi_off");
                }
            }
            break;
        }
        case CVI_EVENT_MODEMNG_SPEECHMNG_OPENWIFI:
        {
            CVI_PARAM_WIFI_S WifiParam = {0};
            CVI_MESSAGE_S Msg = {0};
            CVI_PARAM_GetWifiParam(&WifiParam);
            if (!WifiParam.Enable) {
                Msg.topic = CVI_EVENT_MODEMNG_SETTING;
                Msg.arg1 = CVI_PARAM_MENU_WIFI_STATUS;
                Msg.arg2 = 1;
                CVI_MODEMNG_SendMessage(&Msg);
                widget_t* topwin = window_manager_get_top_main_window(window_manager());
                if (topwin != NULL) {
                    widget_t* wifi_image_widget = widget_lookup(topwin, "wifi_btm_image", TRUE);
                    image_base_set_image(wifi_image_widget, "wifi_on");
                }
            }
            break;
        }
#endif
#endif
        default:
            break;
    }

    return 0;
}
int32_t  CVI_UICOMM_SendAsyncMsg(CVI_MESSAGE_S* pstMsg, CVI_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc)
{
    int32_t  s32Ret = 0;
    CVI_APPCOMM_CHECK_POINTER(pstMsg, -1);

    CVI_MUTEX_LOCK(s_stMessageCtx.MsgMutex);

    if (!s_stMessageCtx.bMsgProcessed) {
        CVI_LOGE("Current Msg not finished\n");
        CVI_MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);
        return -1;
    }

    s_stMessageCtx.bMsgProcessed = false;
    s_stMessageCtx.stMsg.topic = pstMsg->topic;
    s_stMessageCtx.stMsg.arg1 = pstMsg->arg1;
    s_stMessageCtx.stMsg.arg2 = pstMsg->arg2;
    memcpy(s_stMessageCtx.stMsg.aszPayload, pstMsg->aszPayload, sizeof(s_stMessageCtx.stMsg.aszPayload));
    s_stMessageCtx.pfnMsgResultProc = pfnMsgResultProc;

    CVI_LOGD("[what:%#x, arg1:%#x, arg2:%#x]\n", pstMsg->topic, pstMsg->arg1, pstMsg->arg2);
    s32Ret = CVI_MODEMNG_SendMessage(pstMsg);

    if (0 != s32Ret) {
        CVI_LOGE("Error:%#x\n", s32Ret);
        s_stMessageCtx.bMsgProcessed = true;
        CVI_MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);
        return -1;
    }

    CVI_MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);
    return 0;
}

int32_t  ui_common_SubscribeEvents(void)
{
    int32_t  ret = 0;
    uint32_t i = 0;
    CVI_EVENTHUB_SUBSCRIBER_S stSubscriber = {"ui", NULL, ui_common_eventcb, false};
    CVI_MW_PTR SubscriberHdl = NULL;
    CVI_TOPIC_ID topic[] = {
        CVI_EVENT_MODEMNG_CARD_REMOVE,
        CVI_EVENT_MODEMNG_CARD_AVAILABLE,
        CVI_EVENT_MODEMNG_CARD_UNAVAILABLE,
        CVI_EVENT_MODEMNG_CARD_ERROR,
        CVI_EVENT_MODEMNG_CARD_FSERROR,
        CVI_EVENT_MODEMNG_CARD_SLOW,
        CVI_EVENT_MODEMNG_CARD_CHECKING,
        CVI_EVENT_MODEMNG_CARD_FORMATING,
        CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED,
        CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED,
        CVI_EVENT_MODEMNG_CARD_READ_ONLY,
        CVI_EVENT_MODEMNG_CARD_MOUNT_FAILED,
        CVI_EVENT_MODEMNG_RESET,
        CVI_EVENT_MODEMNG_MODESWITCH,
        CVI_EVENT_MODEMNG_MODEOPEN,
        CVI_EVENT_MODEMNG_MODECLOSE,
        CVI_EVENT_MODEMNG_SETTING,
        CVI_EVENT_MODEMNG_START_PIV,
        CVI_EVENT_MODEMNG_PLAYBACK_FINISHED,
        CVI_EVENT_MODEMNG_PLAYBACK_PROGRESS,
        CVI_EVENT_MODEMNG_PLAYBACK_PAUSE,
        CVI_EVENT_MODEMNG_PLAYBACK_RESUME,
        CVI_EVENT_MODEMNG_PLAYBACK_ABNORMAL,
        CVI_EVENT_MODEMNG_RECODER_STARTSTATU,
        CVI_EVENT_MODEMNG_RECODER_STOPSTATU,
        CVI_EVENT_MODEMNG_RECODER_SPLITREC,
        CVI_EVENT_MODEMNG_RECODER_STARTEVENTSTAUE,
        CVI_EVENT_MODEMNG_RECODER_STOPEVENTSTAUE,
        CVI_EVENT_MODEMNG_RECODER_STARTEMRSTAUE,
        CVI_EVENT_MODEMNG_RECODER_STOPEMRSTAUE,
        CVI_EVENT_MODEMNG_RECODER_STARTPIVSTAUE,
        CVI_EVENT_FILEMNG_SPACE_FULL,
        CVI_EVENT_MODETEST_START_RECORD,
        CVI_EVENT_MODETEST_STOP_RECORD,
        CVI_EVENT_MODETEST_PLAY_RECORD,
#ifdef SERVICES_IMAGE_VIEWER_ON
        CVI_EVENT_AHDMNG_PLUG_STATUS,
#endif
        CVI_EVENT_KEYMNG_LONG_CLICK,
        CVI_EVENT_KEYMNG_SHORT_CLICK,
        CVI_EVENT_STORAGEMNG_DEV_CONNECTING,
        CVI_WORK_MODE_UVC,
        CVI_WORK_MODE_STORAGE,
        CVI_EVENT_MODEMNG_UVC_MODE_START,
        CVI_EVENT_MODEMNG_STORAGE_MODE_PREPAREDEV
    };
    ret = CVI_EVENTHUB_RegisterTopic(CVI_EVENT_UI_TOUCH);
    CVI_APPCOMM_CHECK_RETURN(ret, ret);

    ret = CVI_EVENTHUB_CreateSubscriber(&stSubscriber, &SubscriberHdl);
    if (ret != 0) {
        CVI_LOGE("CVI_EVENTHUB_CreateSubscriber failed! \n");
    }

    uint32_t u32ArraySize = UI_ARRAY_SIZE(topic);

    for (i = 0; i < u32ArraySize; i++) {
        ret = CVI_EVENTHUB_Subcribe(SubscriberHdl, topic[i]);
        if (ret) {
            CVI_LOGE("Subscribe topic(%#x) failed. %#x\n", topic[i], ret);
            continue;
        }
    }

    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif