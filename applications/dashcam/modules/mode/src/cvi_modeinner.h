#ifndef __CVI_MODEINNER_H__
#define __CVI_MODEINNER_H__

#include <stdio.h>
#include <pthread.h>

#include "cvi_log.h"
#include "cvi_osal.h"
#include "cvi_mq.h"
#include "cvi_mapi.h"
#include "cvi_mode.h"
#include "cvi_media_init.h"
#include "cvi_media_osd.h"
#ifdef SERVICES_LIVEVIEW_ON
#include "cvi_liveviewmng.h"
#endif
#include "cvi_param.h"
#include "cvi_recordmng.h"
#ifdef SERVICES_PHOTO_ON
#include "cvi_photomng.h"
#endif
#ifdef SERVICES_PLAYER_ON
#include "cvi_playbackmng.h"
#endif
#include "cvi_storagemng.h"
#include "cvi_eventhub.h"

#ifdef SERVICES_LIVEVIEW_ON
#include "cvi_volmng.h"
#endif
#include "cvi_powercontrol.h"
#ifdef CONFIG_GSENSOR_ON
#include "cvi_gsensormng.h"
#endif
#ifdef CONFIG_WIFI_ON
#include "cvi_wifimng.h"
#endif
#ifdef SERVICES_SPEECH_ON
#include "cvi_speechmng.h"
#endif

#ifdef SERVICES_ADAS_ON
#include "cvi_adasmng.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define MODE_ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

/** mode name */
#define MODEEMNG_STATE_BASE                  "Base"
#define MODEEMNG_STATE_REC                   "Movie"
#define MODEEMNG_STATE_PHOTO                 "Photo"
#define MODEEMNG_STATE_PLAYBACK              "PlayBack"
#define MODEEMNG_STATE_UVC                   "UVC"
#define MODEEMNG_STATE_USB_STORAGE           "UsbStorage"
#define MODEEMNG_STATE_UPGRADE               "Upgrade"
#define MODEEMNG_STATE_UPDATE                "Update"
#define MODEEMNG_STATE_LAPSE                 "LapseTime"
#define MODEEMNG_STATE_BOOTFIRST                 "BootFirst"
#define MODEEMNG_STATE_USBMENU                 "UsbMenu"
/** NULL pointer check */
#define MODEMNG_CHECK_POINTER(ptr,errcode,string)\
do{\
    if(NULL == ptr)\
     {\
        CVI_LOGE("%s NULL pointer\n\n",string);\
        return errcode;\
     }\
  }while(0)

/** function ret value check */
#define MODEMNG_CHECK_RET(ret,errcode,errstring)\
do{\
    if(0 != ret)\
    {\
        CVI_LOGE("%s failed, s32Ret(0x%08X)\n\n", errstring, ret);\
        return errcode;\
    }\
  }while(0)

/** message proc function parameter check */
#define MODEMNG_CHECK_MSGPROC_FUNC_PARAM(argv,pStateID,Message,InProgress)\
do{\
    if(NULL == argv || NULL == pStateID || NULL == Message)\
    {\
        CVI_LOGE("parameter argv or pStateID or Message NULL\n\n");\
        CVI_MUTEX_LOCK(CVI_MODEMNG_GetModeCtx()->Mutex);\
        InProgress = false;\
        CVI_MUTEX_UNLOCK(CVI_MODEMNG_GetModeCtx()->Mutex);\
        return CVI_PROCESS_MSG_RESULTE_OK;\
    }\
  }while(0)

/**check ret value, unlock mutex when error */
#define MODEMNG_CHECK_CHECK_RET_WITH_UNLOCK(retvalue,errcode,errstring)\
do{\
    if(0 != retvalue)\
    {\
        CVI_LOGE("%s failed, s32Ret(0x%08X)\n\n", errstring, retvalue);\
        CVI_MUTEX_UNLOCK(CVI_MODEMNG_GetModeCtx()->Mutex);\
        return errcode;\
    }\
  }while(0)

/**check init, unlock mutex when error */
#define MODEMNG_CHECK_CHECK_INIT(retvalue,errcode,errstring)\
do{\
    if(0 == retvalue)\
    {\
        CVI_LOGE("%s failed, s32Ret(0x%08X)\n\n", errstring, retvalue);\
        return errcode;\
    }\
  }while(0)

