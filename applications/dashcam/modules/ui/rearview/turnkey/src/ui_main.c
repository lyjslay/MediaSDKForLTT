#include <sys/prctl.h>
#include <pthread.h>
#include "awtk_app_conf.h"
#include "ui_common.h"
#include "cvi_sysutils.h"
#include "tslib_thread.h"
#include "../res/assets.inc"
#include "scroll_view/children_layouter_list_view.h"
#include "gif_image/gif_image.h"
#include "ui_windowmng.h"

static cvi_osal_task_handle_t      ui_task;
static bool s_buiInit = false;
static pthread_mutex_t             g_uiSysmutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

static void UI_VOLQUEUE_Init(void)
{
    int32_t  s32Ret;
    CVI_MEDIA_SYSHANDLE_S SysHandle = CVI_MEDIA_GetCtx()->SysHandle;
    CVI_VOICEPLAY_CFG_S stVoicePlayCfg = {0};

    stVoicePlayCfg.stAoutOpt.hAudDevHdl = SysHandle.aohdl;

    stVoicePlayCfg.u32MaxVoiceCnt = UI_VOICE_MAX_NUM;
    CVI_VOICEPLAY_VOICETABLE_S astVoiceTab[UI_VOICE_MAX_NUM]=
    {
        {UI_VOICE_START_UP_IDX,UI_VOICE_START_UP_SRC},
        {UI_VOICE_TOUCH_BTN_IDX,UI_VOICE_TOUCH_BTN_SRC},
        {UI_VOICE_CLOSE_IDX,UI_VOICE_CLOSE_SRC},
        {UI_VOICE_PHOTO_IDX,UI_VOICE_PHOTO_SRC},
#ifdef SERVICES_ADAS_ON
        {UI_VOICE_CAR_CLOSING_IDX,UI_VOICE_CAR_CLOSING_SRC},
        {UI_VOICE_CAR_LANE_IDX,UI_VOICE_CAR_LANE_SRC},
        {UI_VOICE_CAR_MOVING_IDX,UI_VOICE_CAR_MOVING_SRC},
        {UI_VOICE_CAR_COLLISION_IDX,UI_VOICE_CAR_COLLISION_SRC},
#endif
    };
    stVoicePlayCfg.pstVoiceTab = astVoiceTab;
    s32Ret = CVI_VOICEPLAY_Init(&stVoicePlayCfg);
    if (s32Ret != 0) {
        CVI_LOGE("CVI_VOICEPLAY_Init failed!\n");
    }
}

void ui_lock(void)
{
    pthread_mutex_lock(&g_uiSysmutex);
}

void ui_unlock(void)
{
    pthread_mutex_unlock(&g_uiSysmutex);
}

static ret_t ui_touchhook(const event_queue_req_t* e)
{
    if (e->event.type == EVT_POINTER_DOWN) {
        CVI_EVENT_S stEvent = {0};
        stEvent.topic = CVI_EVENT_UI_TOUCH;
        CVI_EVENTHUB_Publish(&stEvent);
    }
    return 0;
}

static int32_t  inline_ext_widgets_init(void) {
  widget_factory_t* f = widget_factory();

  FACTORY_TABLE_BEGIN(s_ext_widgets)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_RICH_TEXT, rich_text_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_RICH_TEXT_VIEW, rich_text_view_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_COLOR_PICKER, (tk_create_t)color_picker_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_COLOR_COMPONENT, (tk_create_t)color_component_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_SCROLL_VIEW, (tk_create_t)scroll_view_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_LIST_VIEW, (tk_create_t)list_view_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_LIST_VIEW_H, (tk_create_t)list_view_h_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_LIST_ITEM, (tk_create_t)list_item_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_SCROLL_BAR, (tk_create_t)scroll_bar_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_SCROLL_BAR_DESKTOP, (tk_create_t)scroll_bar_create_desktop)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_SCROLL_BAR_MOBILE, (tk_create_t)scroll_bar_create_mobile)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_SLIDE_VIEW, slide_view_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_SLIDE_INDICATOR, slide_indicator_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_SLIDE_INDICATOR_ARC, slide_indicator_create_arc)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_KEYBOARD, keyboard_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_LANG_INDICATOR, lang_indicator_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_CANDIDATES, candidates_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_TIME_CLOCK, (tk_create_t)time_clock_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_GAUGE, gauge_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_GAUGE_POINTER, gauge_pointer_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_TEXT_SELECTOR, text_selector_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_SWITCH, switch_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_IMAGE_ANIMATION, image_animation_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_PROGRESS_CIRCLE, progress_circle_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_SVG_IMAGE, svg_image_create)
  FACTORY_TABLE_ENTRY(WIDGET_TYPE_GIF_IMAGE, (tk_create_t)gif_image_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_CANVAS_WIDGET, (tk_create_t)canvas_widget_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_IMAGE_VALUE, image_value_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_SLIDE_MENU, slide_menu_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_MUTABLE_IMAGE, mutable_image_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_MLEDIT, mledit_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_LINE_NUMBER, line_number_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_HSCROLL_LABEL, hscroll_label_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_COMBO_BOX_EX, combo_box_ex_create)
  // FACTORY_TABLE_ENTRY(WIDGET_TYPE_DRAGGABLE, draggable_create)

  // FACTORY_TABLE_ENTRY("guage", gauge_create)
  // FACTORY_TABLE_ENTRY("guage_pointer", gauge_pointer_create)

// #ifdef WITH_WIDGET_VPAGE
//   FACTORY_TABLE_ENTRY(WIDGET_TYPE_VPAGE, vpage_create)
// #endif /*WITH_WIDGET_VPAGE*/

