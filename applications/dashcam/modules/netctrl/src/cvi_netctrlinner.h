#ifndef __CVI_NETCTRLINNER_H__
#define __CVI_NETCTRLINNER_H__
#include <stdbool.h>
#include "thttpd.h"
#include "libhttpd.h"
#include "cvi_mode.h"
#include "cvi_eventhub.h"
#include "cvi_ae.h"
#include <sys/socket.h>
#include "cvi_net.h"
#include "cvi_netctrl.h"
#include "cvi_netctrlinner.h"
#include "md5.h"
#include "cvi_storagemng.h"
#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
#include "cvi_thumbnail_extractor/cvi_thumbnail_extractor.h"
#endif
#ifdef COMPONENTS_PLAYER_ON
#include "cvi_player/cvi_player.h"
#endif
#include "cvi_dtcf.h"
#if CONFIG_PWM_ON
#include "cvi_hal_pwm.h"
#endif
#ifdef SERVICES_PLAYER_ON
#include "cvi_player_service.h"
#endif
#include "cvi_timedtask.h"
#include "cvi_hal_wifi.h"
#include "cvi_media_init.h"
#include "cvi_netctrlinner.h"
#include "cvi_system.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifndef PTHREAD_MUTEX_INITIALIZER
# define PTHREAD_MUTEX_INITIALIZER \
  { { 0, 0, 0, 0, 0, { __PTHREAD_SPINS } } }
#endif

#define CVI_WIFIAPP_OK 0
#define CVI_WIFIAPP_FALSE -1
#define CVI_VERSION_NUM "CVITEK20210901"

typedef enum {
	WIFI_SD_FORMAT_FAIL = -1,
	WIFI_SD_FORMAT_SUCCESS = 0,
    WIFI_SD_FORMAT_INIT = 1
} WIFI_SD_STATUS_E;

#define LBUFFSIZE (128)
#define BUFFSIZE (256)
#define XBUFFSIZE (512)
#define OBUFFSIZE (1024)
#define TBUFFSIZE (4096)
#define CVI_NET_WIFI_SSID_LEN (32)
#define CVI_NET_WIFI_PASS_LEN (26)
#define TIMECOUNTMAX (10)

typedef struct tagPDT_NETCTRL_MESSAGE_CONTEXT
{
    CVI_MESSAGE_S stMsg;     /**< the message that has been sent*/
    bool bMsgProcessed;  /**< the flag of message has been processed or not*/
    pthread_mutex_t MsgMutex;  /**< lock*/
} PDT_NETCTRL_MESSAGE_CONTEXT;

int32_t CVI_NETCTRLINNER_SendSyncMsg(CVI_MESSAGE_S* pstMsg, int* ps32Result);
int32_t CVI_NETCTRLINNER_InitCMDSocket(void);
int32_t CVI_NETCTRLINNER_DeInitCMDSocket(void);
int32_t CVI_NETCTRLINNER_TimeoutRest(void);
int32_t CVI_NETCTRLINNER_APPConnetState(void);
int32_t CVI_NETCTRLINNER_GetSdState(void);
void CVI_NETCTRLINNER_SetSdState(int32_t value);
int32_t CVI_NETCTRLINNER_SubscribeEvents(void);
void CVI_NETCTRLINNER_ScanFile(void);
int32_t CVI_NETCTRLINNER_InitTimer(void);
int32_t CVI_NETCTRLINNER_DeInitTimer(void);
int32_t CVI_NETCTRLINNER_StartTimer(void);
int32_t CVI_NETCTRLINNER_StopTimer(void);
void CVI_NETCTRLINNER_UiUpdate(void);
void CVI_NETCTRLINNER_Enablecheckconnect(void *privData, struct timespec *now);
void CVI_NETCTRLINNER_CMDRegister(void);
void* CVI_NETCTRLINNER_SocketXml(void *arg);
void* CVI_NETCTRLINNER_SocketCgi(void *arg);
void* CVI_NETCTRLINNER_SocketcJson(void *arg);
int32_t CVI_NETCTRLINNER_GetFlagFile(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif