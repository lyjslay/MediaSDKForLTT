#ifndef _ONVIF_DEVICE_INTERFACE_
#define _ONVIF_DEVICE_INTERFACE_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdio.h> 
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <semaphore.h>

// common
#define DEVICE_COMMON_LENGTH        (32)
#define DEVICE_CMD_LEN              (128)
#define DEVICE_ROUTE_BUF_LEN        (512)
// device info
#define DEVICE_INFO_LENGTH          (32)
#define DEVICE_INFO_MANUFACTURER    "Cvitek Co., Ltd."
#define DEVICE_INFO_MODEL           "1835-IPC"
#define DEVICE_INFO_FIRMWARE_VER    "v1.0.0"
#define DEVICE_INFO_SN              "CVITEK-IPC001" //FIXME: need to get the real sn
#define DEVICE_INFO_HARDWARE_ID     "1.0"
// device scopes
#define DEVICE_SCOPES_COUNT         (9)
#define DEVICE_SCOPE_LENGTH         (128)
#define DEVICE_SCOPE1   "onvif://www.onvif.org/type/NetworkVideoTransmitter"
#define DEVICE_SCOPE2   "onvif://www.onvif.org/type/video_encoder"
#define DEVICE_SCOPE3   "onvif://www.onvif.org/type/ptz"
#define DEVICE_SCOPE4   "onvif://www.onvif.org/hardware/CViTek-1835"
#define DEVICE_SCOPE5   "onvif://www.onvif.org/type/audio_encoder"
#define DEVICE_SCOPE6   "onvif://www.onvif.org/location/city/ShenZhen"
#define DEVICE_SCOPE7   "onvif://www.onvif.org/location/country/China"
#define DEVICE_SCOPE8   "onvif://www.onvif.org/name/IPC"
#define DEVICE_SCOPE9   "onvif://www.onvif.org/Profile/Streaming"
// device support services
#define DEVICE_SERVICE_MAX          (10)
#define DEVICE_SERVICE_LENGTH       (128)
#define DEVICE_SERVICE1 "http://www.onvif.org/ver10/analytics/wsdl"
#define DEVICE_SERVICE2 "http://www.onvif.org/ver10/device/wsdl"
#define DEVICE_SERVICE3 "http://www.onvif.org/ver10/events/wsdl"
#define DEVICE_SERVICE4 "http://www.onvif.org/ver10/imaging/wsdl"
#define DEVICE_SERVICE5 "http://www.onvif.org/ver10/media2/wsdl"
#define DEVICE_SERVICE6 "http://www.onvif.org/ver10/ptz/wsdl"
// device profiles
#define DEVICE_PROFILE_NUM          (1) 
// net info
#define DEVICE_IPv4                 (4)
#define DEVICE_IPv6                 (6)
#define DEVICE_ADAPTER_NAME         "eth0"
#define DEVICE_UUID_LEN             (128)
#define DEVICE_IP_LEN               (128)
// ntp server
#define NTP_SERVER_CN               "ntp.ntsc.ac.cn"    //国家授时中心 NTP 服务器
#define NTP_SERVER_POOL_CN          "cn.pool.ntp.org"   //国际 NTP 快速授时服务 中国片区
#define NTP_SERVER_ALIYUN           "ntp1.aliyun.com"   //阿里云公共 NTP 服务器
// user
#define DEVICE_USERS_MAX            (10)

// video info
#define DEVICE_VIDEO_WIDTH          (1920)
#define DEVICE_VIDEO_HEIGHT         (1080)


/* device infos */
typedef struct _OnvifDeviceScopes_S
{
    int size;
    char scopes[DEVICE_SCOPES_COUNT][DEVICE_SCOPE_LENGTH];
}OnvifDeviceScopes_S;

typedef struct _OnvifProductInfos_S
{
    char manufacturer[DEVICE_INFO_LENGTH];
    char model[DEVICE_INFO_LENGTH];
    char firmwareVersion[DEVICE_INFO_LENGTH];
    char serialNumber[DEVICE_INFO_LENGTH];
    char hardwareID[DEVICE_INFO_LENGTH];
}OnvifProductInfos_S;

typedef struct _OnvifDeviceInfos_S
{
    char ip[DEVICE_INFO_LENGTH];
    char mac[DEVICE_INFO_LENGTH];
    char getway[DEVICE_INFO_LENGTH];
    char hostname[DEVICE_INFO_LENGTH];
    char uuid[DEVICE_UUID_LEN];
}OnvifDeviceInfos_S;


