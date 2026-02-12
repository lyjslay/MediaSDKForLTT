#include <stdio.h>
#include "ui_windowmng.h"
#include "event_recorder_player.h"
#ifdef CONFIG_GSENSOR_ON
#include "cvi_gsensormng.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static CVI_UI_MESSAGE_CONTEXT s_stMessageCtx = {.bMsgProcessed = true, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
static bool s_bPowerOff = false;

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

static int32_t  UI_Common_PowerOff(void)
{
    CVI_MESSAGE_S Msg = {0};
    uint32_t s32Ret = 0;
    //close backlight
    CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_0, CVI_HAL_SCREEN_STATE_OFF);
    #ifdef CONFIG_GSENSOR_ON
    CVI_PARAM_MENU_S menu_param = {0};
    CVI_PARAM_GetMenuParam(&menu_param);
    if (menu_param.Parking.Current == CVI_MENU_PARKING_ON) {
        CVI_GSENSORMNG_OpenInterrupt(0);
    }
    #endif
    if (s_bPowerOff == false) {
        Msg.topic = CVI_EVENT_MODEMNG_POWEROFF;
        s32Ret = CVI_MODEMNG_SendMessage(&Msg);
        if (0 != s32Ret) {
            CVI_LOGI("CVI_MODEMNG_SendMessage fail\n");
            return -1;
        }
        s_bPowerOff = true;
    }
    
    return 0;
}

static void ui_keyton_voice(void) 
{
    CVI_VOICEPLAY_VOICE_S stVoice=
    {
        .au32VoiceIdx={UI_VOICE_KEY_IDX},
        .u32VoiceCnt=1,
        .bDroppable=true,
    };
    CVI_VOICEPLAY_Push(&stVoice, 0);
}

static int32_t  ui_key_event(int32_t  keyid, bool longkey)
{
    event_queue_req_t r;
    key_event_t event;

    memset(&r, 0x00, sizeof(r));
    memset(&event, 0x00, sizeof(event));

    int32_t  key = 0;
    switch(keyid)
    {
        case CVI_KEYMNG_KEY_IDX_0:
            key = UI_KEY_POWER;
            if (longkey == true) {
                CVI_LOGD("POWER OFF !\n");
                UI_Common_PowerOff();
                return 0;
            }
            break;
        case CVI_KEYMNG_KEY_IDX_1:
            key = UI_KEY_UP;
            break;
        case CVI_KEYMNG_KEY_IDX_2:
            key = UI_KEY_MENU;
            break;
        case CVI_KEYMNG_KEY_IDX_3:
            key = UI_KEY_DOWN;
            break;
    }
    event.key = key;
    event.alt = longkey;
    event.e.type = EVT_UI_KEY_DOWN;

    r.key_event = event;
    main_loop_queue_event(main_loop(), &r);

    return 0;
}

static int32_t  ui_battery_event(int32_t  level)
{
    event_queue_req_t r;
    key_event_t event;

    memset(&r, 0x00, sizeof(r));
    memset(&event, 0x00, sizeof(event));
    event.key = level;
    event.alt = 0;
    event.e.type = EVT_LOW_BATTERY;

    r.key_event = event;
    main_loop_queue_event(main_loop(), &r);

    return 0;
}

static uint8_t langid = 0;
static ret_t ui_common_setlanguage(const idle_info_t* idle)
{
    uint8_t type = *(uint8_t*)(idle->ctx);
    option_setuiLanguage(type);

    return RET_OK;
}

