/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: header file for Wi-Fi Station component
 * Author: Hisilicon
 * Create: 2018-08-04
 */

/**
 * \file
 * \brief describle the information about WiFi STA component. CNcomment:提供WiFi STA组件相关接口、数据结构信息。CNend
 */

#ifndef __HI_WLAN_H__
#define __HI_WLAN_H__

#include "hi_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*************************** Structure Definition ****************************/
/** < \addtogroup     WLAN_STA */
/** < @{ */  /** < <!-- [WLAN_STA] */

#ifndef HI_WLAN_COMMON_DEF
#define HI_WLAN_COMMON_DEF

#define MAX_SSID_LEN        256    /** < max length of AP's ssid */ /** < CNcomment:SSID最大长度 */
#define BSSID_LEN           17     /** < length of MAC address */ /** < CNcomment:MAC地址长度 */
#define MAX_PSSWD_LEN       64     /** < max length of password */ /** < CNcomment:密码最大长度 */
#define WEP_PSSWD_LEN_5     5
#define WEP_PSSWD_LEN_13    13
#define MIN_CHAN            1
#define MAX_CHAN            14
#define BEACON_INT_MAX      1000
#define BEACON_INT_MIN      33
#define DEFAULT_BEACON_INT  100
#define DEFAULT_CHANNEL     6
#define SOCK_BUF_MAX        1024
#define SOCK_PORT           8001

/** < invalid parameter */
/** < CNcomment:无效的参数 -2 */
#define HI_WLAN_INVALID_PARAMETER           -2

/** < no supported WiFi device found */
/** < CNcomment:没有找到WiFi设备 -3 */
#define HI_WLAN_DEVICE_NOT_FOUND            -3

/** < load driver fail */
/** < CNcomment:加载驱动失败 -4 */
#define HI_WLAN_LOAD_DRIVER_FAIL            -4

/** < run wpa_supplicant process fail */
/** < CNcomment:启动wpa_supplicant失败 -5 */
#define HI_WLAN_START_SUPPLICANT_FAIL       -5

/** < cann't connect to wpa_supplicant */
/** < CNcomment:连接wpa_supplicant失败 -6 */
#define HI_WLAN_CONNECT_TO_SUPPLICANT_FAIL  -6

/** < cann't send command to wpa_supplicant */
/** < CNcomment:发送命令给wpa_supplicant失败 -7 */
#define HI_WLAN_SEND_COMMAND_FAIL           -7

/** < run hostapd process fail */
/** < CNcomment:启动hostapd失败 -8 */
#define HI_WLAN_START_HOSTAPD_FAIL          -8

/** < network security mode type */ /** < CNcomment:AP安全模式类型 */
typedef enum hi_wlan_security_e {
    HI_WLAN_SECURITY_OPEN,          /** < OPEN mode, not encrypted */ /** < CNcomment:OPEN模式 */
    HI_WLAN_SECURITY_WEP,           /** < WEP mode */ /** < CNcomment:WEP模式 */
    HI_WLAN_SECURITY_WPA_WPA2_PSK,  /** < WPA-PSK/WPA2-PSK mode */ /** < CNcomment:WPA-PSK或WPA2-PSK模式 */
    HI_WLAN_SECURITY_WPA_WPA2_EAP,  /** < WPA-EAP/WPA2-EAP mode */ /** < CNcomment:WPA-EAP或WPA2-EAP模式 */
    HI_WLAN_SECURITY_WAPI_PSK,      /** < WAPI-PSK mode */ /** < CNcomment:WAPI-PSK模式 */
    HI_WLAN_SECURITY_WAPI_CERT,     /** < WAPI-CERT mode */ /** < CNcomment:WAPI-CERT模式 */
    HI_WLAN_SECURITY_BUTT,
} hi_wlan_security_e;

#endif /* HI_WLAN_COMMON_DEF */
#define SCAN_CHAN_NUM_MIX 14

