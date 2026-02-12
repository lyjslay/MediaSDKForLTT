#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "ui_common.h"
#include "cvi_filemng_dtcf.h"
#include "cvi_media_init.h"
#include "cvi_mode.h"
#ifdef SERVICES_PLAYER_ON
#include "cvi_playbackmng.h"
#include "cvi_player_service.h"
#endif
#include "ui_windowmng.h"

static int32_t  s_curfilenum = 0;
static int32_t  s_totalfile = 0;
static uint32_t s_totaltime = 0;
static uint32_t s_curdirtype = 0;

enum _MSG_WARN_ID
{
    MSG_WARN_ID_Normal = 0,
    MSG_WARN_ID_Delete_File,
    MSG_WARN_ID_Buttom,
};

ret_t back_close_window(void)
{
#ifdef SERVICES_PLAYER_ON
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = CVI_MEDIA_GetCtx()->SysServices.PsHdl;
    CVI_PLAYER_SERVICE_Stop(ps_handle);
    set_filelist_endirs(s_curdirtype);
    ui_winmng_finishwin(UI_PLAYBACK_PAGE);
    ui_winmng_startwin(UI_FILELIST_PAGE, false);
#endif
    return 0;
}

static bool playback_prompt_wrnmsg(int32_t  warnmsg)
{
    if(warnmsg == MSG_WARN_ID_Delete_File) {
        if (s_totalfile == 0) {
            back_close_window();
            return true;
        } else {
            ui_wrnmsg_update_type(MSG_EVENT_ID_DELETE_FILE_CONFIRM);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            return true;
        }
    } else {
        if (s_totalfile == 0) {
            if (s_curdirtype != DTCF_DIR_PHOTO_FRONT && s_curdirtype != DTCF_DIR_PHOTO_REAR) {
                ui_wrnmsg_update_type(MSG_EVENT_ID_NOVIDEO_FILE);
                ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            } else {
                ui_wrnmsg_update_type(MSG_EVENT_ID_DELETE_FILE_CONFIRM);
                ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            }
            return true;
        }
    }
    return false;
}

static void playback_update_time()
{
#ifdef SERVICES_PLAYER_ON
    widget_t* win = window_manager_get_top_window(window_manager());
    widget_t* time = widget_lookup(win, "time", TRUE);
    widget_t* slider = widget_lookup(win, "slider", TRUE);
    uint32_t curmin, cursec = 0;
    uint32_t _totaltime = s_totaltime;
    uint32_t min = _totaltime/60;
    uint32_t sec;
    static double last_time = 0;
    char totaltime[64] = {};

    if (min != 0) {
        sec = _totaltime - min*60;
    } else {
        sec = _totaltime;
    }

    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
    CVI_PLAYER_PLAY_INFO info;
    if (CVI_PLAYER_SERVICE_GetPlayInfo(ps_handle, &info) != 0) {
        CVI_LOGE("Player get play info failed");
    }

    if (0.0 == info.duration_sec) {
        snprintf(totaltime, sizeof(totaltime), "%02d:%02d/%02d:%02d", min, sec, 0, 0);
        widget_set_text_utf8(time, totaltime);
        slider_set_max(slider, _totaltime);
        slider_set_value(slider, 0);

        return;
    } else if (((_totaltime-info.duration_sec) < 1.0) && ((info.duration_sec-last_time) == 0)) {
        curmin = _totaltime/60;
        if (curmin != 0) {
            cursec = _totaltime - curmin*60;
        } else {
            cursec = _totaltime;
        }
    } else {
        curmin = info.duration_sec/60;
        if (curmin != 0) {
            cursec = info.duration_sec - curmin*60;
        } else {
            cursec = info.duration_sec;
        }
    }
    last_time = info.duration_sec;

    snprintf(totaltime, sizeof(totaltime), "%02d:%02d/%02d:%02d", min, sec, curmin, cursec);
    widget_set_text_utf8(time, totaltime);
    slider_set_max(slider, _totaltime);
    slider_set_value(slider, info.duration_sec);
#endif
}

static bool check_jpg_file_complete(const char*filename, uint32_t file_type)
{
    if (file_type == DTCF_DIR_PHOTO_FRONT || file_type == DTCF_DIR_PHOTO_REAR) {
        FILE *fp = fopen(filename, "rb");
        if (NULL == fp) {
            CVI_LOGF("fail to open\n");
            return false;
        }
        unsigned char read_temp_buf[8] = {0};
        uint32_t seek_loc = 0;
        fseek(fp, 0, SEEK_SET);
        fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
        seek_loc = (((read_temp_buf[4] << 8) | read_temp_buf[5]) + 4);
        fseek(fp, seek_loc, SEEK_SET);
        fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
        seek_loc = (read_temp_buf[4]<<24 | read_temp_buf[5]<<16 | read_temp_buf[6]<<8 | read_temp_buf[7]<<0) + seek_loc + 4;
        fseek(fp, seek_loc, SEEK_SET);
        fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
        if ((read_temp_buf[0] == 0xFF) && (read_temp_buf[1] == 0xD9) && (read_temp_buf[2] == 0xFF) && (read_temp_buf[3] == 0xD9)) {
            fclose(fp);
            return true;
        } else {
            fclose(fp);
            return false;
        }
    }
    return true;
}

