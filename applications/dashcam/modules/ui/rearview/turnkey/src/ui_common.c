#include <stdio.h>
#include "ui_windowmng.h"
#include "event_recorder_player.h"
#include "cvi_netctrl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static CVI_UI_MESSAGE_CONTEXT s_stMessageCtx = {.bMsgProcessed = true, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
static uint32_t type;
static bool key_power_off = false;
int32_t  CVI_UICOMM_PowerOff(void);
int32_t  CVI_PowerButton_Event(void);

/* keyevent handling list*/
CVI_KEY_GPIO_EVENT keymngevent[] = { {CVI_KEYMNG_KEY_IDX_0, CVI_UICOMM_PowerOff, CVI_PowerButton_Event},
                                     {CVI_KEYMNG_KEY_IDX_1, CVI_UICOMM_PowerOff, CVI_PowerButton_Event},
                                     {CVI_KEYMNG_KEY_IDX_2, CVI_UICOMM_PowerOff, CVI_PowerButton_Event}, // The interface can be replaced after implementation
                                     {CVI_KEYMNG_KEY_IDX_3, CVI_UICOMM_PowerOff, CVI_PowerButton_Event}};

static ret_t ui_open_homepage(const idle_info_t* idle)
{
    return RET_OK;
}

ret_t ui_open_uvc(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    widget_lookup(win, "uvc", TRUE);

    return RET_OK;
}

ret_t ui_open_storage(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    widget_lookup(win, "storage", TRUE);

    CVI_MESSAGE_S Msg = {0};
    Msg.topic = CVI_EVENT_MODEMNG_STORAGE_MODE_PREPAREDEV;
    CVI_MODEMNG_SendMessage(&Msg);

    return RET_OK;
}

static ret_t ui_close_homepage(const idle_info_t* idle)
{
    ui_home_close();
    return RET_OK;
}

static ret_t ui_open_dirpage(const idle_info_t* idle)
{
    return RET_OK;
}

static ret_t ui_close_page(const idle_info_t* idle)
{
    window_manager_close_all(window_manager());
    return RET_OK;
}

static ret_t ui_open_msgpage(const idle_info_t* idle)
{
    uint32_t type = *(uint32_t*)(idle->ctx);
    ui_wrnmsg_update_type(type);
    ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    return RET_OK;
}

static ret_t ui_close_msgpage(const idle_info_t* idle)
{
    ui_winmng_closeallwin();
    return RET_OK;
}

static ret_t ui_playback_reset(const idle_info_t* idle)
{
    playback_reset_time();
    return RET_OK;
}

static ret_t ui_playback_addtime(const idle_info_t* idle)
{
    playback_add_time();
    return RET_OK;
}

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
            type = MSG_EVENT_ID_NO_CARD;
            break;
        case CVI_CARD_STATE_AVAILABLE:
            if (wrnmsg_window_isopen() == true &&
                ui_wrnmsg_get_type() != MSG_EVENT_APP_CONNECT_SUCCESS) {
                ui_winmng_finishwin(UI_WRNMSG_PAGE);
            }
            return true;
            break;
        case CVI_CARD_STATE_ERROR:
            type = MSG_EVENT_ID_SDCARD_ERROR;
            break;
        case CVI_CARD_STATE_FSERROR:
        case CVI_CARD_STATE_UNAVAILABLE:
            type = MSG_EVENT_ID_SDCARD_NEED_FORMAT;
            break;
        case CVI_CARD_STATE_SLOW:
            type = MSG_EVENT_ID_SDCARD_SLOW;
            break;
        case CVI_CARD_STATE_CHECKING:
            type = MSG_EVENT_ID_SDCARD_CHECKING;
            break;
        case CVI_CARD_STATE_READ_ONLY:
            type = MSG_EVENT_ID_SDCARD_READ_ONLY;
            break;
        case CVI_CARD_STATE_MOUNT_FAILED:
            type = MSG_EVENT_ID_SDCARD_MOUNT_FAILED;
            break;
        default:
            CVI_LOGE("value is invalid\n");
            CVI_MODEMNG_SetParkingRec(false);
            return false;
            break;
    }
    CVI_MODEMNG_SetParkingRec(false);

    CVI_MODEMNG_GetCurMode(&CurMode);
    if (CurMode == CVI_WORK_MODE_MOVIE) {
        ui_wrnmsg_update_type(type);
        ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    }

    return false;
}

