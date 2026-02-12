#ifndef __CVI_PARAM_H__
#define __CVI_PARAM_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "cvi_stg.h"
#include "cvi_hal_wifi.h"
#include "cvi_hal_screen.h"
#include "cvi_mapi.h"
#include "cvi_mapi_ao.h"
#include "cvi_filemng_dtcf.h"
#include "cvi_filemng_comm.h"
#include "cvi_usb.h"
#include "cvi_keymng.h"
#include "cvi_gsensormng.h"
#include "cvi_gaugemng.h"
#ifdef SERVICES_SPEECH_ON
#include "cvi_speechmng.h"
#endif
#ifdef SERVICES_LIVEVIEW_ON
#include "cvi_liveview.h"
#endif
#ifdef SERVICES_ADAS_ON
#include "cvi_adas_service.h"
typedef CVI_ADAS_SERVICE_MODEL_ATTR_S CVI_PARAM_ADAS_MODEL_ATTR_S;
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#pragma pack(push)
#pragma pack(8)

/* global definitions */
#define CVI_PARAM_MAGIC_START           (0xAAAAAAAA)
#define CVI_PARAM_MAGIC_END             (0x55555555)
#define CVI_PARAM_RAW_ON

/* error numbers */
#define CVI_PARAM_EINVAL          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PARAM, CVI_EINVAL)                  /**<Invalid argument */
#define CVI_PARAM_ENOTINIT        CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PARAM, CVI_ENOINIT)                 /**<Not inited */
#define CVI_PARAM_EINITIALIZED    CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PARAM, CVI_EINITIALIZED)            /**<Already Initialized */


/* menu definitions */
#define CVI_PARAM_MENU_ITEM_MAX         (10)
#define CVI_PARAM_MENU_ITEM_DESC_LEN    (64)

#define MAX_PLAYER_INSTANCES            (1)
#define MAX_VPROC_CNT                   (16)
#define MAX_VENC_CNT                    (MAX_CAMERA_INSTANCES * 4)
#define MAX_APP_RTSP_CNT                (1)
#define MAX_RTSP_CNT                    (MAX_CAMERA_INSTANCES + MAX_APP_RTSP_CNT)
#define CVI_PARAM_WND_NUM_MAX           (16)
#define CVI_MEDIA_MAX_OSD_IDX           (16)
#define CVI_PARAM_WORK_MODE_MAX         (4)
#define CVI_PARAM_WORK_MODE_LEN         (32)
#define APP_RTSP_NAME                   "cvi_cam_phone_app"

//menu param
typedef enum _CVI_MEDIA_VIDEO_SIZE_E
{
    CVI_MEDIA_VIDEO_SIZE_1280X720P25,
    CVI_MEDIA_VIDEO_SIZE_1280X720P30,
    CVI_MEDIA_VIDEO_SIZE_1920X1080P25,
    CVI_MEDIA_VIDEO_SIZE_1920X1080P30,
    CVI_MEDIA_VIDEO_SIZE_2304X1296P25,
    CVI_MEDIA_VIDEO_SIZE_2304X1296P30,
    CVI_MEDIA_VIDEO_SIZE_2560X1440P25,
    CVI_MEDIA_VIDEO_SIZE_2560X1440P30,
    CVI_MEDIA_VIDEO_SIZE_2560X1600P25,
    CVI_MEDIA_VIDEO_SIZE_2560X1600P30,
    CVI_MEDIA_VIDEO_SIZE_3840X2160P25,
    CVI_MEDIA_VIDEO_SIZE_3840X2160P30,
    CVI_MEDIA_PHOTO_SIZE_2560X1440P,
    CVI_MEDIA_PHOTO_SIZE_1920X1080P,
    CVI_MEDIA_PHOTO_SIZE_1280X720P,
    CVI_MEDIA_VIDEO_SIZE_BUIT
} CVI_MEDIA_VIDEO_SIZE_E;

typedef enum _CVI_MEDIA_VIDEO_LOOP_E
{
    CVI_MEDIA_VIDEO_LOOP_1MIN,
    CVI_MEDIA_VIDEO_LOOP_3MIN,
    CVI_MEDIA_VIDEO_LOOP_5MIN,
    CVI_MEDIA_VIDEO_LOOP_BUIT
} CVI_MEDIA_VIDEO_LOOP_E;

typedef enum _CVI_MEDIA_PHOTO_SIZE_E
{
    CVI_MEDIA_PHOTO_SIZE_VGA,
    CVI_MEDIA_PHOTO_SIZE_2M,
    CVI_MEDIA_PHOTO_SIZE_5M,
    CVI_MEDIA_PHOTO_SIZE_8M,
    CVI_MEDIA_PHOTO_SIZE_10M,
    CVI_MEDIA_PHOTO_SIZE_12M,
    CVI_MEDIA_PHOTO_SIZE_BUIT
} CVI_MEDIA_PHOTO_SIZE_E;

typedef enum _CVI_MEDIA_PWM_BRI_E
{
    CVI_MEDIA_PWM_BRI_LOW,
    CVI_MEDIA_PWM_BRI_MID,
    CVI_MEDIA_PWM_BRI_HIGH,
    CVI_MEDIA_PWM_BRI_BUIT
} CVI_MEDIA_PWM_BRI_E;

