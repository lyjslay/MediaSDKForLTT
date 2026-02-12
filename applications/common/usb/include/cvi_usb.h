/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_usb.h
 * Description:
 *   usb module interface decalration
 */

#ifndef __CVI_USB_H__
#define __CVI_USB_H__

#include <sys/types.h>
#include "cvi_appcomm.h"
#include "cvi_mapi_define.h"
#include "cvi_usb_storage.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define CVI_USB_EINVAL               CVI_APPCOMM_ERR_ID(CVI_APP_MOD_USBMNG, CVI_EINVAL)       /**<Invalid argument */
#define CVI_USB_ENOTINIT             CVI_APPCOMM_ERR_ID(CVI_APP_MOD_USBMNG, CVI_ENOINIT)      /**<Not inited */
#define CVI_USB_EINITIALIZED         CVI_APPCOMM_ERR_ID(CVI_APP_MOD_USBMNG, CVI_EINITIALIZED) /**<reinitialized */


/** usb event define */
#define CVI_EVENT_USB_OUT            CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_USBMNG, 0) /**<usb out */
#define CVI_EVENT_USB_INSERT         CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_USBMNG, 1) /**<usb insert */
#define CVI_EVENT_USB_UVC_READY      CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_USBMNG, 2) /**<uvc mode: pc ready */
#define CVI_EVENT_USB_STORAGE_READY  CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_USBMNG, 3) /**<storage mode: pc ready */
#define CVI_EVENT_USB_HOSTUVC_READY  CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_USBMNG, 4) /**<host uvc mode: camera ready */
#define CVI_EVENT_USB_HOSTUVC_PC     CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_USBMNG, 5) /**<usb source pc */
#define CVI_EVENT_USB_HOSTUVC_HEAD   CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_USBMNG, 6) /**<usb source head */
#define CVI_EVENT_USB_PC_INSERT      CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_USBMNG, 7) /**<pc usb insert */

#define CVI_USB_HOSTUVC_CAP_MAXCNT            24U
#define CVI_USB_HOSTUVC_LIMITLESS_FRAME_COUNT (-1)

/** ---------------- UVC Define Begin ---------------- */

/** uvc stream format enum */
typedef enum cviUVC_STREAM_FORMAT_E
{
    CVI_UVC_STREAM_FORMAT_YUV420 = 0,
    CVI_UVC_STREAM_FORMAT_MJPEG,
    CVI_UVC_STREAM_FORMAT_H264,
    CVI_UVC_STREAM_FORMAT_BUTT
} CVI_UVC_STREAM_FORMAT_E;

/** uvc video mode enum, including resolution and framerate
   * should be a subset of driver capabilities */
typedef enum cviUVC_VIDEOMODE_E
{
    CVI_UVC_VIDEOMODE_VGA30 = 0, /**<640x360   30fps */
    CVI_UVC_VIDEOMODE_720P30,    /**<1280x720  30fps */
    CVI_UVC_VIDEOMODE_1080P30,   /**<1920x1080 30fps */
    CVI_UVC_VIDEOMODE_4K30,      /**<3840x2160 30fps */
    CVI_UVC_VIDEOMODE_BUTT
} CVI_UVC_VIDEOMODE_E;

typedef struct cviUVC_VIDEOATTR_S
{
    uint32_t u32BitRate;
    CVI_UVC_VIDEOMODE_E enMode;
} CVI_UVC_VIDEOATTR_S;

/** uvc format capabilities, including videomode cnt and list */
typedef struct cviUVC_FORMAT_CAP_S
{
    uint32_t u32Cnt;
    CVI_UVC_VIDEOMODE_E enDefMode;
    CVI_UVC_VIDEOATTR_S astModes[CVI_UVC_VIDEOMODE_BUTT];
} CVI_UVC_FORMAT_CAP_S;

/** uvc device capabilities, including all format */
typedef struct cviUVC_DEVICE_CAP_S
{
    CVI_UVC_FORMAT_CAP_S astFmtCaps[CVI_UVC_STREAM_FORMAT_BUTT];
} CVI_UVC_DEVICE_CAP_S;

/** uvc data source handle */
typedef struct cviUVC_DATA_SOURCE_S
{
    CVI_MAPI_HANDLE_T VcapHdl; /**<vcap handle */
    CVI_MAPI_HANDLE_T VprocHdl; /**<vproc handle */
    CVI_MAPI_HANDLE_T VencHdl;  /**<venc handle */
    CVI_MAPI_HANDLE_T AcapHdl;  /**<audio handle */
    uint32_t          VprocChnId; /**<vproc chn id */
} CVI_UVC_DATA_SOURCE_S;