int32_t  CVI_UICOMM_PowerOff(void)
{
    widget_t* win = window_open_and_close("ui_close_machine", window_manager_get_top_window(window_manager()));
    if (NULL == win) {
        CVI_LOGE("common window_open_and_close fail\n");
        return RET_OK;
    }
    key_power_off = true;
    return 0;
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
            CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_0, CVI_HAL_SCREEN_STATE_OFF);
            CVI_HAL_TOUCHPAD_Suspend();
            ui_close_flag = true;
        } else {
            CVI_LOGE("pannel is going open\n");
            CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_0, CVI_HAL_SCREEN_STATE_ON);
            CVI_HAL_TOUCHPAD_Resume();
            ui_close_flag = false;
        }

    }
    return 0;
}

int32_t  old_mode = CVI_WORK_MODE_MOVIE;
int32_t  ui_common_eventcb(void *argv, CVI_EVENT_S *msg)
{
    CVI_MESSAGE_S Msg = {0};
    int32_t  s32Ret = 0;
    /*receive message result*/
    s32Ret = CVI_UICOMM_MessageResult(msg);
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "MessageResult");
    CVI_LOGD("ui common eventcb will process message topic(%x) \n\n", msg->topic);

    if (!(CVI_EVENT_MODEMNG_RECODER_STARTSTATU  == msg->topic || CVI_EVENT_MODEMNG_RECODER_STOPSTATU ==msg->topic))
    {
        bool bEventContinueHandle  = false;
        s32Ret = UI_POWERCTRL_PreProcessEvent(msg, &bEventContinueHandle);
        CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PreProcessEvent");
        if (!bEventContinueHandle)
        {
            CVI_LOGI("Event %x has been processed by Power Control Module\n", msg->topic);
            return 0;
        }
    }

    /*get cur mode*/
    int32_t  s32CurMode = CVI_MODEMNG_GetCurWorkMode();

    switch(msg->topic) {
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
        case CVI_EVENT_MODEMNG_MODEOPEN:
            {
                ui_lock();
                CVI_LOGD("CVI_EVENT_MODEMNG_MODEOPEN\n");
                s32CurMode = msg->arg1;
                ui_unlock();
                break;
            }
            break;
        case CVI_EVENT_MODEMNG_MODECLOSE:
            {
                ui_lock();
                CVI_LOGD("CVI_EVENT_MODEMNG_MODECLOSE\n");
                s32CurMode = msg->arg1;
                ui_unlock();
                break;
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
            //CVI_MODEMNG_SetEmrState(false);
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        }
        case CVI_EVENT_MODEMNG_CARD_FORMATING:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_PROCESS);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("CVI_EVENT_MODEMNG_CARD_FORMATING\n");
            return;
        }
        case CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_SUCCESS);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED\n");
            return;
        }
        case CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_FAILED);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED\n");
            return;
        }
        case CVI_EVENT_MODEMNG_PLAYBACK_FINISHED:
            idle_queue(ui_playback_reset, NULL);
            CVI_LOGD("CVI_EVENT_MODEMNG_PLAYBACK_FINISHED\n");
            break;
        case CVI_EVENT_MODEMNG_PLAYBACK_PROGRESS:
            idle_queue(ui_playback_addtime, NULL);
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
            type = MSG_EVENT_ID_FILE_ABNORMAL;
            idle_queue(ui_open_msgpage, (void*)&type);
            CVI_LOGD("CVI_EVENT_PLAYBACKMODE_FILE_ABNORMAL\n");
            break;
        }
        case CVI_EVENT_MODETEST_START_RECORD:
        {
            event_recorder_player_start_record("/mnt/system/ui_record.txt");
            break;
        }
        case CVI_EVENT_MODETEST_STOP_RECORD:
        {
            CVI_LOGD("CVI_EVENT_MODETEST_STOP_RECORD\n");
            event_recorder_player_stop_record();
            break;
        }
        case CVI_EVENT_MODETEST_PLAY_RECORD:
        {
            CVI_LOGD("CVI_EVENT_MODETEST_PLAY_RECORD\n");
            event_recorder_player_start_play("/mnt/system/ui_record.txt", msg->arg1);
            break;
        }
        case CVI_EVENT_SENSOR_PLUG_STATUS:
        {
            int32_t  snsid = msg->aszPayload[1];
            uint32_t curWind = (uint32_t)CVI_PARAM_Get_View_Win();
            if (msg->arg1 == CVI_SENSOR_PLUG_OUT) {
                curWind &= (~(0x1 << snsid) & 0xFFFF);
            } else if (msg->arg1 == CVI_SENSOR_PLUG_IN) {
                curWind |= (0x1 << snsid);
            }
            curWind = (((curWind & 0xFFFF) << 16) | (curWind & 0xFFFF));
            CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_VIEW_WIN_STATUS, curWind);

            {
                CVI_MESSAGE_S Msg = {0};
                Msg.topic = CVI_EVENT_MODEMNG_SWITCH_LIVEVIEW;
                Msg.arg1 = curWind;
                CVI_MODEMNG_SendMessage(&Msg);
            }
            break;
        }
        case CVI_EVENT_KEYMNG_LONG_CLICK:
        {
            int32_t  keymngcunt = 0;
            int32_t  lkeyflage = 0;
            KEYGPIOEVENT longkeybackll;
            keymngcunt = (sizeof(keymngevent)/sizeof(keymngevent[0]));
            for (int32_t  i = 0; i < keymngcunt; i++) {
                if ((msg->arg1) == (int32_t )(keymngevent[i].gpioidx)) {
                    longkeybackll = keymngevent[i].longkeyback;
                    longkeybackll();
                    lkeyflage = 1;
                    break;
                }
            }

            if (0 == lkeyflage) {
                CVI_LOGE("key mng long click msg error, msg->arg1 = (%d)\n", msg->arg1);
            }

            break;
        }
        case CVI_EVENT_KEYMNG_SHORT_CLICK:
        {
            int32_t  keymngcunt = 0;
            int32_t  skeyflage = 0;
            KEYGPIOEVENT shortkeybackll;
            keymngcunt = (sizeof(keymngevent)/sizeof(keymngevent[0]));
            for (int32_t  i = 0; i < keymngcunt; i++) {
                if ((msg->arg1) == (int32_t )(keymngevent[i].gpioidx)) {
                    shortkeybackll = keymngevent[i].shortkeyback;
                    shortkeybackll();
                    skeyflage = 1;
                    break;
                }
            }

            if (0 == skeyflage) {
                CVI_LOGE("key mng short click msg error, msg->arg1 = (%d)\n", msg->arg1);
            }

            break;
        }
        case CVI_EVENT_UI_TOUCH:
        {
            CVI_VOICEPLAY_VOICE_S stVoice=
            {
                .au32VoiceIdx={UI_VOICE_TOUCH_BTN_IDX},
                .u32VoiceCnt=1,
                .bDroppable=true,
            };
            CVI_VOICEPLAY_Push(&stVoice, 0);
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
#ifdef SERVICES_ADAS_ON
        case CVI_EVENT_ADASMNG_CAR_MOVING:
            {
                CVI_VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_CAR_MOVING_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=true,
                };
                CVI_VOICEPLAY_Push(&stVoice, 0);
                break;
            }
        case CVI_EVENT_ADASMNG_CAR_CLOSING:
            {
                CVI_VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_CAR_CLOSING_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=true,
                };
                CVI_VOICEPLAY_Push(&stVoice, 0);
                break;
            }
        case CVI_EVENT_ADASMNG_CAR_COLLISION:
            {
                CVI_VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_CAR_COLLISION_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=true,
                };
                CVI_VOICEPLAY_Push(&stVoice, 0);
                break;
            }
        case CVI_EVENT_ADASMNG_CAR_LANE:
            {
                CVI_VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_CAR_LANE_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=true,
                };
                CVI_VOICEPLAY_Push(&stVoice, 0);
                break;
            }