typedef enum _CVI_MEDIA_VIDEO_VENCTYPE_E
{
    CVI_MEDIA_VIDEO_VENCTYPE_H264,
    CVI_MEDIA_VIDEO_VENCTYPE_H265,
    CVI_MEDIA_VIDEO_VENCTYPE_BUIT
} CVI_MEDIA_VIDEO_VENCTYPE_E;

typedef enum _CVI_MEDIA_VIDEO_LAPSETIME_E
{
    CVI_MEDIA_VIDEO_LAPSETIME_OFF,
    CVI_MEDIA_VIDEO_LAPSETIME_1S,
    CVI_MEDIA_VIDEO_LAPSETIME_2S,
    CVI_MEDIA_VIDEO_LAPSETIME_3S,
    CVI_MEDIA_VIDEO_LAPSETIME_BUIT
} CVI_MEDIA_VIDEO_LAPSETIME_E;

typedef enum _CVI_MEDIA_VIDEO_AUDIO_E
{
    CVI_MEDIA_VIDEO_AUDIO_OFF,
    CVI_MEDIA_VIDEO_AUDIO_ON,
    CVI_MEDIA_VIDEO_AUDIO_BUIT
} CVI_MEDIA_VIDEO_AUDIO_E;

typedef enum _CVI_MEDIA_MOTION_E
{
    CVI_MEDIA_MOTION_OFF,
    CVI_MEDIA_MOTION_ON,
    CVI_MEDIA_MOTION_BUIT
} CVI_MEDIA_MOTION_E;
typedef enum _CVI_MEDIA_VIDEO_OSD_E
{
    CVI_MEDIA_VIDEO_OSD_OFF,
    CVI_MEDIA_VIDEO_OSD_ON,
    CVI_MEDIA_VIDEO_OSD_BUIT
} CVI_MEDIA_VIDEO_OSD_E;

typedef enum _CVI_MEDIA_AUDIO_KEYTONE_E
{
    CVI_MEDIA_AUDIO_KEYTONE_OFF,
    CVI_MEDIA_AUDIO_KEYTONE_ON,
    CVI_MEDIA_AUDIO_KEYTONE_BUIT
} CVI_MEDIA_AUDIO_KEYTONE_E;

typedef enum _CVI_MEDIA_VENC_BIND_MODE_E
{
    CVI_MEDIA_VENC_BIND_NONE,
    CVI_MEDIA_VENC_BIND_VPSS,
    CVI_MEDIA_VENC_BIND_VI,
    CVI_MEDIA_BIND_MODE_BUIT
} CVI_MEDIA_VENC_BIND_MODE_E;

typedef enum _CVI_MENU_SCREENDORMANT_E
{
    CVI_MENU_SCREENDORMANT_OFF,
    CVI_MENU_SCREENDORMANT_1MIN,
    CVI_MENU_SCREENDORMANT_3MIN,
    CVI_MENU_SCREENDORMANT_5MIN,
    CVI_MENU_SCREENDORMANT_BUIT
} CVI_MENU_SCREENDORMANT_E;

typedef enum _CVI_MENU_FATIGUEDRIVE_E
{
    CVI_MENU_FATIGUEDRIVE_OFF,
    CVI_MENU_FATIGUEDRIVE_1HOUR,
    CVI_MENU_FATIGUEDRIVE_2HOUR,
    CVI_MENU_FATIGUEDRIVE_3HOUR,
    CVI_MENU_FATIGUEDRIVE_4HOUR,
    CVI_MENU_FATIGUEDRIVE_BUIT
} CVI_MENU_FATIGUEDRIVE_E;

typedef enum _CVI_MENU_SPEEDSTAMP_E
{
    CVI_MENU_SPEEDSTAMP_OFF,
    CVI_MENU_SPEEDSTAMP_ON,
    CVI_MENU_SPEEDSTAMP_BUIT
} CVI_MENU_SPEEDSTAMP_E;

typedef enum _CVI_MENU_GENSOR_E
{
    CVI_MENU_GENSOR_OFF,
    CVI_MENU_GENSOR_LOW,
    CVI_MENU_GENSOR_MID,
    CVI_MENU_GENSOR_HIGH,
    CVI_MENU_GENSOR_BUIT
} CVI_MENU_GENSOR_E;
typedef enum _CVI_MENU_PHOTORES_E
{
    CVI_MENU_PHOTORES_VGA,
    CVI_MENU_PHOTORES_SVGA,
    CVI_MENU_PHOTORES_2M,
    CVI_MENU_PHOTORES_3M,
    CVI_MENU_PHOTORES_5M,
    CVI_MENU_PHOTORES_8M,
    CVI_MENU_PHOTORES_10M,
    CVI_MENU_PHOTORES_12M,
    CVI_MENU_PHOTORES_BUIT
} CVI_MENU_PHOTORES_E;

