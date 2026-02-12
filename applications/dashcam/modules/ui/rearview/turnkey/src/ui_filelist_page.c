#include "ui_windowmng.h"
#include "thm_image/thm_image.h"
#include "image_loader/image_loader_stb.h"
#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
#include "cvi_thumbnail_extractor/cvi_thumbnail_extractor.h"
#endif
#include "cvi_filemng_dtcf.h"
#ifdef SERVICES_PLAYER_ON
#include "cvi_playbackmng.h"
#endif
#include "cvi_param.h"

CVI_DTCF_DIR_E aenDirs = 0;
static uint32_t pu32FileObjCnt = 0;

static ret_t on_filelistitem_click(void* ctx, event_t* e)
{
    (void)e;
    (void)ctx;
    widget_t* list_item = WIDGET(ctx);
    int32_t  index = widget_index_of(list_item);
    set_info_playback_window(index, pu32FileObjCnt, aenDirs);
    ui_winmng_startwin(UI_PLAYBACK_PAGE, true);

    return RET_OK;
}

static ret_t on_filelistthm_click(void* ctx, event_t* e)
{
    (void)e;
    (void)ctx;
    thm_image_t* image = THM_IMAGE(ctx);
    int32_t  index = image->fileindex;
    set_info_playback_window(index, pu32FileObjCnt, aenDirs);
    ui_winmng_startwin(UI_PLAYBACK_PAGE, true);

    return RET_OK;
}

static ret_t load_image(const char* filename, bitmap_t* image) {
    ret_t ret = RET_OK;
#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
    CVI_THUMBNAIL_EXTRACTOR_HANDLE_T viewer_handle = NULL;
    ret = CVI_THUMBNAIL_EXTRACTOR_Create(&viewer_handle);
    CVI_THUMBNAIL_PACKET_S packet = {0};
    image_loader_t* loader = image_loader_stb();
    ret = CVI_THUMBNAIL_EXTRACTOR_GetThumbnail(viewer_handle, filename, &packet);
    uint32_t size = packet.size;
    if (size == 0) {
        CVI_LOGE("Get Thumbnail faile!\n");
        CVI_THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
        return RET_FAIL;
    }
    asset_info_t* info = asset_info_create(ASSET_TYPE_IMAGE, ASSET_TYPE_IMAGE_PNG, "name", size);
    return_value_if_fail(info != NULL, RET_OOM);
    memcpy(info->data, packet.data, size);
    ret = image_loader_load(loader, info, image);
    asset_info_destroy(info);
    CVI_THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
    CVI_THUMBNAIL_EXTRACTOR_ClearPacket(&packet);
#endif
    return ret;
}

ret_t filelist_prepare_image_t(uint32_t index, bitmap_t* image)
{
    ret_t ret = RET_OK;
    char filename[MAX_PATH];
    CVI_FILEMNG_GetFileByIndex(index, filename, MAX_PATH);

    ret = load_image(filename, image);
    if (ret != RET_OK) {
        CVI_LOGE("load_image failed!\n");
    }
    return ret;
}

static ret_t on_filelistback_click(void* ctx, event_t* e)
{
    (void)e;
    (void)ctx;

    ui_winmng_finishwin(UI_FILELIST_PAGE);
    ui_winmng_startwin(UI_DIR_PAGE, true);

    return RET_OK;
}

void set_filelist_endirs(uint32_t type)
{
    aenDirs = type;
    return;
}

ret_t open_filelist_window(void* ctx, event_t* e)
{
    widget_t* win = WIDGET(ctx);

    if (win) {
        uint32_t i = 0;
        widget_t* button = widget_lookup(win, "back_button", TRUE);
        CVI_PARAM_FILEMNG_S FileMng;
        CVI_PARAM_GetFileMngParam(&FileMng);
        widget_t* scroll_viewthm = widget_lookup(win, "scroll_viewthm", TRUE);
        scroll_view_set_snap_to_page(scroll_viewthm, TRUE);
        scroll_view_set_move_to_page(scroll_viewthm, TRUE);

        widget_t* title = widget_lookup(win, "title", TRUE);
        widget_on(button, EVT_CLICK, on_filelistback_click, win);
        widget_set_text_utf8(title, FileMng.FileMngDtcf.aszDirNames[aenDirs]);
        CVI_FILEMNG_SetSearchScope(&aenDirs, 1, &pu32FileObjCnt);
        window_manager_get_top_window(window_manager());
        for(i = 0; i < pu32FileObjCnt; i++) {
            static widget_t* list_itemthm = NULL;
            if (i%6 == 0) {
                list_itemthm = list_item_create(scroll_viewthm, 0, 0, scroll_viewthm->w, scroll_viewthm->h);
                widget_set_children_layout(list_itemthm, "default(r=2,c=3,s=2,m=2)");
                widget_set_style_color(list_itemthm, "pressed:bg_color", 0x00000000);
                widget_set_style_color(list_itemthm, "over:border_color", 0x00000000);
            }
            widget_t* image = thm_image_create(list_itemthm, 0, 0, 0, 0);
            thm_image_set_prepare_image(image, filelist_prepare_image_t, i);
            widget_on(image, EVT_CLICK, on_filelistthm_click, image);
        }
        return RET_OK;
    }
    return RET_FAIL;
}

ret_t close_filelist_window(void* ctx, event_t* e)
{
    (void)e;
    (void)ctx;
    aenDirs = 0;
    return RET_OK;
}