#endif
        default:
            break;
    }

    if (s32CurMode == CVI_WORK_MODE_MOVIE) {
        ui_homepage_eventcb(argv, msg);
    } else if (s32CurMode == CVI_WORK_MODE_PLAYBACK) {
        ui_playbackpage_eventcb(argv, msg);
    } else if(s32CurMode == CVI_WORK_MODE_UPDATE) {
        // ui_wrnmsg_update_type(MSG_EVENT_ID_OTA_UP_FILE);
        // ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    } else if (s32CurMode == CVI_WORK_MODE_LAPSE) {
        ui_homepage_eventcb(argv, msg);
    } else if (s32CurMode == CVI_WORK_MODE_UVC) {
        if (old_mode != s32CurMode) {
            ui_winmng_startwin(UI_UVC_PAGE, false);

            CVI_MESSAGE_S Msg = {0};
            Msg.topic = CVI_EVENT_MODEMNG_UVC_MODE_START;
            CVI_MODEMNG_SendMessage(&Msg);
        }
        old_mode = s32CurMode;

    } else if (s32CurMode == CVI_WORK_MODE_STORAGE) {
        ui_winmng_startwin(UI_STORAGE_PAGE, false);
    } else if (s32CurMode == CVI_WORK_MODE_PHOTO) {
        ui_home_photo_page_eventcb(argv, msg);
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
        CVI_EVENT_SENSOR_PLUG_STATUS,
    #ifdef SERVICES_PHOTO_ON
        CVI_PHOTO_SERVICE_EVENT_PIV_START,
        CVI_PHOTO_SERVICE_EVENT_PIV_END,
    #endif
        CVI_EVENT_KEYMNG_LONG_CLICK,
        CVI_EVENT_KEYMNG_SHORT_CLICK,
        CVI_EVENT_UI_TOUCH,
        CVI_EVENT_STORAGEMNG_DEV_CONNECTING,
        CVI_WORK_MODE_UVC,
        CVI_WORK_MODE_STORAGE,
        CVI_EVENT_MODEMNG_UVC_MODE_START,
        CVI_EVENT_MODEMNG_STORAGE_MODE_PREPAREDEV,
    #ifdef SERVICES_ADAS_ON
        CVI_EVENT_ADASMNG_CAR_MOVING,
        CVI_EVENT_ADASMNG_CAR_CLOSING,
        CVI_EVENT_ADASMNG_CAR_COLLISION,
        CVI_EVENT_ADASMNG_CAR_LANE,
    #ifdef SERVICES_ADAS_LABEL_CAR_ON
        CVI_EVENT_ADASMNG_LABEL_CAR,
    #endif
    #ifdef SERVICES_ADAS_LABEL_LANE_ON
        CVI_EVENT_ADASMNG_LABEL_LANE,
    #endif
    #endif
    #if defined (ENABLE_VIDEO_MD)
        CVI_EVENT_VIDEOMD_CHANGE,
    #endif
        CVI_EVENT_NETCTRL_UIUPDATE,
        CVI_EVENT_NETCTRL_APPCONNECT_SUCCESS,
        CVI_EVENT_NETCTRL_APPDISCONNECT,
        CVI_EVENT_NETCTRL_APPCONNECT_SETTING,
    #ifdef SERVICES_SPEECH_ON
        CVI_EVENT_MODEMNG_SPEECHMNG_STARTREC,
        CVI_EVENT_MODEMNG_SPEECHMNG_STOPREC,
        CVI_EVENT_MODEMNG_SPEECHMNG_OPENFRONT,
        CVI_EVENT_MODEMNG_SPEECHMNG_OPENREAR,
        CVI_EVENT_MODEMNG_SPEECHMNG_CLOSESCREEN,
        CVI_EVENT_MODEMNG_SPEECHMNG_OPENSCREEN,
        CVI_EVENT_MODEMNG_SPEECHMNG_EMRREC,
        CVI_EVENT_MODEMNG_SPEECHMNG_PIV,
        CVI_EVENT_MODEMNG_SPEECHMNG_CLOSEWIFI,
        CVI_EVENT_MODEMNG_SPEECHMNG_OPENWIFI,
        CVI_EVENT_MODEMNG_START_SPEECH,
        CVI_EVENT_MODEMNG_STOP_SPEECH
    #endif
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