typedef enum _CVI_MENU_CAP_QUALITY_E
{
    CVI_MENU_CAP_QUALITY_LOW,
    CVI_MENU_CAP_QUALITY_MID,
    CVI_MENU_CAP_QUALITY_HIGH,
    CVI_MENU_CAP_QUALITY_BUIT
} CVI_MENU_CAP_QUALITY_E;

typedef enum _CVI_MENU_GPSSTAMP_E
{
    CVI_MENU_GPSSTAMP_OFF,
    CVI_MENU_GPSSTAMP_ON,
    CVI_MENU_GPSSTAMP_BUIT
} CVI_MENU_GPSSTAMP_E;

typedef enum _CVI_MENU_GPSINFO_E
{
    CVI_MENU_GPSINFO_OFF,
    CVI_MENU_GPSINFO_ON,
    CVI_MENU_GPSINFO_BUIT
} CVI_MENU_GPSINFO_E;

typedef enum _CVI_MENU_SPEEDUNIT_E
{
    CVI_MENU_SPEEDUNIT_KMH,
    CVI_MENU_SPEEDUNIT_MPH,
    CVI_MENU_SPEEDUNIT_BUIT
} CVI_MENU_SPEEDUNIT_E;

typedef enum _CVI_MENU_REARCAM_MIRROR_E
{
    CVI_MENU_REARCAM_MIRROR_OFF,
    CVI_MENU_REARCAM_MIRROR_ON,
    CVI_MENU_REARCAM_MIRROR_BUIT
} CVI_MENU_REARCAM_MIRROR_E;

typedef enum _CVI_MENU_WIFI_E
{
    CVI_MENU_WIFI_OFF,
    CVI_MENU_WIFI_ON,
    CVI_MENU_WIFI_BUIT
} CVI_MENU_WIFI_E;

typedef enum _CVI_MENU_LANGUAGE_E
{
    CVI_MENU_LANGUAGE_CHN,
    CVI_MENU_LANGUAGE_ENG,
    CVI_MENU_LANGUAGE_BUIT
} CVI_MENU_LANGUAGE_E;

typedef enum _CVI_MENU_TIME_FORMAT_E
{
    CVI_MENU_TIME_FORMAT_12,
    CVI_MENU_TIME_FORMAT_24,
    CVI_MENU_TIME_FORMAT_BUIT
} CVI_MENU_TIME_FORMAT_E;

typedef enum _CVI_MENU_TIME_ZONE_E
{
    CVI_MENU_TIME_ZONE_CHN,
    CVI_MENU_TIME_ZONE_ENG,
    CVI_MENU_TIME_ZONE_UNI,
    CVI_MENU_TIME_ZONE_JAP,
    CVI_MENU_TIME_ZONE_EGP,
    CVI_MENU_TIME_ZONE_BUIT
} CVI_MENU_TIME_ZONE_E;

typedef enum _CVI_MENU_FREQUENCY_E
{
    CVI_MENU_FREQUENCY_OFF,
    CVI_MENU_FREQUENCY_50,
    CVI_MENU_FREQUENCY_60,
    CVI_MENU_FREQUENCY_BUIT
} CVI_MENU_FREQUENCY_E;

typedef enum _CVI_MENU_PARKING_E
{
    CVI_MENU_PARKING_OFF,
    CVI_MENU_PARKING_ON,
    CVI_MENU_PARKING_BUIT
} CVI_MENU_PARKING_E;

typedef enum _CVI_PARAM_MENU_E
{
    CVI_PARAM_MENU_VIDEO_SIZE,
    CVI_PARAM_MENU_VIDEO_LOOP,
    CVI_PARAM_MENU_GSENSOR,
    CVI_PARAM_MENU_CARNUM,
    CVI_PARAM_MENU_VIDEO_CODEC,
    CVI_PARAM_MENU_LAPSE_TIME,
    CVI_PARAM_MENU_AUDIO_STATUS,
    CVI_PARAM_MENU_OSD_STATUS,
    CVI_PARAM_MENU_PWM_BRI_STATUS,
    CVI_PARAM_MENU_VIEW_WIN_STATUS,
    CVI_PARAM_MENU_SCREENDORMANT,
    CVI_PARAM_MENU_KEYTONE,
    CVI_PARAM_MENU_FATIGUE_DRIVE,
    CVI_PARAM_MENU_SPEED_STAMP,
    CVI_PARAM_MENU_GPS_STAMP,
    CVI_PARAM_MENU_SPEED_UNIT,
    CVI_PARAM_MENU_REARCAM_MIRROR,
    CVI_PARAM_MENU_WIFI_STATUS,
    CVI_PARAM_MENU_LANGUAGE,
    CVI_PARAM_MENU_TIME_FORMAT,
    CVI_PARAM_MENU_TIME_ZONE,
    CVI_PARAM_MENU_FREQUENCY,
    CVI_PARAM_MENU_DEFAULT,
    CVI_PARAM_MENU_PARKING,
    CVI_PARAM_MENU_PHOTO_SIZE,
	CVI_PARAM_MENU_PHOTO_QUALITY,
	CVI_PARAM_MENU_MOTION_DET,
    CVI_PARAM_MENU_REC_INX,
    CVI_PARAM_MENU_REC_LOOP,
    CVI_PARAM_MENU_SET_CARNUM_OSD,
    CVI_PARAM_MENU_BUIT
} CVI_PARAM_MENU_E;