typedef struct hi_wlan_sta_scan_cfg_e {
    hi_u32 scan_chan[SCAN_CHAN_NUM_MIX];
    hi_u8   scan_chan_len;
} hi_wlan_sta_scan_cfg_e;

#ifndef HI_WLAN_STA_COMMON_DEF
#define HI_WLAN_STA_COMMON_DEF

#define PIN_CODE_LEN      8                 /** < length of wps pin code */ /** < CNcomment:pin码长度 */

/** < WiFi event type */ /** < CNcomment:WiFi事件类型 */
typedef enum hi_wlan_sta_event_e {
    HI_WLAN_STA_EVENT_DISCONNECTED,         /** < network disconnected event */ /** < CNcomment:网络断开事件 */
    HI_WLAN_STA_EVENT_SCAN_RESULTS_AVAILABLE,    /** < scan done event */ /** < CNcomment:扫描结束事件 */
    HI_WLAN_STA_EVENT_CONNECTING,           /** < try to connect to network event */ /** < CNcomment:正在连接AP事件 */
    HI_WLAN_STA_EVENT_CONNECTED,            /** < network connected event */ /** < CNcomment:连接上AP事件 */
    HI_WLAN_STA_EVENT_SUPP_STOPPED,         /** < supplicant abnormal event */ /** < CNcomment:wpa_supplicant停止事件 */
    HI_WLAN_STA_EVENT_DRIVER_STOPPED,       /** < driver abnormal event */ /** < CNcomment:驱动退出事件 */
    HI_WLAN_P2P_EVENT_PEERS_CHANGED,        /** < p2p status event */ /** < CNcomment:状态变更事件 */
    HI_WLAN_P2P_EVENT_GROUP_STARTED,        /** < p2p group started event */ /** < CNcomment:建立群组事件 */
    HI_WLAN_P2P_EVENT_GROUP_REMOVED,        /** < p2p group removed event */ /** < CNcomment:删除群组事件 */
    HI_WLAN_P2P_EVENT_CONNECTED,            /** < p2p connected event */ /** < CNcomment:设备连接上事件 */
    HI_WLAN_P2P_EVENT_DISCONNECTED,         /** < p2p disconnected event */ /** < CNcomment:设备未连接事件 */
    HI_WLAN_P2P_EVENT_CONNECTION_REQUESTED,       /** < p2p connection requested event */ /** < CNcomment:请求连接事件 */
    HI_WLAN_P2P_EVENT_PERSISTENT_GROUPS_CHANGED,  /** < p2p groups changed event */ /** < CNcomment:群组变更事件 */
    HI_WLAN_P2P_EVENT_INVITATION,           /** < p2p invitation event */ /** < CNcomment:设备邀请事件 */
    HI_WLAN_P2P_EVENT_DEVICE_FOUND,         /** < p2p device found event */ /** < CNcomment:设备发现事件 */
    HI_WLAN_P2P_EVENT_NEGOTIATION_FAILURE,  /** < p2p negotiation failure event */ /** < CNcomment:p2p neg失败事件 */
    HI_WLAN_P2P_EVENT_FORMATION_FAILURE,    /** < p2p formation failure event */ /** < CNcomment:p2p formation失败事件 */
    HI_WLAN_WPS_EVENT_TIMEOUT,              /** < wps timeout event */ /** < CNcomment:p2p wps超时事件 */
    HI_WLAN_WPS_EVENT_OVERLAP,              /** < wps overlap event */ /** < CNcomment:p2p wps设备连接事件 */
    HI_WLAN_STA_EVENT_BUTT,
} hi_wlan_sta_event_e;

/** < WPS method type */ /** < CNcomment:WPS连接类型 */
typedef enum hi_wps_method_e {
    WPS_PBC,           /** < Push Button method */ /** < CNcomment:Push Button方式 */
    WPS_PIN_DISPLAY,   /** < PIN Display method */ /** < CNcomment:PIN Display方式 */
    WPS_PIN_KEYPAD,    /** < PIN Keypad method */ /** < CNcomment:PIN Keypad方式 */
    WPS_PIN_LABEL,     /** < PIN Label method */ /** < CNcomment:PIN Keypad方式 */
    WPS_BUTT,
} hi_wps_method_e;