static int32_t  play_file()
{
    int32_t  s32Ret = 0;
#ifdef SERVICES_PLAYER_ON
    char filename[MAX_PATH];
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
    CVI_PLAYER_MEDIA_INFO_S info = {};

    s32Ret = CVI_FILEMNG_GetFileByIndex(s_curfilenum, filename, MAX_PATH);
    if (s32Ret != 0) {
        CVI_LOGE("error CVI_FILEMNG_GetFileByIndex\n");
        return s32Ret;
    }
    CVI_LOGE("filename = %s\n", filename);
    widget_t* win = window_manager_get_top_main_window(window_manager());
    widget_t* filenametitle = widget_lookup(win, "filename", TRUE);
    widget_t* filetime = widget_lookup(win, "time", TRUE);
    widget_t* slider = widget_lookup(win, "slider", TRUE);

    if (s_curdirtype == DTCF_DIR_PHOTO_FRONT || s_curdirtype == DTCF_DIR_PHOTO_REAR) {
        widget_set_visible(slider, FALSE);
        widget_set_visible(filetime, FALSE);
    } else {
        widget_set_visible(slider, TRUE);
        widget_set_visible(filetime, TRUE);
    }

    s32Ret = CVI_PLAYER_SERVICE_SetInput(ps_handle, filename);
    if (s32Ret != 0) {
        CVI_LOGE("Player set input %s failed", filename);
        return s32Ret;
    }

    s32Ret = CVI_PLAYER_SERVICE_GetMediaInfo(ps_handle, &info);
    if (s32Ret != 0) {
        CVI_LOGE("Player Get MediaInfo %s failed", filename);
        return s32Ret;
    }

    if (s_curdirtype == DTCF_DIR_PHOTO_FRONT || s_curdirtype == DTCF_DIR_PHOTO_REAR) {
        bool jpg_flag = check_jpg_file_complete(filename, s_curdirtype);
        if (false == jpg_flag) {
            remove(filename);
            continue_play_file(PLAY_SELETE_NEXT_FILE);
            s32Ret = -1;
            return s32Ret;
        }
    }

    #ifdef SERVICES_Player_Subvideo_ON
    CVI_PLAYER_SERVICE_SetPlaySubStreamFlag(ps_handle, true);
    #endif

    if (strcmp(info.video_codec, "hevc")) {
        s_totaltime = (uint32_t)info.duration_sec;
        widget_set_text_utf8(filenametitle, strrchr(filename, '/') + 1);
        CVI_PLAYER_SERVICE_Play(ps_handle);
    } else {
        ui_wrnmsg_update_type(MSG_EVENT_ID_NOTSUPPORT_H265);
        ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    }
#endif
    return s32Ret;
}

ret_t continue_play_file(int32_t  switch_file_flag)
{
#ifdef SERVICES_PLAYER_ON
    bool ret = false;
    int32_t  s32Ret = -1;
    ret = playback_prompt_wrnmsg(MSG_WARN_ID_Normal);
    if (ret == true) {
        return RET_OK;
    }

    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = CVI_MEDIA_GetCtx()->SysServices.PsHdl;
    switch (switch_file_flag) {
        case PLAY_SELETE_CUR_FILE:
            CVI_PLAYER_SERVICE_Play(ps_handle);
            break;
        case PLAY_SELETE_NEXT_FILE:
            CVI_PLAYER_SERVICE_Stop(ps_handle);
            s_curfilenum--;
            if(s_curfilenum < 0) {
                s_curfilenum = s_totalfile - 1;
            }
            s32Ret = play_file();
            if(s32Ret != 0) {
                ui_wrnmsg_update_type(MSG_EVENT_ID_FILE_ABNORMAL);
                ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            }
            playback_update_time();
            break;
        case PLAY_SELETE_PRE_FILE:
            CVI_PLAYER_SERVICE_Stop(ps_handle);
            s_curfilenum++;
            if(s_curfilenum > (s_totalfile - 1)) {
                s_curfilenum = 0;
            }
            s32Ret = play_file();
            if(s32Ret != 0) {
                ui_wrnmsg_update_type(MSG_EVENT_ID_FILE_ABNORMAL);
                ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            }
            playback_update_time();
            break;
        default:
            CVI_LOGI("ERROR PARM\n");
            break;
    }
#endif
    return RET_OK;
}