/* menu manager */
typedef struct _CVI_PARAM_MENU_ITEM_S {
    char    Desc[CVI_PARAM_MENU_ITEM_DESC_LEN];
    int32_t     Value;
} CVI_PARAM_MENU_ITEM_S;

typedef struct _CVI_PARAM_VALUESET_S {
    uint32_t ItemCnt;
    uint32_t Current;
    CVI_PARAM_MENU_ITEM_S Items[CVI_PARAM_MENU_ITEM_MAX];
} CVI_PARAM_VALUESET_S;


typedef struct _CVI_PARAM_USER_MENU_S {
    CVI_BOOL                    bBootFirst;
    CVI_CHAR                    cUserReserved[3];
    CVI_U32                     u32UserCarNum[8];
    CVI_CHAR                    cUserCarName[16];
    CVI_U32                     u32UserReserved[3];
} CVI_PARAM_USER_MENU_S;

typedef struct _CVI_PARAM_MENU_S {
    CVI_PARAM_VALUESET_S VideoSize;
    CVI_PARAM_VALUESET_S VideoLoop;
    CVI_PARAM_VALUESET_S VideoCodec;
    CVI_PARAM_VALUESET_S LapseTime;
    CVI_PARAM_VALUESET_S AudioEnable;
    CVI_PARAM_VALUESET_S Osd;
    CVI_PARAM_VALUESET_S PwmBri;
    CVI_PARAM_VALUESET_S ViewWin;
    CVI_PARAM_VALUESET_S LcdContrl;
    CVI_PARAM_VALUESET_S ScreenDormant;
    CVI_PARAM_VALUESET_S KeyTone;
    CVI_PARAM_VALUESET_S FatigueDirve;
    CVI_PARAM_VALUESET_S SpeedStamp;
    CVI_PARAM_VALUESET_S GPSStamp;
    CVI_PARAM_VALUESET_S SpeedUnit;
    CVI_PARAM_VALUESET_S CamMirror;
    CVI_PARAM_VALUESET_S Language;
    CVI_PARAM_VALUESET_S TimeFormat;
    CVI_PARAM_VALUESET_S TimeZone;
    CVI_PARAM_VALUESET_S Frequence;
    CVI_PARAM_VALUESET_S Parking;
    CVI_PARAM_VALUESET_S PhotoSize;
    CVI_PARAM_VALUESET_S PhotoQuality;
    CVI_PARAM_VALUESET_S MotionDet;
    CVI_PARAM_VALUESET_S CarNumStamp;
    CVI_PARAM_VALUESET_S RecLoop;
    CVI_PARAM_USER_MENU_S       UserData;//             bBootFirst;
} CVI_PARAM_MENU_S;

/* global parameters manager */
typedef struct _CVI_PARAM_HEAD_S {
    uint32_t ParamLen;
} CVI_PARAM_HEAD_S;

typedef struct cviMEDIA_FILEMNG_ATTR_S {
    CVI_FILEMNG_COMM_CFG_S FileMngComm;
    CVI_FILEMNG_DTCF_CFG_S FileMngDtcf;
} CVI_PARAM_FILEMNG_S;

typedef struct _CVI_PARAM_WIFI_S {
    bool                  Enable;
    char                  WifiDefaultSsid[CVI_HAL_WIFI_SSID_LEN];
    CVI_HAL_WIFI_CFG_S    WifiCfg;
} CVI_PARAM_WIFI_S;

typedef struct _CVI_PARAM_PWM_S {
    bool                  Enable;
    CVI_HAL_SCREEN_PWM_S    PWMCfg;
} CVI_PARAM_PWM_S;

typedef struct _CVI_PARAM_DEVMNG_S {
    STG_DEVINFO_S         Stg;
    CVI_PARAM_WIFI_S      Wifi;
    CVI_KEYMNG_CFG_S      stkeyMngCfg;
    CVI_GSENSORMNG_CFG_S  Gsensor;
    CVI_GAUGEMNG_CFG_S    GaugeCfg;
    CVI_PARAM_PWM_S       PWM;
} CVI_PARAM_DEVMNG_S;

typedef struct _CVI_PARAM_DISP_ATTR_S {
    uint32_t    Width;
    uint32_t    Height;
    uint32_t    Rotate;
    int32_t     Fps;
} CVI_PARAM_DISP_ATTR_S;

typedef struct _CVI_PARAM_WND_ATTR_S {
    uint32_t        WndCnt;
#ifdef SERVICES_LIVEVIEW_ON
    CVI_LIVEVIEW_SERVICE_WNDATTR_S Wnds[CVI_PARAM_WND_NUM_MAX];
#endif
} CVI_PARAM_WND_ATTR_S;