typedef struct _OnvifServices_S
{
    int size;
    char namespaces[DEVICE_SERVICE_MAX][DEVICE_SCOPE_LENGTH];
} OnvifServices_S;

typedef enum _VideoEncType_E
{
    VideoEnc_JPEG = 0,
    VideoEnc_MPEG4 = 1,
    VideoEnc_H264 = 2,
    VideoEnc_H265 = 3
} VideoEnc_E;

typedef enum _VideoH264Profile_E
{
    H264Profile_Baseline = 0,
    H264Profile_Main = 1,
    H264Profile_Extended = 2,
    H264Profile_High = 3
} VideoH264Profile_E;

typedef struct _VideoResolution_S
{
    int Width;
    int Height;
} VideoResolution_S;

typedef struct _VideoH264Cfg_S
{
    int govLength;
    VideoH264Profile_E profile;
} VideoH264Cfg_S;

typedef struct _OnvifVideoEncCfg_S
{
    char name[DEVICE_COMMON_LENGTH];	
    int useCount;
    char token[DEVICE_COMMON_LENGTH];
    VideoEnc_E encoding;
    VideoResolution_S resolution;
    float quality;
    char sessionTimeout[DEVICE_COMMON_LENGTH];
    int gop;
    int frameRateLimit;
    int bitrate;
    VideoEnc_E advanceEncType;
    VideoH264Cfg_S h264;
} OnvifVideoEncCfg_S;

typedef struct _IntRectangle_S
{
    int x;
    int y;
    int width;
    int height;
} IntRectangle_S;

typedef struct _OnvifVideoSrcCfg_S
{
    char name[DEVICE_COMMON_LENGTH];	
    int useCount;
    char token[DEVICE_COMMON_LENGTH];
    char sourceToken[DEVICE_COMMON_LENGTH];
    VideoResolution_S resolution;
    float framerate;
    IntRectangle_S bounds;
} OnvifVideoSrcCfg_S;

typedef enum _TimeType_E
{
    TimeType_Manual = 0,
    TimeType_NTP = 1
} TimeType_E;

typedef struct _SystemCfg_S
{
    TimeType_E timeType;
    int timeZoom;
} SystemCfg_S;

/* user manager */
typedef enum _UserLevel_E 
{
	UserLevel__Administrator = 0,
	UserLevel__Operator = 1,
	UserLevel__User = 2,
	UserLevel__Anonymous = 3,
	UserLevel__Extended = 4
} UserLevel_E;

typedef struct _OnvifUser_S
{
    char name[DEVICE_COMMON_LENGTH];
    char password[DEVICE_COMMON_LENGTH];
    UserLevel_E level;
} OnvifUser;

typedef struct _OnvifUserMng_S
{
    int count;
    OnvifUser *current;
    OnvifUser users[DEVICE_USERS_MAX]; // max users number is 10
} OnvifUserMng_S;


/* system info */
typedef struct _SystemTime_S
{
    int timeZone;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
} SystemTime_S;

typedef struct _SystemStatus_S
{
    pthread_t discover;
    pthread_t webservices;
    pthread_t hellobye;
    int exit_discover;  // 0: not exit 1: exit
    int exit_webservices; // 0: not exit 1: exit
    int exit_hellobye;
    int hello_bye;      // 0: do nothing 1: hello  2: bye 
    sem_t hello_bye_sem;
} SystemStatus_S;


extern OnvifUserMng_S gOnvifUsers;
extern OnvifProductInfos_S gOnvifProductInfo;
extern OnvifDeviceScopes_S gOnvifDevScopes;
extern OnvifDeviceInfos_S gOnvifDevInfo;
extern OnvifServices_S gOnvifServices;
extern OnvifVideoEncCfg_S gOnvifVideoEncCfg;
extern OnvifVideoSrcCfg_S gOnvifVideoSrcCfg;
extern SystemCfg_S gSystemCfg;
extern SystemStatus_S gSystemStatus;

int onvif_register_callback();
int onvif_device_init();
int onvif_product_info_init();
int onvif_device_info_init();
int onvif_device_scopes_init();
int onvif_services_init();
int onvif_video_config_init();
int onvif_audio_config_init();
int onvif_system_config_init();
int onvif_system_status_init();

int onvif_users_init();
int onvif_add_user(OnvifUser user);
int onvif_delete_user(OnvifUser user);
int onvif_set_user(OnvifUser user);