void playback_add_time(void)
{
    playback_update_time();
}

void playback_reset_time(void)
{
    if (s_curdirtype == DTCF_DIR_PHOTO_FRONT || s_curdirtype == DTCF_DIR_PHOTO_REAR) {
        return;
    }
    playback_update_time();
    continue_play_file(PLAY_SELETE_NEXT_FILE);
}

static ret_t on_startplay_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    bool ret = false;
    ret = playback_prompt_wrnmsg(MSG_WARN_ID_Normal);
    if (ret == true) {
        return RET_OK;
    }
    continue_play_file(PLAY_SELETE_CUR_FILE);
    return RET_OK;
}

static ret_t on_stopplay_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
#ifdef SERVICES_PLAYER_ON
    bool ret = false;
    ret = playback_prompt_wrnmsg(MSG_WARN_ID_Normal);
    if (ret == true) {
        return RET_OK;
    }
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = CVI_MEDIA_GetCtx()->SysServices.PsHdl;
    CVI_PLAYER_SERVICE_Stop(ps_handle);
#endif
    return RET_OK;
}

static ret_t on_preplay_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    continue_play_file(PLAY_SELETE_PRE_FILE);
    return RET_OK;
}

static ret_t on_nextplay_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    continue_play_file(PLAY_SELETE_NEXT_FILE);
    return RET_OK;
}

static ret_t on_back_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    back_close_window();
    return RET_OK;
}

static ret_t on_pauseplay_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
#ifdef SERVICES_PLAYER_ON
    bool ret = false;
    ret = playback_prompt_wrnmsg(MSG_WARN_ID_Normal);
    if (ret == true) {
        return RET_OK;
    }
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = CVI_MEDIA_GetCtx()->SysServices.PsHdl;
    CVI_PLAYER_SERVICE_Pause(ps_handle);
#endif
    return RET_OK;
}

static int32_t  playback_update_fileinfo(void)
{
    int32_t  s32Ret = 0;
    CVI_DTCF_DIR_E aenDirs = s_curdirtype;
    uint32_t totalfile = 0;
    s32Ret = CVI_FILEMNG_SetSearchScope(&aenDirs, 1, &totalfile);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    s_totalfile = totalfile;

    if (s_curfilenum >= s_totalfile)
    {
        s_curfilenum = 0;
    }

    CVI_LOGD("CurGrpIdx:%u, GroupCnt:%u \n", s_curfilenum, s_totalfile);

    if (0 == s_totalfile)
    {
        CVI_LOGD("file count:%d \n", s_totalfile);
        return -1;
    }

    return 0;
}

static int32_t  playback_delete_curfile(void)
{
    int32_t  s32Ret = 0;
    char filename[MAX_PATH];

    CVI_FILEMNG_GetFileByIndex(s_curfilenum, filename, MAX_PATH);
    CVI_LOGE("delete cur filename = %s\n", filename);
    if (1 < s_totalfile)
    {
        s32Ret = CVI_FILEMNG_RemoveFile(filename);
        CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        cvi_async();

        s_totalfile--;
        if (s_curfilenum >= s_totalfile)
        {
            s_curfilenum = s_totalfile - 1;
        }

        s32Ret = playback_update_fileinfo();
        CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        play_file();
    }
    else
    {
        s_totalfile = 0;
        s_curfilenum = 0;
        s32Ret = CVI_FILEMNG_RemoveFile(filename);
        CVI_APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        cvi_async();
        playback_prompt_wrnmsg(MSG_WARN_ID_Delete_File);
        return 0;
    }
    return 0;
}

static ret_t on_deletecurfile_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    playback_prompt_wrnmsg(MSG_WARN_ID_Delete_File);
    return RET_OK;
}

ret_t delete_file_playback(void)
{
    CVI_MAPI_DISP_HANDLE_T disp_handle = CVI_MEDIA_GetCtx()->SysHandle.dispHdl;
    CVI_MAPI_DISP_ClearBuf(disp_handle);
    playback_delete_curfile();
    return 0;
}

static ret_t on_deleteallfile_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;

    return RET_OK;
}

static ret_t on_cancel_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    window_close(win);
    ui_winmng_finishwin(UI_PLAYBACK_PAGE);

    return RET_OK;
}

static ret_t init_listview_widget(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);
    widget_t* win = widget_get_window(widget);
    (void)ctx;

    if (widget->name != NULL) {
        const char* name = widget->name;
        if (tk_str_eq(name, "list_item_deletecur")) {
            widget_on(widget, EVT_CLICK, on_deletecurfile_click, win);
        } else if (tk_str_eq(name, "list_item_deleteall")) {
            widget_on(widget, EVT_CLICK, on_deleteallfile_click, win);
        } else if (tk_str_eq(name, "list_item_cancel")) {
            widget_on(widget, EVT_CLICK, on_cancel_click, win);
        }
    }

    return RET_OK;
}