typedef struct _CVI_PARAM_RECORD_CHN_ATTR_S {
    bool        Enable;
    bool        Subvideoen;
    bool        AudioStatus;
    uint32_t    SubBindVencId;
    uint32_t    BindVencId;
    uint32_t    FileType;
    uint64_t    SplitTime;
    uint32_t    PreTime;
    uint32_t    PostTime;
    float       TimelapseFps;
    uint32_t    TimelapseGop;
    uint32_t    MemoryBufferSec;
    uint32_t    PreallocUnit;
    float       NormalExtendVideoBufferSec;
    float       EventExtendVideoBufferSec;
    float       ExtendOtherBufferSec;
    float       ShortFileMs;
    char        devmodel[32];
} CVI_PARAM_RECORD_CHN_ATTR_S;

typedef struct _CVI_PARAM_RECORD_ATTR_S {
    uint32_t                    ChnCnt;
    CVI_PARAM_RECORD_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} CVI_PARAM_RECORD_ATTR_S;

typedef struct _CVI_PARAM_RTSP_CHN_ATTR_S {
    bool        Enable;
    uint32_t    BindVencId;
    int32_t         MaxConn;
    int32_t         Timeout;
    int32_t         Port;
    char        Name[32];
} CVI_PARAM_RTSP_CHN_ATTR_S;

typedef struct _CVI_PARAM_RTSP_ATTR_S {
    CVI_PARAM_RTSP_CHN_ATTR_S   ChnAttrs[MAX_RTSP_CNT];
} CVI_PARAM_RTSP_ATTR_S;

#ifdef ENABLE_ISP_IRCUT
typedef struct _CVI_PARAM_ISPIR_CHN_ATTR_S {
    bool        bEnable;
    int32_t     s32IRControlMode;
    int32_t     s32CamId;
    int32_t     s32IrCutA;
    int32_t     s32IrCutB;
    int32_t     s32LedIr;
    int16_t     s16Normal2IrIsoThr;
    int16_t     s16Ir2NormalIsoThr;
    char        DayBinPath[128];
    char        NightBinPath[128];
} CVI_PARAM_ISPIR_CHN_ATTR_S;

typedef struct _CVI_PARAM_ISPIR_ATTR_S {
    CVI_PARAM_ISPIR_CHN_ATTR_S   stIspIrChnAttrs[MAX_CAMERA_INSTANCES];
} CVI_PARAM_ISPIR_ATTR_S;
#endif

typedef struct _CVI_PARAM_PIV_CHN_ATTR_S {
    uint32_t BindVencId;
} CVI_PARAM_PIV_CHN_ATTR_S;

typedef struct _CVI_PARAM_PIV_ATTR_S {
    CVI_PARAM_PIV_CHN_ATTR_S    ChnAttrs[MAX_CAMERA_INSTANCES];
} CVI_PARAM_PIV_ATTR_S;

typedef struct _CVI_PARAM_THUMBNAIL_CHN_ATTR_S {
    uint32_t            BindVencId;
} CVI_PARAM_THUMBNAIL_CHN_ATTR_S;

typedef struct _CVI_PARAM_THUMBNAIL_ATTR_S {
    CVI_PARAM_THUMBNAIL_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} CVI_PARAM_THUMBNAIL_ATTR_S;

typedef struct _CVI_PARAM_PHOTO_CHN_ATTR_S{
    bool        Enable;
    uint32_t    BindVencId;
} CVI_PARAM_PHOTO_CHN_ATTR_S;

typedef struct _CVI_PARAM_PHOTO_ATTR_S {
    uint32_t    photoid;
    uint32_t    VprocDev_id;
    CVI_PARAM_PHOTO_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} CVI_PARAM_PHOTO_ATTR_S;

typedef struct _CVI_PARAM_MD_CHN_ATTR_S{
    bool        Enable;
    uint32_t    BindVprocId;
    uint32_t    BindVprocChnId;
} CVI_PARAM_MD_CHN_ATTR_S;

typedef struct _CVI_PARAM_MD_ATTR_S {
    uint32_t    motionSensitivity;
    CVI_PARAM_MD_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} CVI_PARAM_MD_ATTR_S;

typedef struct _CVI_PARAM_VPSS_ATTR_S {
    VI_VPSS_MODE_S stVIVPSSMode;
    VPSS_MODE_S stVPSSMode;
} CVI_PARAM_VPSS_ATTR_S;

#ifdef SERVICES_ADAS_ON
typedef struct _CVI_PARAM_ADAS_CHN_ATTR_S{
    bool        Enable;
    uint32_t    BindVprocId;
    uint32_t    BindVprocChnId;
} CVI_PARAM_ADAS_CHN_ATTR_S;

typedef struct _CVI_PARAM_ADAS_ATTR_S {
    int32_t adas_cnt;
    CVI_PARAM_ADAS_MODEL_ATTR_S stADASModelAttr;
    CVI_PARAM_ADAS_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} CVI_PARAM_ADAS_ATTR_S;
#endif

#ifdef SERVICES_QRCODE_ON
typedef struct _CVI_PARAM_QRCODE_CHN_ATTR_S {
    bool        Enable;
    uint32_t    BindVprocId;
    uint32_t    BindVprocChnId;
} CVI_PARAM_QRCODE_CHN_ATTR_S;

typedef struct _CVI_PARAM_QRCODE_ATTR_S {
    int32_t qrcode_cnt;
    CVI_PARAM_QRCODE_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} CVI_PARAM_QRCODE_ATTR_S;