/** Expression Check Without Return */
#define MODEMNG_CHECK_EXPR_WITHOUT_RETURN(expr, errstring) \
    do {                                                      \
        if (expr) {                                        \
            CVI_LOGE("[%s] failed\n", errstring);                \
        }                                                     \
    } while (0)

int32_t CVI_MODEMNG_PublishEvent(CVI_MESSAGE_S* pstMsg, int32_t s32Result);

/** update statemng status */
#define CVI_MODEMNG_UPDATESTATUS(pstMsg,Result,bProgress)\
do{\
    CVI_MUTEX_LOCK(CVI_MODEMNG_GetModeCtx()->Mutex);\
    CVI_MODEMNG_GetModeCtx()->bInProgress = bProgress;\
    CVI_MUTEX_UNLOCK(CVI_MODEMNG_GetModeCtx()->Mutex);\
    CVI_MODEMNG_PublishEvent(pstMsg, Result);\
  }while(0)

CVI_MODEMNG_S* CVI_MODEMNG_GetModeCtx(void);

int32_t CVI_MODEMNG_BaseStateInit(void);
int32_t CVI_MODEMNG_BaseStatesDeinit(void);
int32_t CVI_MODEMNG_MovieStatesInit(const CVI_STATE_S* pstBase);
int32_t CVI_MODEMNG_MovieStatesDeinit(void);
int32_t CVI_MODEMNG_PlaybackStatesInit(const CVI_STATE_S* pstBase);
int32_t CVI_MODEMNG_PlaybackStatesDeinit(void);
int32_t CVI_MODEMNG_MonitorStatusNotify(CVI_EVENT_S* pstMsg);
int32_t CVI_MODEMNG_ResetMovieMode(CVI_PARAM_CFG_S *Param);
int32_t CVI_MODEMNG_InitStorage(void);
int32_t CVI_MODEMNG_StopRec(void);
void CVI_MODEMNG_InitFilemng(void);
int32_t CVI_MODEMNG_DeintStorage(void);
int32_t CVI_MODEMNG_UpDateStatesInit(const CVI_STATE_S* pstBase);
int32_t CVI_MODEMNG_UpDateStatesDeinit(void);
int32_t CVI_MODEMNG_UvcStatesInit(const CVI_STATE_S* pstBase);
int32_t CVI_MODEMNG_UvcStatesDeinit(void);
int32_t CVI_MODEMNG_StorageStatesInit(const CVI_STATE_S* pstBase);
int32_t CVI_MODEMNG_StorageStatesDeinit(void);

bool CVI_MODEMNG_SetCardState(uint32_t u32CardState);
int32_t CVI_MODEMNG_SetEmrState(bool en);
int32_t CVI_MODEMNG_StartMemoryBufferRec(void);
int32_t CVI_MODEMNG_StopMemoryBufferRec(void);
void CVI_MODEMNG_SetMediaVencFormat(int32_t value);
int32_t CVI_MODEMNG_Format(char *labelname);
int32_t CVI_MODEMNG_LiveViewSwitch(uint32_t viewwin);
int32_t CVI_MODEMNG_OpenUvcMode(void);
int32_t CVI_MODEMNG_CloseUvcMode(void);
int32_t CVI_MODEMNG_ContextInit(const CVI_MODEMNG_CONFIG_S* pstModemngCfg);

#ifdef SERVICES_PHOTO_ON
int32_t CVI_MODEMNG_PhotoStatesInit(const CVI_STATE_S* pstBase);
int32_t CVI_MODEMNG_PhotoStatesDeinit(void);
#endif

int32_t CVI_MODEMNG_SetCurModeMedia(CVI_WORK_MODE_E CurMode);
int32_t CVI_MODEMNG_BootFirstStatesInit(const CVI_STATE_S* pstBase);
int32_t CVI_MODEMNG_BootFirstStatesDeinit(void);
int32_t CVI_MODEMNG_UsbMenuStatesInit(const CVI_STATE_S* pstBase);
int32_t CVI_MODEMNG_UsbMenuStatesDeinit(void);

#ifdef SERVICES_SPEECH_ON
int32_t CVI_SPEECHMNG_StartSpeech(void);
int32_t CVI_MODEMNG_StopEventSpeech(void);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __CVI_MODEINNER__ */