// #ifdef TK_FILE_BROWSER_VIEW_H
//   FACTORY_TABLE_ENTRY(WIDGET_TYPE_FILE_BROWSER_VIEW, file_browser_view_create)
// #endif /*TK_FILE_BROWSER_VIEW_H*/
  FACTORY_TABLE_END()

  children_layouter_factory_t* chf = children_layouter_factory();

  children_layouter_factory_register(chf, CHILDREN_LAYOUTER_LIST_VIEW,
                                     children_layouter_list_view_create);

  return widget_factory_register_multi(f, s_ext_widgets);
}

static int32_t  gui_app_start(int32_t  w, int32_t  h)
{
    int32_t  lcd_w = w;
    int32_t  lcd_h = h;
    ui_lock();
#ifdef NDEBUG
    log_set_log_level(LOG_LEVEL_INFO);
#else
    log_set_log_level(LOG_LEVEL_DEBUG);
#endif /*NDEBUG*/

#ifdef CONFIG_TOUCHPAD_ON
    tslib_settouchhook(ui_touchhook);
#endif

    log_debug("APP_RES_ROOT = %s\n",APP_RES_ROOT);
    tk_init(lcd_w, lcd_h, APP_TYPE, APP_NAME, APP_RES_ROOT);

#if defined(WITH_LCD_PORTRAIT)
    if (lcd_w > lcd_h) {
        tk_set_lcd_orientation(LCD_ORIENTATION_90);
    }
#endif /*WITH_LCD_PORTRAIT*/

#ifdef WITH_LCD_LANDSCAPE
    if (lcd_w < lcd_h) {
        tk_set_lcd_orientation(LCD_ORIENTATION_270);
    }
#endif /*WITH_LCD_PORTRAIT*/

    system_info_set_default_font(system_info(), APP_DEFAULT_FONT);
    assets_init();

#ifndef WITHOUT_EXT_WIDGETS
    inline_ext_widgets_init();
#endif /*WITHOUT_EXT_WIDGETS*/
    log_info("Build at: %s %s\n", __DATE__, __TIME__);

#ifdef ENABLE_CURSOR
    window_manager_set_cursor(window_manager(), "cursor");
#endif /*ENABLE_CURSOR*/

    //on_change_locale
    locale_info_change(locale_info(), "zh", "CN");
    ui_unlock();
    ui_winmng_init();
    UI_POWERCTRL_Init();
    UI_VOLQUEUE_Init();
    tk_run();

    return 0;
}

static void start_uiapp(void *arg)
{
    gui_app_start(LCD_WIDTH, LCD_HEIGHT);
}

#if 0
int32_t  CVI_UIAPP_Start(void)
{
    if (s_buiInit == false) {
        ui_common_SubscribeEvents();
/* option: to control fb options
 * - bit[0]: if true, double buffer
 * - bit[1]: if true, fb on vpss not vo
 * - bit[2:3]: 0:ARGB8888, 1:ARGB1555, 2:ARGB4444
 */
        int32_t  option = 0;
#if defined(FB_DUAL_BUFFER)
        option |= 0x1;
#endif
#if defined(FB_ARGB4444)
        option |= (0x2 << 2);
#elif defined(FB_ARGB1555)
        option |= (0x1 << 2);
#endif
        char cmd[64] = {0};
        snprintf(cmd, sizeof(cmd), "/mnt/system/ko/loadfbko.sh %d", option);
        cvi_system(cmd);

        //start ui task
        cvi_osal_task_attr_t ta;
        ta.name = "cvi_ui";
        ta.entry = start_uiapp;
        ta.param = NULL;
        ta.priority = CVI_OSAL_PRI_RT_LOWEST;
        ta.detached = false;
        int32_t  rc = cvi_osal_task_create(&ta, &ui_task);
        if (rc != CVI_OSAL_SUCCESS) {
            CVI_LOGE("cvi_ui task create failed, %d\n", rc);
            return -1;
        }
        s_buiInit = true;
    } else {
        CVI_LOGI("ui already init\n");
    }

    return 0;
}

int32_t  CVI_UIAPP_Stop(void)
{
    if (s_buiInit == true) {
        tk_quit();
        UI_POWERCTRL_Deinit();
        int32_t  rc = cvi_osal_task_join(ui_task);
        if (rc != CVI_OSAL_SUCCESS) {
            CVI_LOGE("lv_event task join failed, %d\n", rc);
            return -1;
        }
        cvi_osal_task_destroy(&ui_task);
        s_buiInit = false;
    }

    return 0;
}
#else
int32_t  CVI_UIAPP_Start(void)
{
    {
        //ui_common_SubscribeEvents();
/* option: to control fb options
 * - bit[0]: if true, double buffer
 * - bit[1]: if true, fb on vpss not vo
 * - bit[2:3]: 0:ARGB8888, 1:ARGB1555, 2:ARGB4444
 */
        int32_t  option = 0;
#if defined(FB_DUAL_BUFFER)
        option |= 0x1;
#endif
#if defined(FB_ARGB4444)
        option |= (0x2 << 2);
#elif defined(FB_ARGB1555)
        option |= (0x1 << 2);
#endif
        char cmd[64] = {0};
        snprintf(cmd, sizeof(cmd), "/mnt/system/ko/loadfbko.sh %d", option);
        cvi_system(cmd);
    }


    int s32Ret = 0;

    /** insmod touchpad driver */
    s32Ret = cvi_insmod("/mnt/system/ko/3rd/gt9xx.ko",NULL);
    if(0 != s32Ret)
    {
        printf("insmod touchpad:failed, errno(%d)\n", errno);
        return -1;
    }

}

int32_t  CVI_UIAPP_Stop(void)
{
}
#endif