#endif

typedef struct _CVI_PARAM_MEDIA_COMM_S {
    uint32_t                    PowerOnMode;    // 0: record 1:photo 2:playback 3:usb
    CVI_PARAM_VPSS_ATTR_S       Vpss;
    CVI_PARAM_DISP_ATTR_S       Vo;             // vo config
    CVI_PARAM_WND_ATTR_S        Window;         // window config
    CVI_MAPI_ACAP_ATTR_S        Ai;             // ai config
    CVI_MAPI_AENC_ATTR_S        Aenc;           // aecn config
    CVI_MAPI_AO_ATTR_S          Ao;             // ao config
    CVI_PARAM_RECORD_ATTR_S     Record;         // record
    CVI_PARAM_RTSP_ATTR_S       Rtsp;           // rtsp
    CVI_PARAM_PIV_ATTR_S        Piv;            // PIV
    CVI_PARAM_THUMBNAIL_ATTR_S  Thumbnail;      // Thumbnail
    CVI_PARAM_PHOTO_ATTR_S      Photo;          // photo resolution ratio
    CVI_PARAM_MD_ATTR_S         Md;             // video motion detect
#ifdef SERVICES_SPEECH_ON
    CVI_SPEECHMNG_PARAM_S       Speech;         // speech config
#endif
#ifdef SERVICES_ADAS_ON
    CVI_PARAM_ADAS_ATTR_S       ADAS;           // ADAS
#endif
#ifdef SERVICES_QRCODE_ON
    CVI_PARAM_QRCODE_ATTR_S     QRCODE;
#endif
#ifdef ENABLE_ISP_IRCUT
    CVI_PARAM_ISPIR_ATTR_S     IspIr;
#endif
} CVI_PARAM_MEDIA_COMM_S;

typedef struct _CVI_PARAM_MEDIA_SNS_ATTR_S {
    bool                           SnsEnable;
    CVI_MAPI_VCAP_SENSOR_ATTR_T    SnsChnAttr;
} CVI_PARAM_MEDIA_SNS_ATTR_S;

typedef struct _CVI_PARAM_MEDIA_VACP_ATTR_S {
    bool                        VcapEnable;
    uint32_t                    VcapId;
    CVI_MAPI_VCAP_CHN_ATTR_T    VcapChnAttr;
} CVI_PARAM_MEDIA_VACP_ATTR_S;

typedef struct _CVI_MEDIA_VPROC_CHN_ATTR_S {
    uint32_t        VprocChnid;
    bool            VprocChnEnable;
    uint32_t        VprocChnVbCnt;
    uint32_t        VprocChnLowDelayCnt;
    VPSS_CHN_ATTR_S VpssChnAttr;
    bool            bFb;
} CVI_MEDIA_VPROC_CHN_ATTR_S;

typedef struct _CVI_MEDIA_EXT_VPROC_CHN_ATTR_S {
    bool                    ChnEnable;
    CVI_MAPI_EXTCHN_ATTR_T  ChnAttr;
} CVI_MEDIA_EXT_VPROC_CHN_ATTR_S;

typedef struct _CVI_PARAM_MEDIA_VPROC_ATTR_S {
    uint32_t                        Vprocid;
    VPSS_GRP_ATTR_S                 VpssGrpAttr;
    CVI_MEDIA_VPROC_CHN_ATTR_S      VprocChnAttr[CVI_MAPI_VPROC_MAX_CHN_NUM];
    CVI_MEDIA_EXT_VPROC_CHN_ATTR_S  ExtChnAttr[CVI_MAPI_VPROC_MAX_CHN_NUM];
} CVI_PARAM_MEDIA_VPROC_ATTR_S;

typedef struct _CVI_MEDIA_VENC_CHN_ATTR_S {
    bool                        VencChnEnable;
    uint32_t                    VencId;
    uint32_t                    BindVprocId;
    uint32_t                    BindVprocChnId;
    float                       framerate;
    CVI_MEDIA_VENC_BIND_MODE_E  bindMode;
    CVI_MAPI_VENC_CHN_PARAM_T   MapiVencAttr;
} CVI_MEDIA_VENC_CHN_ATTR_S;

typedef struct _CVI_PARAM_MEDIA_VENC_ATTR_S {
    CVI_MEDIA_VENC_CHN_ATTR_S   VencChnAttr[MAX_VENC_CNT];
} CVI_PARAM_MEDIA_VENC_ATTR_S;

typedef struct _CVI_PARAM_MEDIA_VB_ATTR_S {
    uint32_t                     Poolcnt;
    CVI_MAPI_MEDIA_SYS_VB_POOL_T Vbpool[CVI_MAPI_VB_POOL_MAX_NUM];
} CVI_PARAM_MEDIA_VB_ATTR_S;

typedef struct _CVI_PARAM_MEDIA_OSD_ATTR_S {
    int32_t                 OsdCnt;
    CVI_MAPI_OSD_ATTR_S OsdAttrs[CVI_MEDIA_MAX_OSD_IDX];
} CVI_PARAM_MEDIA_OSD_ATTR_S;