/** < Callback function of receiving WiFi events */ /** < CNcomment:接收WiFi事件的回调函数 */
typedef hi_void(*hi_wlan_sta_event_callback)(hi_wlan_sta_event_e event, const hi_void *priv_data,
                                             hi_u32 priv_data_size);

#endif /* HI_WLAN_STA_COMMON_DEF */

/** < connection state type */ /** < CNcomment:网络连接状态类型 */
typedef enum hi_wlan_sta_conn_state_e {
    HI_WLAN_STA_CONN_STATUS_DISCONNECTED,   /** < not connected to any network */ /** < CNcomment:网络断开状态 */
    HI_WLAN_STA_CONN_STATUS_CONNECTING,     /** < connecting to a network */ /** < CNcomment:正在连接AP状态 */
    HI_WLAN_STA_CONN_STATUS_CONNECTED,      /** < connected to a network */ /** < CNcomment:连接上AP状态 */
    HI_WLAN_STA_CONN_STATUS_BUTT,
} hi_wlan_sta_conn_state_e;

/** < network information */ /** < CNcomment:AP信息结构体 */
typedef struct hi_wlan_sta_access_point_e {
    hi_char ssid[MAX_SSID_LEN + 1];         /** < AP's SSID */ /** < CNcomment:AP的SSID */
    hi_char bssid[BSSID_LEN + 1];           /** < AP's MAC address */ /** < CNcomment:AP的MAC地址 */
    hi_s32  level;                          /** < AP's signal level, 0 - 100 */ /** CNcomment:AP的信号强度，0 - 100 */
    hi_u32  channel;                        /** < AP's channel number */ /** < CNcomment:AP的信道 */
    hi_wlan_security_e security;            /** < AP's security mode */ /** < CNcomment:AP的安全模式 */
} hi_wlan_sta_access_point_e;

typedef enum hi_wlan_bandwith_e {
    HI_WLAN_BAND_WIDTH_20M,
    HI_WLAN_BAND_WIDTH_10M,
    HI_WLAN_BAND_WIDTH_5M,

    HI_WLAN_BAND_WIDTH_BUTT,
} hi_wlan_bandwith_e;

typedef struct hi_wlan_mode_conf {
    hi_u8   bw_enable;                      /* 1；使能，0：不使能 */
    hi_wlan_bandwith_e   bw_bandwidth;      /* 0\1\2  20 10 5 */
} hi_wlan_mode_conf;

typedef enum hi_wlan_hwmode_e {
    HI_WLAN_HWMODE_11N,
    HI_WLAN_HWMODE_11G,
    HI_WLAN_HWMODE_11B,

    HI_WLAN_HW_MODE_BUTT,
} hi_wlan_hwmode_e;

typedef enum {
    HI_WLAN_NO_PROTECTION,
    HI_WLAN_PROTECTION_OPTIONAL,
    HI_WLAN_PROTECTION_REQUIRED,

    HI_WLAN_PROTECTION_BUTT,
} hi_wlan_pmf_mode;

typedef enum {
    HI_WLAN_WPS_PBC,
    HI_WLAN_WPS_PIN,

    HI_WLAN_WPS_BUTT,
} hi_wlan_wps_method;

/** < network configuration */ /** < CNcomment:需要连接的AP配置 */
typedef struct hi_wlan_sta_config_s {
    hi_char   ssid[MAX_SSID_LEN + 1];       /** < network's SSID */ /** < CNcomment:AP的SSID */
    hi_char   bssid[BSSID_LEN + 1];         /** < network's MAC address */ /** < CNcomment:AP的MAC地址 */
    hi_wlan_security_e security;            /** < network's security mode */ /** < CNcomment:AP的安全模式 */
    hi_char   psswd[MAX_PSSWD_LEN + 1];     /** < network's password, if not OPEN mode */ /** < CNcomment:密码 */
    hi_bool   hidden_ssid;                  /** < whether network hiddens it's SSID */ /** < CNcomment:AP是否是隐藏SSID */
    hi_wlan_mode_conf  bw_sta_config;       /** < network's Narrow band suppout and  speed */
    hi_wlan_hwmode_e     hw_mode;           /** < set Operation mode n\g\b */

    hi_wlan_pmf_mode  pmf_mode;
    hi_wlan_wps_method  wps_method;
    hi_char wps_pin[PIN_CODE_LEN + 1];
    hi_u32 promis_enable;
} hi_wlan_sta_config_s;