static void init_children_listview(widget_t* widget) {
  widget_foreach(widget, init_listview_widget, widget);
}

static ret_t on_delete_click(void* ctx, event_t* e)
{
    (void)e;
    (void)ctx;
#ifdef SERVICES_PLAYER_ON
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = CVI_MEDIA_GetCtx()->SysServices.PsHdl;
    CVI_PLAYER_SERVICE_Stop(ps_handle);
    bool ret = false;
    ret = playback_prompt_wrnmsg(MSG_WARN_ID_Delete_File);
    if (ret == true) {
        return RET_OK;
    }

    CVI_MAPI_DISP_HANDLE_T disp_handle = CVI_MEDIA_GetCtx()->SysHandle.dispHdl;
    CVI_MAPI_DISP_ClearBuf(disp_handle);
    playback_delete_curfile();
#endif
    return RET_OK;
}

static ret_t on_sliderprogress_changed(void* ctx, event_t* e)
{
#ifdef SERVICES_PLAYER_ON
    (void)e;
    widget_t* widget = WIDGET(ctx);
    uint32_t slider_value = widget_get_value(widget);

    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
    if (CVI_PLAYER_SERVICE_Seek(ps_handle, slider_value*1000) != 0) {
        CVI_LOGE("Player seek locate failed");
    }
#endif
    return RET_OK;
}

static ret_t init_widget(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);
    widget_t* win = widget_get_window(widget);
    (void)ctx;

    if (widget->name != NULL) {
        const char* name = widget->name;
        if (tk_str_eq(name, "palyback_pre_button")) {
            widget_on(widget, EVT_CLICK, on_preplay_click, win);
        } else if (tk_str_eq(name, "palyback_start_button")) {
            widget_on(widget, EVT_CLICK, on_startplay_click, win);
        } else if (tk_str_eq(name, "palyback_stop_button")) {
            widget_on(widget, EVT_CLICK, on_stopplay_click, win);
        } else if (tk_str_eq(name, "palyback_next_button")) {
            widget_on(widget, EVT_CLICK, on_nextplay_click, win);
        } else if (tk_str_eq(name, "palyback_back_button")) {
            widget_on(widget, EVT_CLICK, on_back_click, win);
        } else if (tk_str_eq(name, "palyback_pause_button")) {
            widget_on(widget, EVT_CLICK, on_pauseplay_click, win);
        } else if (tk_str_eq(name, "slider")) {
            widget_on(widget, EVT_VALUE_CHANGING, on_sliderprogress_changed, widget);
        } else if (tk_str_eq(name, "palyback_delete_button")) {
            widget_on(widget, EVT_CLICK, on_delete_click, win);
        }
    }

    return RET_OK;
}

static void init_children_widget(widget_t* widget) {
  widget_foreach(widget, init_widget, widget);
}

void set_info_playback_window(int32_t  curfilenum, int32_t  total, uint32_t type)
{
    s_curfilenum = curfilenum;
    s_curdirtype = type;
    s_totalfile = total;
    return;
}

ret_t open_playback_window(void* ctx, event_t* e)
{
    (void)e;
    int32_t  s32Ret = -1;
    widget_t* win = WIDGET(ctx);
    if (win) {
        window_manager_get_top_window(window_manager());
        init_children_widget(win);
        s32Ret = play_file();
        if(s32Ret != 0) {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FILE_ABNORMAL);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
        }
        return RET_OK;
    }

    return RET_FAIL;
}

ret_t close_playback_window(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {

    }
    s_curfilenum = 0;
    s_curdirtype = 0;
    s_totalfile = 0;
    return RET_OK;
}


int32_t  ui_playbackpage_eventcb(void *argv, CVI_EVENT_S *msg)
{
    switch(msg->topic) {
        case CVI_EVENT_MODEMNG_MODEOPEN:
        {
            ui_winmng_startwin(UI_DIR_PAGE, true);
            break;
        }
        case CVI_EVENT_MODEMNG_MODECLOSE:
        {
            ui_winmng_closeallwin();
            break;
        }
        case CVI_EVENT_MODEMNG_CARD_REMOVE:
        {
            CVI_MESSAGE_S Msg = {0};
            Msg.topic = CVI_EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = CVI_WORK_MODE_MOVIE;
            CVI_MODEMNG_SendMessage(&Msg);
            break;
        }
        case CVI_EVENT_MODEMNG_PLAYBACK_PLAY:
            CVI_LOGD("CVI_EVENT_MODEMNG_PLAYBACK_PLAY\n");
            break;
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
            CVI_LOGI("CVI_EVENT_MODEMNG_PLAYBACK_ABNORMAL\n");
            break;
        }
        default:
            break;
    }

    return 0;
}