typedef struct _CVI_PARAM_MEDIA_SPEC_S {
    uint32_t                             MediaMode;
    CVI_PARAM_MEDIA_SNS_ATTR_S           SnsAttr;
    CVI_PARAM_MEDIA_VACP_ATTR_S          VcapAttr;
    CVI_PARAM_MEDIA_VPROC_ATTR_S         VprocAttr;
    CVI_PARAM_MEDIA_VENC_ATTR_S          VencAttr;
    CVI_PARAM_MEDIA_VB_ATTR_S            Vb;             // vb config
    CVI_PARAM_MEDIA_OSD_ATTR_S           Osd;            // osd config
} CVI_PARAM_MEDIA_SPEC_S;

typedef struct _CVI_PARAM_CAM_MEDIA {
    uint32_t                    CamID;
    uint32_t                    CurMediaMode;
} CVI_PARAM_CAM_MEDIA;

/* work mode manager */
typedef struct _CVI_PARAM_CAM_CFG {
    bool                        CamEnable;
    bool                        CamIspEnable;
    CVI_PARAM_CAM_MEDIA         CamMediaInfo;
    uint32_t                    MediaModeCnt;
    CVI_PARAM_MEDIA_SPEC_S      MediaSpec[CVI_PARAM_MEDIA_CNT];
} CVI_PARAM_CAM_CFG;

typedef struct _CVI_PARAM_UVC_PARAM_S {
    uint32_t                VcapId;
    uint32_t                VprocId;
    uint32_t                VprocChnId;
    uint32_t                AcapId;
    CVI_UVC_CFG_ATTR_S      UvcCfg;
} CVI_PARAM_UVC_PARAM_S;

typedef struct _CVI_PARAM_MODE_S {
    uint32_t                CamNum;
    CVI_PARAM_VPSS_ATTR_S   Vpss;
    CVI_PARAM_CAM_MEDIA     CamMediaInfo[CVI_PARAM_MEDIA_CNT];
} CVI_PARAM_MODE_S;

typedef struct _CVI_PARAM_USB_MODE_S {
    uint32_t                UsbWorkMode;
    CVI_PARAM_UVC_PARAM_S   UvcParam;
    CVI_USB_STORAGE_CFG_S   StorageCfg;
} CVI_PARAM_USB_MODE_S;

typedef struct _CVI_PARAM_WORK_MODE_S {
    CVI_PARAM_MODE_S        RecordMode;
    CVI_PARAM_MODE_S        PhotoMode;
    CVI_PARAM_USB_MODE_S    UsbMode;
} CVI_PARAM_WORK_MODE_S;

typedef struct _CVI_PARAM_CFG_S {
    uint32_t MagicStart;
    CVI_PARAM_HEAD_S            Head;
    CVI_PARAM_FILEMNG_S         FileMng;
    CVI_PARAM_DEVMNG_S          DevMng;
    CVI_PARAM_CAM_CFG           CamCfg[MAX_CAMERA_INSTANCES];
    CVI_PARAM_WORK_MODE_S       WorkModeCfg;
    CVI_PARAM_MEDIA_COMM_S      MediaComm;
    CVI_PARAM_MENU_S            Menu;
    uint32_t MagicEnd;
    uint32_t crc32;
} CVI_PARAM_CFG_S;

typedef struct _CVI_PARAM_CONTEXT_S {
    bool bInit;
    bool bChanged;
    pthread_mutex_t mutexLock;
    CVI_PARAM_CFG_S *pstCfg;
} CVI_PARAM_CONTEXT_S;

