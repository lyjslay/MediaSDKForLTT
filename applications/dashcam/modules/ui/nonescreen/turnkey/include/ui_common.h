#ifndef __UI_COMMON_H__
#define __UI_COMMON_H__
#include <stdbool.h>
#include "cvi_mq.h"
#include "cvi_osal.h"
#include "cvi_eventhub.h"
#include "cvi_log.h"
#include "cvi_storagemng.h"
#include "cvi_recordmng.h"
#include "cvi_mode.h"
#include "cvi_filemng_dtcf.h"
#include "cvi_param.h"
#include "cvi_media_init.h"
#include "cvi_media_dump.h"
#include "cvi_filemng_comm.h"
#include "cvi_keymng.h"
//#include "cvi_volmng.h"
#include "cvi_sys.h"
#include "cvi_usb.h"
#include "cvi_uvc.h"
#include "cvi_gaugemng.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define UI_ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))
typedef int32_t  (*CVI_UI_MSGRESULTPROC_FN_PTR)(CVI_EVENT_S* pstEvent);

#define UI_VOICE_MAX_NUM            (4)
#define UI_VOICE_START_UP_IDX       (0)
#define UI_VOICE_START_UP_SRC       "/mnt/system/bin/voice/ANI.wav"
#define UI_VOICE_TOUCH_BTN_IDX      (1)
#define UI_VOICE_TOUCH_BTN_SRC      "/mnt/system/bin/voice/touch.wav"
#define UI_VOICE_CLOSE_IDX          (2)
#define UI_VOICE_CLOSE_SRC          "/mnt/system/bin/voice/ANI.wav"
#define UI_VOICE_PHOTO_IDX          (3)
#define UI_VOICE_PHOTO_SRC          "/mnt/system/bin/voice/photo.wav"

#define EVT_USER_START 0x2000

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

typedef struct tagCVI_UI_MESSAGE_CONTEXT
{
    CVI_MESSAGE_S stMsg;     /**< the message that has been sent*/
    bool bMsgProcessed;  /**< the message sent has been processed or not*/
    CVI_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc;   /**< used to process the response*/
    pthread_mutex_t MsgMutex;  /**< the mutex protect sent msg and bool flag*/
}CVI_UI_MESSAGE_CONTEXT;

int32_t  CVI_UIAPP_Start(void);
int32_t  CVI_UIAPP_Stop(void);
int32_t  ui_common_SubscribeEvents(void);
int32_t  UI_POWERCTRL_PreProcessEvent(CVI_EVENT_S* pstEvent, bool* pbEventContinueHandle);
int32_t  CVI_UICOMM_SendAsyncMsg(CVI_MESSAGE_S* pstMsg, CVI_UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