// TODO: Sam, write callback, get cfg from device by callback func
// int get_video_encode_cfg(OnvifVideoEncCfg_S cfg);
// int get_video_source_cfg(OnvifVideoSrcCfg_S cfg);

/* device interfaces, server will callback */
typedef struct _OnvifNetworkCallback_S
{
    struct _OnvifNetworkCallback_S * next; /* Multi-NIC structure */
    char *name; /* NIC name */
    int (*get_name)(char *name[], int *count_adapter);
    int (*get_mtu)(const char *adapter_name, int *mtu);
    int (*set_mtu)(const char *adapter_name, const int mtu);
    int (*get_hwaddr)(const char *adapter_name, unsigned char hwaddr[]);
    int (*set_hwaddr)(const char *adapter_name, const unsigned char hwaddr[]);
    int (*get_ip)(const char *adapter_name, struct sockaddr *addr);
    int (*set_ip)(const char *adapter_name, const struct sockaddr *addr);
    int (*set_dhcp)(const char *adapter_name);
    int (*get_netmask)(const char *adapter_name, struct sockaddr *netmask);
    int (*set_netmask)(const char *adapter_name, const struct sockaddr *netmask);
    int (*get_broadaddr)(const char *adapter_name, struct sockaddr *broadaddr);
    int (*set_broadaddr)(const char *adapter_name, struct sockaddr *broadaddr);
    int (*get_ipv6)(const char *adapter_name, struct sockaddr_in6 *ipv6);
    int (*get_gateway)(const char *adapter_name, struct sockaddr *gateway[], int *count_gateway);
    int (*set_gateway)(const char *adapter_name, struct sockaddr *gateway[], int count_gateway);
    //dynimic DNS
    int (*get_ddns)(const char *adapter_name, struct sockaddr_in6 *gateway6[], int *count_gateway6);
    int (*set_ddns)(const char *adapter_name, struct sockaddr_in6 *gateway6[], int *count_gateway6);
    //static DNS
    int (*get_dns)(char ***search, char ***nameserver, char **domain, int *size_search, int *size_ns);
    int (*set_dns)(char **search, char **nameserver, char *domain, int size_search, int size_ns);
    int (*get_hostname)(const char *adapter_name, struct sockaddr_in6 *gateway6[], int *count_gateway6);
    int (*set_hostname)(const char *adapter_name, struct sockaddr_in6 *gateway6[], int *count_gateway6);
    int (*is_running)(const char *adapter_name, int *is_running);
    int (*is_ipaddr)(const char *ip_addr, int iptype);
    int (*covprefixlen)(struct sockaddr *paddr);
} OnvifNetworkCallback_S;

typedef struct _OnvifVideoEncCallback_S
{
    OnvifVideoEncCfg_S (*get_video_enc)(void);
    int (*set_video_enc)(const OnvifVideoEncCfg_S enc);
    int (*set_ve_name)(const char *name);
    int (*set_ve_useCount)(const int count);
    int (*set_ve_token)(const char *token);
    int (*set_ve_encoding)(const VideoEnc_E enc);
    int (*set_ve_resolution)(const VideoResolution_S reso);
    int (*set_ve_quality)(const float quality);
    int (*set_ve_sessionTimeout)(const char *timeout);
    int (*set_ve_gop)(const int gop);
    int (*set_ve_frameRateLimit)(const int limit);
    int (*set_ve_bitrate)(const int rate);
    int (*set_ve_advanceEncType)(const VideoEnc_E type);
    int (*set_ve_h264)(const VideoH264Cfg_S h264);
} OnvifVideoEncCallback_S;

typedef struct _OnvifSysCfgCallback_S
{
    int (*set_local_time)(const SystemTime_S local);
    int (*get_local_time)(const SystemTime_S *local);
    int (*set_utc_time)(const SystemTime_S utc);
    int (*get_utc_time)(const SystemTime_S *utc);
    int (*sys_reboot)(void);
} OnvifSysCfgCallback_S;

// extern OnvifNetworkCallback_S gOnvifNetworkCallback;

typedef struct _OnvifCallback_S
{
    OnvifNetworkCallback_S network;
    OnvifVideoEncCallback_S videoEnc;
    OnvifSysCfgCallback_S system;
} OnvifCallback_S;

extern OnvifCallback_S gOnvifCallback;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // _ONVIF_DEVICE_INTERFACE_