/** < network status information */ /** < CNcomment:网络连接状态信息 */
typedef struct hi_wlan_sta_conn_status_e {
    hi_wlan_sta_conn_state_e state;         /** < connection state */ /** < CNcomment:网络的连接状态 */
    hi_wlan_sta_access_point_e ap;          /** < network information which connected or connecting */
    /** < CNcomment:连接上或者正在连接的AP信息 */
} hi_wlan_sta_conn_status_e;

/** < @} */  /** < <!-- ==== Structure Definition End ==== */

/******************************* API Declaration *****************************/
/** < \addtogroup     WLAN_STA */
/** < @{ */  /** < <!-- [WLAN_STA] */

/**
\brief: Initialize STA.CNcomment:初始化STA CNend
\attention \n
\param    N/A.CNcomment:无 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\see \n
::hi_wlan_sta_init
*/
HI_OPEN_API hi_s32 hi_wlan_sta_init(hi_void);

/**
\brief: Deintialize STA.CNcomment:去初始化STA CNend
\attention \n
\param  N/A.CNcomment:无 CNend
\retval N/A.CNcomment:无 CNend
\see \n
::hi_wlan_sta_deinit
*/
HI_OPEN_API hi_void hi_wlan_sta_deinit(hi_void);

/**
\brief: Open WiFi STA device.CNcomment:打开WiFi STA设备 CNend
\attention \n
\param[out] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[in] nameBufSize  parameter ifname length.CNcomment:ifanme的大小, 如: strlen(ifname)
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_DEVICE_NOT_FOUND
\retval  ::HI_WLAN_LOAD_DRIVER_FAIL
\see \n
::HI_WLAN_STA_Open
*/
HI_OPEN_API hi_s32 hi_wlan_sta_open(hi_char *ifname, hi_u32 name_bug_size, hi_wlan_sta_config_s *sta_cfg);

/**
\brief: Close WiFi STA device.CNcomment:关闭WiFi STA设备 CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\see \n
::hi_wlan_sta_close
*/
HI_OPEN_API hi_s32 hi_wlan_sta_close(const hi_char *ifname);

/**
\brief: Start WiFi STA.CNcomment:启动WiFi STA CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[in] pfnEventCb  call back function that receives events.CNcomment:接收事件的回调函数 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_START_SUPPLICANT_FAIL
\retval  ::HI_WLAN_CONNECT_TO_SUPPLICANT_FAIL
\see \n
::hi_wlan_sta_start
*/
HI_OPEN_API hi_s32 hi_wlan_sta_start(const hi_char *ifname, hi_wlan_sta_event_callback event_cb);

/**
\brief: Stop WiFi STA.CNcomment:停用WiFi STA CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\see \n
::hi_wlan_sta_stop
*/
HI_OPEN_API hi_s32 hi_wlan_sta_stop(const hi_char *ifname);

/**
\brief: Start to scan.CNcomment:开始扫描 CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_SEND_COMMAND_FAIL
\see \n
::hi_wlan_sta_start_scan
*/
HI_OPEN_API hi_s32 hi_wlan_sta_start_scan(const hi_char *ifname);

/**
\brief: Start to scan.CNcomment:指定信道扫描 CNend
\attention \n
\param[in] scan_cfg  hi_wlan_sta_scan_cfg_e.CNcomment:hi_wlan_sta_scan_cfg_e结构体 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_SEND_COMMAND_FAIL
\see \n
::hi_wlan_sta_start_scan
*/
HI_OPEN_API hi_s32 hi_wlan_sta_start_chan_scan(hi_wlan_sta_scan_cfg_e *scan_cfg);