int32_t  ui_common_eventcb(void *argv, CVI_EVENT_S *msg)
{
    static int32_t  key_tong;
    int32_t  s32Ret = 0;

    if (s_bPowerOff == true) {
        CVI_LOGI("power off ignore event id: %x\n", msg->topic);
        return 0;
    }

    /*play key tone*/
    CVI_PARAM_GetKeyTone(&key_tong);
    if (key_tong == CVI_MEDIA_AUDIO_KEYTONE_ON) {
        if (msg->topic == CVI_EVENT_KEYMNG_LONG_CLICK || 
            msg->topic == CVI_EVENT_KEYMNG_SHORT_CLICK) {
            ui_keyton_voice();
        }
    }

    /*receive message result*/
    s32Ret = CVI_UICOMM_MessageResult(msg);
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "MessageResult");

    /*receive event control power*/
    bool bEventContinueHandle  = false;
    s32Ret = UI_POWERCTRL_PreProcessEvent(msg, &bEventContinueHandle);
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PreProcessEvent");
    if (!bEventContinueHandle) {
        CVI_LOGI("Event %x has been processed by Power Control Module\n", msg->topic);
        return 0;
    }

    /*get cur mode*/
    int32_t  s32CurMode = CVI_MODEMNG_GetCurWorkMode();

    switch(msg->topic) {
        case CVI_EVENT_MODEMNG_CARD_FORMATING:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_PROCESS);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("CVI_EVENT_MODEMNG_CARD_FORMATING\n");
            return 0;
        }
        case CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED:
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_SUCCESS);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED\n");
            return 0;
        case CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_FAILED);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED\n");
            return 0;
        }
        case CVI_EVENT_MODETEST_START_RECORD:
        {
            event_recorder_player_start_record("/mnt/system/ui_record.txt");
            return 0;
        }
        case CVI_EVENT_MODETEST_STOP_RECORD:
        {
            CVI_LOGD("CVI_EVENT_MODETEST_STOP_RECORD\n");
            event_recorder_player_stop_record();
            return 0;
        }
        case CVI_EVENT_MODETEST_PLAY_RECORD:
        {
            CVI_LOGD("CVI_EVENT_MODETEST_PLAY_RECORD\n");
            event_recorder_player_start_play("/mnt/system/ui_record.txt", msg->arg1);
            return 0;
        }
        case CVI_EVENT_KEYMNG_LONG_CLICK:
        {
            if ((0 == CVI_NETCTRL_NetToUiConnectState()) && 
                (s_stMessageCtx.bMsgProcessed == true)) {
                ui_key_event(msg->arg1, true);
            }
            return 0;
        }
        case CVI_EVENT_KEYMNG_SHORT_CLICK:
        {
            if ((0 == CVI_NETCTRL_NetToUiConnectState()) && 
                (s_stMessageCtx.bMsgProcessed == true)) {
                ui_key_event(msg->arg1, false);
            }
            return 0;
        }
        case CVI_EVENT_USB_OUT:
        {
            UI_Common_PowerOff();
            return 0;
        }
        case CVI_EVENT_MODEMNG_SETTING_LANGUAGE:
        {
            langid = msg->arg1;
            idle_queue(ui_common_setlanguage, (void*)&langid);
            return 0;
        }
        case CVI_EVENT_MODEMNG_MODEOPEN:
        case CVI_EVENT_MODEMNG_MODECLOSE:
            s32CurMode = msg->arg1;
            break;
        case CVI_EVENT_MODEMNG_UPFILE_SUCCESSED:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_OTA_UP_FILE_SUCCESSED);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("CVI_EVENT_MODEMNG_START_UPFILE\n");
            return 0;
        }
        case CVI_EVENT_MODEMNG_UPFILE_FAIL:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_OTA_UP_FILE_FAIL);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_MESSAGE_S Msg = {0};
            Msg.topic = CVI_EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = CVI_WORK_MODE_MOVIE;
            CVI_MODEMNG_SendMessage(&Msg);
            CVI_LOGD("CVI_EVENT_MODEMNG_UPFILE_FAIL\n");
            return 0;
        }
        case CVI_EVENT_MODEMNG_UPFILE_FAIL_FILE_ERROR:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_OTA_UP_FILE_FAIL_FILE_ERROR);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_MESSAGE_S Msg = {0};
            Msg.topic = CVI_EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = CVI_WORK_MODE_MOVIE;
            CVI_MODEMNG_SendMessage(&Msg);
            CVI_LOGD("CVI_EVENT_MODEMNG_UPFILE_FAIL_FILE_ERROR\n");
            return 0;
        }
        case CVI_EVENT_GAUGEMNG_LEVEL_CHANGE:
        {
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            ui_battery_event(msg->arg1);
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
    if (s32CurMode == CVI_WORK_MODE_MOVIE) {
        ui_homepage_eventcb(argv, msg);
    } else if (s32CurMode == CVI_WORK_MODE_PLAYBACK) {
        ui_playbackpage_eventcb(argv, msg);
    } else if(s32CurMode == CVI_WORK_MODE_UPDATE) {
        ui_wrnmsg_update_type(MSG_EVENT_ID_OTA_UP_FILE);
        ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    } else if (s32CurMode == CVI_WORK_MODE_LAPSE) {
        ui_homepage_eventcb(argv, msg);
    } else if (s32CurMode == CVI_WORK_MODE_UVC) {
        ui_wrnmsg_update_type(MSG_EVENT_ID_UVC);
        ui_winmng_startwin(UI_WRNMSG_PAGE, false);     
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
        CVI_EVENT_MODEMNG_SETTING_LANGUAGE,
        CVI_EVENT_MODEMNG_START_UPFILE,
        CVI_EVENT_MODEMNG_UPFILE_SUCCESSED,
        CVI_EVENT_MODEMNG_UPFILE_FAIL,
        CVI_EVENT_MODEMNG_UPFILE_FAIL_FILE_ERROR,
        CVI_EVENT_MODEMNG_RECODER_STARTSTATU,
        CVI_EVENT_MODEMNG_RECODER_STOPSTATU,
        CVI_EVENT_MODEMNG_RECODER_SPLITREC,
        CVI_EVENT_MODEMNG_RECODER_STARTEVENTSTAUE,
        CVI_EVENT_MODEMNG_RECODER_STOPEVENTSTAUE,
        CVI_EVENT_MODEMNG_RECODER_STARTEMRSTAUE,
        CVI_EVENT_MODEMNG_RECODER_STOPEMRSTAUE,
        CVI_EVENT_MODEMNG_RECODER_STARTPIVSTAUE,
        CVI_EVENT_MODETEST_START_RECORD,
        CVI_EVENT_MODETEST_STOP_RECORD,
        CVI_EVENT_MODETEST_PLAY_RECORD,
        CVI_EVENT_KEYMNG_LONG_CLICK,
        CVI_EVENT_KEYMNG_SHORT_CLICK,
        CVI_EVENT_GSENSORMNG_COLLISION,
        CVI_EVENT_USB_OUT,
        CVI_EVENT_USB_INSERT,
        CVI_EVENT_NETCTRL_APPCONNECT_SUCCESS,
        CVI_EVENT_MODEMNG_CARD_FORMAT,
        CVI_EVENT_NETCTRL_UIUPDATE,
        CVI_EVENT_GAUGEMNG_LEVEL_CHANGE,
        CVI_EVENT_GAUGEMNG_LEVEL_LOW,
        CVI_EVENT_GAUGEMNG_LEVEL_ULTRALOW,
        CVI_EVENT_GAUGEMNG_LEVEL_NORMAL,
        CVI_EVENT_GAUGEMNG_CHARGESTATE_CHANGE,
    };

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