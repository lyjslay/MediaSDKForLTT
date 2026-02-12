#ifndef __UI_COMMON_H__
#define __UI_COMMON_H__
#include <stdbool.h>
#include "awtk.h"
#include "main_loop/main_loop_simple.h"
#include "cvi_mq.h"
#include "cvi_osal.h"
#include "cvi_eventhub.h"
#include "cvi_log.h"
#include "cvi_storagemng.h"
#include "cvi_recordmng.h"
#ifdef SERVICES_PHOTO_ON
#include "cvi_photomng.h"
#endif
#include "cvi_mode.h"
#include "cvi_filemng_dtcf.h"
#include "cvi_param.h"
#include "cvi_media_init.h"
#include "cvi_media_dump.h"
#include "cvi_filemng_comm.h"
#include "cvi_keymng.h"
#include "cvi_volmng.h"
#include "cvi_liveviewmng.h"
#include "cvi_sys.h"
#include "cvi_usb.h"
#include "cvi_uvc.h"
#include "cvi_net.h"
#include "cvi_gaugemng.h"
#ifdef CONFIG_PWM_ON
#include "cvi_hal_pwm.h"
#endif
#include "cvi_hal_screen.h"
#include "cvi_hal_touchpad.h"
#if defined (ENABLE_VIDEO_MD)
#include "cvi_videomd.h"
#endif
#ifdef SERVICES_ADAS_ON
#include "cvi_adasmng.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define UI_ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))
typedef int32_t  (*CVI_UI_MSGRESULTPROC_FN_PTR)(CVI_EVENT_S* pstEvent);

#define UI_VOICE_MAX_NUM            (8)
#define UI_VOICE_START_UP_IDX       (0)
#define UI_VOICE_START_UP_SRC       "/mnt/system/bin/voice/ANI.wav"
#define UI_VOICE_TOUCH_BTN_IDX      (1)
#define UI_VOICE_TOUCH_BTN_SRC      "/mnt/system/bin/voice/click.wav"
#define UI_VOICE_CLOSE_IDX          (2)
#define UI_VOICE_CLOSE_SRC          "/mnt/system/bin/voice/ANI.wav"
#define UI_VOICE_PHOTO_IDX          (3)
#define UI_VOICE_PHOTO_SRC          "/mnt/system/bin/voice/photo.wav"
#define UI_VOICE_CAR_CLOSING_IDX    (4)
#define UI_VOICE_CAR_CLOSING_SRC    "/mnt/system/bin/voice/distance.wav"
#define UI_VOICE_CAR_LANE_IDX       (5)
#define UI_VOICE_CAR_LANE_SRC       "/mnt/system/bin/voice/lane.wav"
#define UI_VOICE_CAR_MOVING_IDX     (6)
#define UI_VOICE_CAR_MOVING_SRC     "/mnt/system/bin/voice/moving.wav"
#define UI_VOICE_CAR_COLLISION_IDX  (7)
#define UI_VOICE_CAR_COLLISION_SRC  "/mnt/system/bin/voice/collision.wav"

#define MSG_EVENT_ID_INVALID 100

typedef struct tagCVI_UI_MESSAGE_CONTEXT
{
    CVI_MESSAGE_S stMsg;     /**< the message that has been sent*/
    bool bMsgProcessed;  /**< the message sent has been processed or not*/
    CVI_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc;   /**< used to process the response*/
    pthread_mutex_t MsgMutex;  /**< the mutex protect sent msg and bool flag*/
}CVI_UI_MESSAGE_CONTEXT;

#define UI_KEY_BACK      TK_KEY_F1
#define UI_KEY_OK        TK_KEY_F2
#define UI_KEY_UP        TK_KEY_F3
#define UI_KEY_DOWN      TK_KEY_F4

typedef enum cviEVENT_UI_E
{
    CVI_EVENT_UI_TOUCH = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_UI, 0),
    CVI_EVENT_UI_BUTT
} CVI_EVENT_UI_E;

typedef enum cviUIEVENT_COMMON_E {
    EVT_LOW_BATTERY             = EVT_USER_START + 1,
    EVT_NO_SDCARD,
    EVT_SDCARD_NEED_FORMAT,
    EVT_SDCARD_SLOW,
    EVT_SDCARD_CHECKING,
    EVT_SDCARD_ERROR,
    EVT_SDCARD_READ_ONLY,
    EVT_SDCARD_MOUNT_FAILED,
    EVT_FORMAT_PROCESS,
    EVT_FORMAT_FAILED,
    EVT_FORMAT_SUCCESS,
    EVT_OPEN_HOMEPAGE,
    EVT_CLOSE_HOMEPAGE,
    EVT_OPEN_DIRPAGE,
    EVT_CLOSE_DIRPAGE,
    EVT_OPEN_MSGPAGE,
    EVT_CLOSE_MSGPAGE,
    EVT_FILE_ABNORMAL           = EVT_USER_START + 100,
    EVT_NOVIDEO_FILE,
    EVT_NOPHOTO_FILE,
    EVT_DELETE_FILE_CONFIRM,
    EVT_NOTSUPPORT_H265,
} CVI_UIEVENT_COMMON_E;