/**
\brief: Get scan results.CNcomment:获取扫描到的AP CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[out] pstApList AP list.CNcomment: 保存扫描到的AP列表 CNend
\param[inout] pstApNum  number of APs.CNcomment: AP列表中AP的数量 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_SEND_COMMAND_FAIL
\see \n
::hi_wlan_sta_get_scan_results
*/
HI_OPEN_API hi_s32 hi_wlan_sta_get_scan_results(const hi_char *ifname, hi_wlan_sta_access_point_e *ap_list,
    hi_u32 *ap_num);

/**
\brief: Connect to AP.CNcomment:开始连接AP CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[in] pstStaCfg  AP configuration try to connect.CNcomment:需要连接的AP的信息 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_SEND_COMMAND_FAIL
\see \n
::hi_wlan_sta_connect
*/
HI_OPEN_API hi_s32 hi_wlan_sta_connect(const hi_char *ifname, hi_wlan_sta_config_s *sta_cfg);

/**
\brief: Disconnect to AP.CNcomment:断开连接 CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_SEND_COMMAND_FAIL
\see \n
::hi_wlan_sta_disconnect
*/
HI_OPEN_API hi_s32 hi_wlan_sta_disconnect(const hi_char *ifname);

/**
\brief: Get current network connection status.CNcomment:获得当前的连接状态信息 CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[out] pstConnStatus network connection status.CNcomment:保存连接状态信息 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_SEND_COMMAND_FAIL
\see \n
::hi_wlan_sta_get_connection_status
*/
HI_OPEN_API hi_s32 hi_wlan_sta_get_connection_status(const hi_char *ifname, hi_wlan_sta_conn_status_e *conn_status);

/**
\brief: WPS connect to AP.CNcomment:开始连接WPS AP CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[in] wps_method  WPS method.CNcomment:WPS方法 CNend
\param[in] pstPin  Pin code if WPS method is PIN.CNcomment:WPS pin码 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_SEND_COMMAND_FAIL
\see \n
::HI_WLAN_STA_StartWps
*/
HI_OPEN_API hi_s32 hi_wlan_sta_start_wps(const hi_char *ifname, hi_wlan_sta_config_s *sta_cfg);

/**
\brief: Get local WiFi MAC address.CNcomment:获取本地WiFi MAC地址 CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[out] pstMac  MAC address of local WiFi.CNcomment:保存本地WiFi MAC地址 CNend
\param[in] macBufSize  parameter ifname length.CNcomment:ifname的大小, 大小一般固定为17 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\see \n
::hi_wlan_sta_get_mac_address
*/
HI_OPEN_API hi_s32 hi_wlan_sta_get_mac_address(const hi_char *ifname, hi_char *mac, hi_u8 mac_buf_size);

/** < @} */  /** < <!-- ==== API Declaration End ==== */
/*************************** Structure Definition ****************************/
/** < \addtogroup     WLAN_AP */
/** < @{ */  /** < <!-- [WLAN_AP] */

/** < network security mode type */ /** < CNcomment:AP安全模式类型 */ /** < AP's configuration */
typedef struct hi_wlan_ap_config_s {
    hi_char   ssid[MAX_SSID_LEN + 1];       /** < network's SSID */ /** < CNcomment:SSID */
    hi_s32    channel;                      /** < network's channel */ /** < CNcomment:信道号 */
    hi_wlan_security_e security;            /** < network's security mode */ /** < CNcomment:安全模式 */
    hi_char   psswd[MAX_PSSWD_LEN + 1];     /** < network's password, if not OPEN mode */ /** < CNcomment:密码 */
    hi_bool   hidden_ssid;                  /** < whether network hiddens it's SSID */ /** < CNcomment:是否隐藏SSID */
    hi_u32    beacon_int;                   /** < Beacon interval in kus (1.024 ms) (default: 100; range 15..65535) */
    hi_char   hw_mode;                      /** < set Operation mode */
    hi_wlan_bandwith_e   bw_bandwidth;      /** < set bandwidth */
} hi_wlan_ap_config_s;

