#ifndef __CVI_MEDIA_INIT_H__
#define __CVI_MEDIA_INIT_H__

#include "cvi_param.h"
#include "cvi_modetest.h"
#include "cvi_appcomm.h"
#include "cvi_record_service.h"
#ifdef SERVICES_PHOTO_ON
#include "cvi_photo_service.h"
#endif
#ifdef SERVICES_RTSP_ON
#include "cvi_rtsp_service.h"
#endif
#ifdef SERVICES_PLAYER_ON
#include "cvi_player_service.h"
#endif
#ifdef SERVICES_LIVEVIEW_ON
#include "cvi_liveview.h"
#endif
#ifdef SERVICES_ADAS_ON
#include "cvi_adas_service.h"
#endif

#ifdef SERVICES_QRCODE_ON
#include "cvi_qrcode_ser.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

//media param
typedef struct cviMEDIA_SYSHANDLE_S {
    CVI_MAPI_VCAP_SENSOR_HANDLE_T   sns[MAX_DEV_INSTANCES];
    CVI_MAPI_VPROC_HANDLE_T         vproc[MAX_VPROC_CNT];
    CVI_MAPI_VENC_HANDLE_T          venchdl[MAX_CAMERA_INSTANCES][MAX_VENC_CNT];
    CVI_MAPI_ACAP_HANDLE_T          aihdl;
    CVI_MAPI_AENC_HANDLE_T          aenchdl;
    CVI_MAPI_DISP_HANDLE_T          dispHdl;
    CVI_MAPI_AO_HANDLE_T            aohdl;
} CVI_MEDIA_SYSHANDLE_S;

typedef struct cviCAMERA_SERVICE_S {
    CVI_RECORD_SERVICE_HANDLE_T     RecordHdl[MAX_CAMERA_INSTANCES];
    CVI_RECORD_SERVICE_PARAM_S      RecordParams[MAX_CAMERA_INSTANCES];
#ifdef SERVICES_PHOTO_ON
    CVI_PHOTO_SERVICE_HANDLE_T      PhotoHdl[MAX_CAMERA_INSTANCES];
    CVI_PHOTO_SERVICE_PARAM_S       PhotoParams[MAX_CAMERA_INSTANCES];
#endif
#ifdef SERVICES_RTSP_ON
    CVI_RTSP_SERVICE_HANDLE_T       RtspHdl[MAX_RTSP_CNT];
    CVI_RTSP_SERVICE_PARAM_S        RtspParams[MAX_RTSP_CNT];
#endif
#ifdef SERVICES_ADAS_ON
    CVI_ADAS_SERVICE_HANDLE_T       ADASHdl[MAX_CAMERA_INSTANCES];
    CVI_ADAS_SERVICE_PARAM_S        ADASParams[MAX_CAMERA_INSTANCES];
#endif
#ifdef SERVICES_PLAYER_ON
    CVI_PLAYER_SERVICE_HANDLE_T     PsHdl;
    CVI_PLAYER_SERVICE_PARAM_S      PsParam;
#endif
#ifdef SERVICES_LIVEVIEW_ON
    CVI_LIVEVIEW_SERVICE_HANDLE_T   LvHdl;
    CVI_LIVEVIEW_SERVICE_PARAM_S    LvParams;
#endif
#ifdef SERVICES_QRCODE_ON
    CVI_QRCODE_SERVICE_HANDLE_T     QRCodeHdl[MAX_CAMERA_INSTANCES];
    CVI_QRCODE_SERVICE_PARAM_S      QRCodeParams[MAX_CAMERA_INSTANCES];
#endif
    CVI_MT_HANDLE_T                 MtHdl;
    CVI_MT_SERVICE_PARAM_T          MtParam;
} CVI_MEDIA_CAMERA_SERVICE_S;

typedef struct cviMEDIA_PARAM_INIT_S {
    CVI_MEDIA_SYSHANDLE_S           SysHandle;
    CVI_MEDIA_CAMERA_SERVICE_S            SysServices;
} CVI_MEDIA_PARAM_INIT_S;

#define CVI_MEDIA_EINITIALIZED      CVI_APPCOMM_ERR_ID(CVI_APP_MOD_MEDIA,CVI_EINITIALIZED)/**<Initialized already */
#define CVI_MEDIA_ENOTINIT          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_MEDIA,CVI_ENOINIT)/**<Not inited */
#define CVI_MEDIA_EINVAL            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_MEDIA,CVI_EINTER)/**<Parameter invalid */
#define CVI_MEDIA_EINTER            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_MEDIA,CVI_EINTER)/**<Internal error */
#define CVI_MEDIA_ENULLPTR          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_MEDIA,CVI_ERRNO_CUSTOM_BOTTOM + 1)/**<Null pointer */
#define CVI_MEDIA_EINPROGRESS       CVI_APPCOMM_ERR_ID(CVI_APP_MOD_MEDIA,CVI_ERRNO_CUSTOM_BOTTOM + 2)/**<Operation now in progress */