int32_t CVI_PARAM_Init(void);
int32_t CVI_PARAM_Deinit(void);
CVI_PARAM_CONTEXT_S *CVI_PARAM_GetCtx(void);
int32_t CVI_PARAM_GetParam(CVI_PARAM_CFG_S *param);
int32_t CVI_PARAM_Save2Bin(void);
int32_t CVI_PARAM_LoadFromBin(const char *path);
int32_t CVI_PARAM_LoadFromFlash(char *partition, uint64_t addr, uint64_t len);
int32_t CVI_PARAM_SaveParam(void);
void CVI_PARAM_SetSaveFlg(void);
int32_t CVI_PARAM_GetCamStatus(uint32_t CamId, bool *Param);
int32_t CVI_PARAM_GetMediaMode(uint32_t CamId, CVI_PARAM_MEDIA_SPEC_S *Param);
int32_t CVI_PARAM_GetVbParam(CVI_MAPI_MEDIA_SYS_ATTR_T *Param);
int32_t CVI_PARAM_GetVpssModeParam(CVI_PARAM_VPSS_ATTR_S *Param);
int32_t CVI_PARAM_GetWorkModeParam(CVI_PARAM_WORK_MODE_S *Param);
int32_t CVI_PARAM_GetVoParam(CVI_PARAM_DISP_ATTR_S *Param);
int32_t CVI_PARAM_GetWndParam(CVI_PARAM_WND_ATTR_S *Param);
int32_t CVI_PARAM_GetAiParam(CVI_MAPI_ACAP_ATTR_S *Param);
int32_t CVI_PARAM_GetAencParam(CVI_MAPI_AENC_ATTR_S *Param);
int32_t CVI_PARAM_GetAoParam(CVI_MAPI_AO_ATTR_S *Param);
int32_t CVI_PARAM_GetOsdParam(CVI_PARAM_MEDIA_OSD_ATTR_S *Param);
int32_t CVI_PARAM_GetStgInfoParam(STG_DEVINFO_S *Param);
int32_t CVI_PARAM_GetFileMngParam(CVI_PARAM_FILEMNG_S *Param);
int32_t CVI_PARAM_GetUsbParam(CVI_PARAM_USB_MODE_S *Param);
int32_t CVI_PARAM_GetMenuParam(CVI_PARAM_MENU_S *Param);
int32_t CVI_PARAM_GetDevParam(CVI_PARAM_DEVMNG_S *Param);
int32_t CVI_PARAM_GetMenuScreenDormantParam(int32_t *Value);
int32_t CVI_PARAM_SetParam(CVI_PARAM_CFG_S *param);
int32_t CVI_PARAM_SetCamStatus(uint32_t CamId, bool Param);
int32_t CVI_PARAM_SetCamIspInfoStatus(uint32_t CamId, bool Param);
int32_t CVI_PARAM_SetWndParam(CVI_PARAM_WND_ATTR_S *Param);
int32_t CVI_PARAM_SetOsdParam(CVI_PARAM_MEDIA_OSD_ATTR_S *Param);
int32_t CVI_PARAM_SetMenuParam(uint32_t CamId, CVI_PARAM_MENU_E MenuItem, int32_t Value);
int32_t CVI_PARAM_GetKeyMngCfg(CVI_KEYMNG_CFG_S *Param);
int32_t CVI_PARAM_GetGaugeMngCfg(CVI_GAUGEMNG_CFG_S *Param);

int32_t CVI_PARAM_GetWifiParam(CVI_PARAM_WIFI_S *Param);
int32_t CVI_PARAM_SetWifiParam(CVI_PARAM_WIFI_S *Param);

int32_t CVI_PARAM_GetPWMParam(CVI_PARAM_PWM_S *Param);
int32_t CVI_PARAM_SetPWMParam(CVI_PARAM_PWM_S *Param);

int32_t CVI_PARAM_GetGsensorParam(CVI_GSENSORMNG_CFG_S *Param);
int32_t CVI_PARAM_SetGsensorParam(CVI_GSENSORMNG_CFG_S *Param);
int32_t CVI_PARAM_GetKeyTone(int32_t *Value);
int32_t CVI_PARAM_GetFatigueDrive(int32_t *Value);
int32_t CVI_PARAM_GetMenuSpeedStampParam(int32_t *Value);
int32_t CVI_PARAM_GetMenuGPSStampParam(int32_t *Value);
int32_t CVI_PARAM_LoadDefaultParamFromFlash(CVI_PARAM_CFG_S* param);
int32_t CVI_PARAM_GetVideoSizeEnum(int32_t Value, CVI_MEDIA_VIDEO_SIZE_E *VideoSize);
int32_t CVI_PARAM_SetVENCParam();
void CVI_PARAM_SetMediaModeParam(int32_t cammodevule);
int32_t CVI_PARAM_GetMediaPhotoSize(CVI_PARAM_MEDIA_COMM_S *Param);
int32_t CVI_PARAM_GetMediaComm(CVI_PARAM_MEDIA_COMM_S *Param);
int32_t CVI_PARAM_SetMediaComm(CVI_PARAM_MEDIA_COMM_S *Param);
int32_t CVI_PARAM_SetBootFirstFlag(CVI_BOOL Value);
void CVI_PARAM_GetOsdCarNameParam(char *string_carnum_stamp, CVI_MENU_LANGUAGE_E* lang);
void CVI_PARAM_SetOsdCarNameParam(char *string_carnum_stamp);
void CVI_PARAM_GetMdConfigParam(CVI_PARAM_MD_ATTR_S *Md);
int32_t CVI_PARAM_GetRecLoop(int32_t *Value);
int32_t CVI_PARAM_Get_View_Win(void);
int32_t CVI_PARAM_GetAhdDefaultMode(uint32_t CamId, int32_t *mode);
#ifdef SERVICES_SPEECH_ON
int32_t CVI_PARAM_GetSpeechParam(CVI_SPEECHMNG_PARAM_S *Param);
int32_t CVI_PARAM_SetSpeechParam(CVI_SPEECHMNG_PARAM_S *Param);
#endif
#ifdef SERVICES_ADAS_ON
void CVI_PARAM_GetADASConfigParam(CVI_PARAM_ADAS_ATTR_S *ADAS);
#endif
#ifdef ENABLE_ISP_IRCUT
void CVI_PARAM_GetISPIrConfigParam(CVI_PARAM_ISPIR_ATTR_S *ISPIR);
#endif
#ifdef SERVICES_QRCODE_ON
void CVI_PARAM_GetQRCodeConfigParam(CVI_PARAM_QRCODE_ATTR_S *QRCODE);
#endif
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_PARAM_H__ */