/** < @} */  /** < <!-- ==== Structure Definition End ==== */

/******************************* API Declaration *****************************/
/** < \addtogroup     WLAN_AP */
/** < @{ */  /** < <!-- [WLAN_AP] */

/**
\brief: Initialize SoftAP.CNcomment:初始化SoftAP CNend
\attention \n
\param    N/A.CNcomment:无 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\see \n
::hi_wlan_ap_init
*/
HI_OPEN_API hi_s32 hi_wlan_ap_init(hi_void);

/**
\brief: Deintialize SoftAP.CNcomment:去初始化SoftAP CNend
\attention \n
\param  N/A.CNcomment:无 CNend
\retval N/A.CNcomment:无 CNend
\see \n
::hi_wlan_ap_deinit
*/
HI_OPEN_API hi_void hi_wlan_ap_deinit(hi_void);

/**
\brief: Open WiFi SoftAP device.CNcomment:打开WiFi SoftAP设备 CNend
\attention \n
\param[out] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[in] nameBufSize  parameter ifname length.CNcomment:ifanme的大小, 如: strlen(ifname)
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_DEVICE_NOT_FOUND
\retval  ::HI_WLAN_LOAD_DRIVER_FAIL
\see \n
::hi_wlan_ap_open
*/
HI_OPEN_API hi_s32 hi_wlan_ap_open(hi_char *ifname, hi_u32 name_buf_size, hi_wlan_bandwith_e bw);

/**
\brief: Close WiFi SoftAP device.CNcomment:关闭WiFi SoftAP设备 CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\see \n
::hi_wlan_ap_close
*/
HI_OPEN_API hi_s32 hi_wlan_ap_close(const hi_char *ifname);

/**
\brief: start SoftAP with configuration.CNcomment:开启SoftAP CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[in] pstApCfg  AP's configuration.CNcomment:AP的配置参数 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_START_HOSTAPD_FAIL
\see \n
::hi_wlan_ap_start
*/
HI_OPEN_API hi_s32 hi_wlan_ap_start(const hi_char *ifname, hi_wlan_ap_config_s *ap_cfg);

/**
\brief: Stop SoftAP.CNcomment:关闭SoftAP CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\see \n
::hi_wlan_ap_stop
*/
HI_OPEN_API hi_s32 hi_wlan_ap_stop(const hi_char *ifname);

/**
\brief: Set SoftAP.CNcomment:设置SoftAP CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[in] pstApCfg  AP's configuration.CNcomment:AP的配置参数 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\retval  ::HI_WLAN_START_HOSTAPD_FAIL
\see \n
::hi_wlan_ap_setsoftap
*/
HI_OPEN_API hi_s32 hi_wlan_ap_setsoftap(const hi_char *ifname, hi_wlan_ap_config_s *ap_cfg);

/**
\brief: Get local WiFi MAC address.CNcomment:获取本地WiFi MAC地址 CNend
\attention \n
\param[in] ifname  WiFi network interface name.CNcomment:WiFi网络接口名, 如: wlan0 CNend
\param[out] pstMac  MAC address of local WiFi.CNcomment:保存本地WiFi MAC地址 CNend
\param[in] macBufSize  parameter ifname length.CNcomment:ifname的大小, 大小一般固定为17 CNend
\retval  ::HI_SUCCESS
\retval  ::HI_FAILURE
\retval  ::HI_WLAN_INVALID_PARAMETER
\see \n
::hi_wlan_ap_getmacaddress
*/
HI_OPEN_API hi_s32 hi_wlan_ap_getmacaddress(const hi_char *ifname, hi_char *pstMac, hi_u8 mac_buf_size);

/** < @} */  /** < <!-- ==== API Declaration End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* __HI_WLAN_STA_H__ */