/** NULL pointer check */
#define MEDIA_CHECK_POINTER(ptr,errcode,string)\
do{\
    if(NULL == ptr)\
     {\
        CVI_LOGE("%s NULL pointer\n\n",string);\
        return errcode;\
     }\
  }while(0)

/** function ret value check */
#define MEDIA_CHECK_RET(ret,errcode,errstring)\
do{\
    if(0 != ret)\
    {\
        CVI_LOGE("%s failed, s32Ret(0x%08X)\n\n", errstring, ret);\
        return errcode;\
    }\
  }while(0)

/**check init, unlock mutex when error */
#define MEDIA_CHECK_CHECK_INIT(retvalue,errcode,errstring)\
do{\
    if(0 == retvalue)\
    {\
        CVI_LOGE("%s failed, s32Ret(0x%08X)\n\n", errstring, retvalue);\
        return errcode;\
    }\
  }while(0)

typedef enum cviEVENT_SENSOR_E
{
    CVI_EVENT_SENSOR_PLUG_STATUS = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_MEDIA, 0), /**<plug in or plug out event*/
    CVI_EVENT_SENSOR_BUTT
} CVI_EVENT_SENSOR_E;

typedef enum cviSENSOR_PLUG_E
{
    CVI_SENSOR_PLUG_IN = 0,
    CVI_SENSOR_PLUG_OUT,
    CVI_SENSOR_PLUG_BUTT,
} CVI_SENSOR_PLUG_E;

int32_t CVI_MEDIA_StartAudioInTask(void);
int32_t CVI_MEDIA_StopAudioInTask(void);
int32_t CVI_MEDIA_VideoInit(void);
int32_t CVI_MEDIA_VideoDeInit(void);
int32_t CVI_MEDIA_VcapInit(void);
int32_t CVI_MEDIA_VcapDeInit(void);
int32_t CVI_MEDIA_VbInit(void);
int32_t CVI_MEDIA_VbInitPlayBack(void);
int32_t CVI_MEDIA_VbDeInit(void);
int32_t CVI_MEDIA_DispInit(bool windowMode);
int32_t CVI_MEDIA_DispDeInit(void);
int32_t CVI_MEDIA_LiveViewSerInit(void);
int32_t CVI_MEDIA_LiveViewSerDeInit(void);
int32_t CVI_MEDIA_AiInit(void);
int32_t CVI_MEDIA_VencInit(void);
int32_t CVI_MEDIA_VencDeInit(void);
int32_t CVI_MEDIA_AiDeInit(void);
int32_t CVI_MEDIA_AencInit(void);
int32_t CVI_MEDIA_AencDeInit(void);
int32_t CVI_MEDIA_RecordSerInit(void);
int32_t CVI_MEDIA_RecordSerDeInit(void);
int32_t CVI_MEDIA_RtspSerInit(void);
int32_t CVI_MEDIA_RtspSerDeInit(void);
int32_t CVI_MEDIA_AoInit(void);
int32_t CVI_MEDIA_AoDeInit(void);
int32_t CVI_MEDIA_PlayBackSerInit(void);
int32_t CVI_MEDIA_PlayBackSerDeInit(void);
uint32_t CVI_MEDIA_Res2RecordMediaMode(int32_t res);
uint32_t CVI_MEDIA_Res2PhotoMediaMode(int32_t res);
uint32_t CVI_MEDIA_AhdResToRecordMediaSize(int32_t res);
uint32_t CVI_MEDIA_AhdResToPhotoMediaSize(int32_t res);
int32_t CVI_MEDIA_SetAntiFlicker(void);
bool CVI_MEDIA_Is_CameraEnabled(int32_t cam_index);
CVI_MEDIA_PARAM_INIT_S* CVI_MEDIA_GetCtx(void);
int32_t CVI_MEDIA_VIDEOMD_Init(void);
int32_t CVI_MEDIA_VIDEOMD_DeInit(void);
int32_t CVI_MEDIA_APP_RTSP_Init(uint32_t id, char *name);
int32_t CVI_MEDIA_SwitchRTSPChanel(uint32_t value, uint32_t id, char *name);
int32_t CVI_MEDIA_APP_RTSP_DeInit();

int32_t CVI_MEDIA_PhotoVprocInit(void);
int32_t CVI_MEDIA_PhotoVprocDeInit(void);
#ifdef SERVICES_PHOTO_ON
int32_t CVI_MEDIA_PhotoSerInit(void);
int32_t CVI_MEDIA_PhotoSerDeInit(void);
#endif

#ifdef SERVICES_ADAS_ON
int32_t CVI_MEDIA_ADASInit(void);
int32_t CVI_MEDIA_ADASDeInit(void);
#endif

#ifdef SERVICES_QRCODE_ON
int32_t CVI_MEDIA_QRCodeInit(void);
int32_t CVI_MEDIA_QRCodeDeInit(void);
#endif

#ifdef SERVICES_SUBVIDEO_ON
int32_t CVI_MEDIA_StartVideoInTask(void);
int32_t CVI_MEDIA_StopVideoInTask(void);
#endif

int32_t CVI_MEDIA_SensorDet(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