enum _MSG_EVENT_ID
{
    MSG_EVENT_ID_LOW_BATTERY = 0,
    MSG_EVENT_ID_NO_CARD,
    MSG_EVENT_ID_SDCARD_NEED_FORMAT,
    MSG_EVENT_ID_SDCARD_SLOW,
    MSG_EVENT_ID_SDCARD_CHECKING,
    MSG_EVENT_ID_SDCARD_ERROR,
    MSG_EVENT_ID_SDCARD_READ_ONLY,
    MSG_EVENT_ID_SDCARD_MOUNT_FAILED,
    MSG_EVENT_ID_FORMAT_PROCESS,
    MSG_EVENT_ID_FORMAT_FAILED,
    MSG_EVENT_ID_FORMAT_SUCCESS,
    MSG_EVENT_ID_FILE_ABNORMAL,
    MSG_EVENT_ID_NOVIDEO_FILE,
    MSG_EVENT_ID_NOPHOTO_FILE,
    MSG_EVENT_ID_DELETE_FILE_CONFIRM,
    MSG_EVENT_ID_NOTSUPPORT_H265,
    MSG_EVENT_APP_CONNECT_SUCCESS,
    MSG_EVENT_APP_DISCONNECT,
    MSG_EVENT_ID_SET_DEFAULT_SETTING,
    MSG_EVENT_ID_MAX
};

typedef int32_t  (*KEYGPIOEVENT)(void);

typedef struct cviKEY_GPIO_EVENT {
	CVI_KEYMNG_KEY_IDX_E gpioidx;
	KEYGPIOEVENT longkeyback;
    KEYGPIOEVENT shortkeyback;
} CVI_KEY_GPIO_EVENT;

#define PLAY_SELETE_CUR_FILE  0
#define PLAY_SELETE_NEXT_FILE 1
#define PLAY_SELETE_PRE_FILE 2

//ui_main.c
int32_t  CVI_UIAPP_Start(void);
int32_t  CVI_UIAPP_Stop(void);
void ui_lock(void);
void ui_unlock(void);

//ui_common.c
int32_t  ui_common_SubscribeEvents(void);
bool ui_common_cardstatus(void);
int32_t  CVI_UICOMM_SendAsyncMsg(CVI_MESSAGE_S* pstMsg, CVI_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc);
ret_t ui_open_uvc(void* ctx, event_t* e);
ret_t ui_open_storage(void* ctx, event_t* e);

//ui_home_page.c
ret_t ui_home_open(void* ctx, event_t* e);
ret_t ui_home_close(void);
int32_t  ui_homepage_eventcb(void *argv, CVI_EVENT_S *msg);

//ui_home_photo_page.c
ret_t ui_home_photo_open(void* ctx, event_t* e);
ret_t ui_home_photo_close(void* ctx, event_t* e);
//ui_busying_page.c
ret_t close_busying_page(void);
ret_t open_busying_page(void);

//ui_setting_page.c
ret_t open_menu_window(void* ctx, event_t* e);
ret_t close_menu_window(void* ctx, event_t* e);
ret_t close_option_window(void* ctx, event_t* e);
ret_t open_option_window(void* ctx, event_t* e);
//ui_format_page.c
ret_t open_format_window(void* ctx, event_t* e);
ret_t close_format_window(void* ctx, event_t* e);
//ui_settime_page.c
ret_t open_settime_window(void* ctx, event_t* e);
ret_t close_settime_window(void* ctx, event_t* e);

//ui_wrnmsg_page.c
ret_t open_wrnmsg_window(void* ctx, event_t* e);
ret_t close_wrnmsg_window(void* ctx, event_t* e);
bool wrnmsg_window_isopen(void);
ret_t ui_wrnmsg_update_msglael(const idle_info_t* idle);
ret_t ui_wrnmsg_update_type(uint32_t type);
ret_t ui_wrnmsg_get_type(void);

//ui_dir_page.c
ret_t open_dir_window(void* ctx, event_t* e);
ret_t close_dir_window(void* ctx, event_t* e);
//ui_filelist_page.c
ret_t open_filelist_window(void* ctx, event_t* e);
ret_t close_filelist_window(void* ctx, event_t* e);
void set_filelist_endirs(uint32_t type);
//ui_playback_page.c
ret_t continue_play_file(int32_t  switch_file_flag);
ret_t delete_file_playback(void);
ret_t open_playback_window(void* ctx, event_t* e);
ret_t close_playback_window(void* ctx, event_t* e);
ret_t back_close_window(void);
void set_info_playback_window(int32_t  curfilenum, int32_t  total, uint32_t type);
void playback_add_time(void);
void playback_reset_time(void);
int32_t  ui_playbackpage_eventcb(void *argv, CVI_EVENT_S *msg);

//ui_powercontrol.c
int32_t  UI_POWERCTRL_Init(void);
int32_t  UI_POWERCTRL_Deinit(void);
int32_t  UI_POWERCTRL_PreProcessEvent(CVI_EVENT_S* pstEvent, bool* pbEventContinueHandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