/** uvc data source handle */
typedef struct cviUVC_BUFFER_CFG_S
{
    uint32_t u32BufCnt;
    uint32_t u32BufSize;
} CVI_UVC_BUFFER_CFG_S;

/** uvc configure */
typedef struct cviUVC_CFG_ATTR_S
{
    char    szDevPath[CVI_APPCOMM_MAX_PATH_LEN]; /**<uvc device path */
    CVI_UVC_DEVICE_CAP_S  stDevCap;
    CVI_UVC_BUFFER_CFG_S stBufferCfg;
} CVI_UVC_CFG_ATTR_S;

typedef struct cviUVC_CFG_S
{
    CVI_UVC_CFG_ATTR_S    attr;
    CVI_UVC_DATA_SOURCE_S stDataSource;
} CVI_UVC_CFG_S;

/** ---------------- UVC Define End ---------------- */

/** usb event information */
typedef struct cviUSB_EVENT_INFO_S
{
    int32_t     s32EventId;
    char        szPayload[CVI_APPCOMM_MAX_PATH_LEN];
} CVI_USB_EVENT_INFO_S;

/** usb state callback */
typedef int32_t (*CVI_USB_EVENTPROC_CALLBACK_FN_PTR)(const CVI_USB_EVENT_INFO_S *pstEventInfo);

/** usb state enum, including uvc/storage state */
typedef enum cviUSB_STATE_E
{
    CVI_USB_STATE_OUT = 0,          /**<usb out */
    CVI_USB_STATE_INSERT,           /**<usb insert */
    CVI_USB_STATE_PC_INSERT,        /**<PC usb insert */
    CVI_USB_STATE_UVC_READY,        /**<uvc driver load ready */
    CVI_USB_STATE_UVC_PC_READY,     /**<uvc pc interaction ready */
    CVI_USB_STATE_UVC_MEDIA_READY,  /**<uvc media ready */
    CVI_USB_STATE_STORAGE_READY,    /**<storage driver load ready */
    CVI_USB_STATE_STORAGE_PC_READY, /**<storage pc interaction ready */
    CVI_USB_STATE_STORAGE_SD_READY, /**<storage sd device ready */
    CVI_USB_STATE_HOSTUVC_READY,        /* host uvc driver load ready */
    CVI_USB_STATE_HOSTUVC_CAMERA_READY, /* host uvc camera ready */
    CVI_USB_STATE_HOSTUVC_MEDIA_READY,  /* host uvc media ready */
    CVI_USB_STATE_BUTT
} CVI_USB_STATE_E;

/** usb mode enum */
typedef enum cviUSB_MODE_E
{
    CVI_USB_MODE_CHARGE = 0,
    CVI_USB_MODE_UVC,
    CVI_USB_MODE_STORAGE,
    CVI_USB_MODE_HOSTUVC, /* host uvc not suport */
    CVI_USB_MODE_BUTT
} CVI_USB_MODE_E;

typedef int32_t (*CVI_USB_GET_STORAGE_STATE_FN_PTR)(void* pvPrivData);

/** usb configure */
typedef struct cviUSB_CFG_S
{
    CVI_UVC_CFG_S stUvcCfg;
    CVI_USB_STORAGE_CFG_S stStorageCfg;
    CVI_USB_EVENTPROC_CALLBACK_FN_PTR pfnEventProc;
    CVI_USB_GET_STORAGE_STATE_FN_PTR pfnGetStorageState;
} CVI_USB_CFG_S;

/** usb connect power source */
typedef enum tagUSB_POWER_SOURCE_E
{
    USB_POWER_SOURCE_NONE = 0,
    USB_POWER_SOURCE_PC,
    USB_POWER_SOURCE_POWER,
    USB_POWER_SOURCE_BUTT
} USB_POWER_SOURCE_E;

int32_t CVI_USB_Init(const CVI_USB_CFG_S* pstCfg);
int32_t CVI_USB_Deinit(void);
int32_t CVI_USB_SetMode(CVI_USB_MODE_E enMode);
int32_t CVI_USB_SetUvcCfg(const CVI_UVC_CFG_S* pstCfg);
int32_t CVI_USB_SetStorageCfg(const CVI_USB_STORAGE_CFG_S* pstCfg);
int32_t CVI_USB_GetUvcCfg(CVI_USB_CFG_S* pstCfg);
int32_t CVI_USB_GetStorageCfg(CVI_USB_STORAGE_CFG_S* pstCfg);
void CVI_USB_CheckPower_Soure(USB_POWER_SOURCE_E* PowerSoureState);

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#endif /* __CVI_USB_H__ */