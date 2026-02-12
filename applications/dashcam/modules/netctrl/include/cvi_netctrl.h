#include <stdbool.h>

#ifndef __CVI_NETCTRL_H__
#define __CVI_NETCTRL_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum _cvi_NET_EVENT_E
{
    CVI_EVENT_NETCTRL_CONNECT = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_NETCTRL, 0),
    CVI_EVENT_NETCTRL_UIUPDATE,
    CVI_EVENT_NETCTRL_APPCONNECT_SUCCESS,
    CVI_EVENT_NETCTRL_APPDISCONNECT,
    CVI_EVENT_NETCTRL_APPCONNECT_SETTING,
    CVI_EVENT_NETCTRL_BUIT
} CVI_NET_EVENT_E;

typedef enum {
	WIFI_APP_DISCONNECT = 0,
	WIFI_APP_CONNECTTED,
} WIFI_APP_CONNECT_E;

typedef enum {
	WIFI_APP_SETTING_OUT = 0,
	WIFI_APP_SETTING_IN,
} WIFI_APP_SETTING_E;

int32_t CVI_NETCTRL_Init(void);
int32_t CVI_NETCTRL_DeInit(void);
int32_t CVI_NETCTRL_NetToUiConnectState(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif