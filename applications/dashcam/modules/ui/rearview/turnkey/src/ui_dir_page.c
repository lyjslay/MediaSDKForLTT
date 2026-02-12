#include "ui_windowmng.h"
#include "cvi_filemng_dtcf.h"
#ifdef SERVICES_PLAYER_ON
#include "cvi_playbackmng.h"
#endif
#include "cvi_param.h"
#include "cvi_mode.h"

CVI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT];

static ret_t on_diritem_click(void* ctx, event_t* e)
{
    (void)e;
    if (CVI_MODEMNG_GetInProgress() == true) {
        return RET_OK;
    }

    widget_t* list_item = WIDGET(ctx);
    int32_t  index = widget_index_of(list_item);

    set_filelist_endirs(enDirs[index]);
    ui_winmng_startwin(UI_FILELIST_PAGE, false);

    return RET_OK;
}

static ret_t on_dirback_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    ui_winmng_finishwin(UI_DIR_PAGE);

    CVI_MESSAGE_S Msg;
    Msg.topic = CVI_EVENT_MODEMNG_MODESWITCH;
    Msg.arg1 = CVI_WORK_MODE_MOVIE;
    CVI_MODEMNG_SendMessage(&Msg);

    return RET_OK;
}

ret_t open_dir_window(void* ctx, event_t* e)
{
    widget_t* win = WIDGET(ctx);

    if (win) {
        uint32_t i = 0, j = 0;
        widget_t* scroll_view = widget_lookup(win, "scroll_view", TRUE);
        widget_t* button = widget_lookup(win, "back_button", TRUE);
        CVI_PARAM_FILEMNG_S FileMng;
        CVI_PARAM_GetFileMngParam(&FileMng);
        widget_on(button, EVT_CLICK, on_dirback_click, win);

        for(i = 0; i < DTCF_DIR_BUTT; i++) {
            if(0 < strnlen(FileMng.FileMngDtcf.aszDirNames[i], CVI_DIR_LEN_MAX)) {
                enDirs[j] = i;
                widget_t* list_item = list_item_create(scroll_view, 0, 0, 0, 0);
                widget_set_text_utf8(list_item, FileMng.FileMngDtcf.aszDirNames[i]);
                widget_on(list_item, EVT_CLICK, on_diritem_click, list_item);
                j++;
            }
        }
        return RET_OK;
    }

    return RET_FAIL;
}

ret_t close_dir_window(void* ctx, event_t* e)
{
    (void)e;
    (void)ctx;
    return RET_OK;
}