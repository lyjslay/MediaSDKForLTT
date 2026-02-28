#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "cvi_appcomm.h"
#include "cvi_sysutils.h"
#include "cvi_media_init.h"
#include "cvi_audio_service.h"
#ifdef SERVICES_SUBVIDEO_ON
#include "vcap.h"
#endif
#include "cvi_recordmng.h"
#ifdef SERVICES_PHOTO_ON
#include "cvi_photomng.h"
#endif
#include "cvi_hal_screen.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_param.h"
#include "cvi_system.h"
#include "cvi_media_dump.h"
#include "cvi_eventhub.h"
#include "cvi_hal_pwm.h"
#include "cvi_mode.h"
#ifdef SERVICES_LIVEVIEW_ON
#include "cvi_volmng.h"
#endif

#ifdef CONFIG_GPS_ON
#include "cvi_gpsmng.h"
static CVI_GPSMNG_CALLBACK gstGPSCallback = {0};
static CVI_RECORD_SERVICE_GPS_INFO_S gstGPSInfo = {0};
static pthread_mutex_t gGPSMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifdef ENABLE_VIDEO_MD
#include "cvi_videomd.h"
#define IVE_KO_PATH CVI_KOMOD_PATH "/" CHIP_TYPE "_ive.ko"
#endif

#ifdef SERVICES_ADAS_ON
#include "cvi_adasmng.h"
#define TPU_KO_PATH CVI_KOMOD_PATH "/" CHIP_TYPE "_tpu.ko"
#endif

typedef struct {
    CVI_MAPI_VENC_CHN_ATTR_T venc_attr;
#ifdef SERVICES_RTSP_ON
    CVI_RTSP_SERVICE_PARAM_S *param;
#endif
    pthread_mutex_t mutex;
} RTSP_VENC_CTX;

RTSP_VENC_CTX rtsp_venc_ctx[MAX_RTSP_CNT] = {0};

static CVI_MEDIA_PARAM_INIT_S  SysMediaParams;

CVI_MEDIA_PARAM_INIT_S* CVI_MEDIA_GetCtx(void)
{
    return &SysMediaParams;
}

static int32_t CVI_MEDIA_InitVproc(CVI_PARAM_MEDIA_VPROC_ATTR_S *mediaVprocAttr, CVI_MAPI_VPROC_ATTR_T *mapiVprocAttr)
{
    int32_t i = 0;
    memcpy(&mapiVprocAttr->attr_inp, &(mediaVprocAttr->VpssGrpAttr), sizeof(VPSS_GRP_ATTR_S));

    for(i = 0; i < CVI_MAPI_VPROC_MAX_CHN_NUM; i++) {
        if(mediaVprocAttr->VprocChnAttr[i].VprocChnEnable == true) {
            memcpy(&(mapiVprocAttr->attr_chn[i]), &(mediaVprocAttr->VprocChnAttr[i].VpssChnAttr), sizeof(VPSS_CHN_ATTR_S));
            mapiVprocAttr->chn_vbcnt[i] = mediaVprocAttr->VprocChnAttr[i].VprocChnVbCnt;
            mapiVprocAttr->lowdelay_cnt[i] = mediaVprocAttr->VprocChnAttr[i].VprocChnLowDelayCnt;
            mapiVprocAttr->chn_num++;
        }
    }
    mapiVprocAttr->attr_inp.u8VpssDev = 1;
    return 0;
}

bool CVI_MEDIA_Is_CameraEnabled(int32_t cam_index) {
    bool is_enabled = false;
    CVI_PARAM_GetCamStatus(cam_index, &is_enabled);
    return is_enabled;
}

uint32_t CVI_MEDIA_Res2RecordMediaMode(int32_t res)
{
    uint32_t MediaSize = 0;

    switch (res) {
        case AHD_MODE_1280X720H_NTSC:
        case AHD_MODE_1280X720H_PAL:
        case AHD_MODE_1280X720P25:
            MediaSize = CVI_MEDIA_VIDEO_SIZE_1280X720P25;
            break;
        case AHD_MODE_1280X720P30:
            MediaSize = CVI_MEDIA_VIDEO_SIZE_1280X720P30;
            break;
        case AHD_MODE_1920X1080P25:
            MediaSize = CVI_MEDIA_VIDEO_SIZE_1920X1080P25;
            break;
        case AHD_MODE_1920X1080P30:
            MediaSize = CVI_MEDIA_VIDEO_SIZE_1920X1080P30;
            break;
        case AHD_MODE_2304X1296P25:
            MediaSize = CVI_MEDIA_VIDEO_SIZE_2304X1296P25;
            break;
        case AHD_MODE_2304X1296P30:
            MediaSize = CVI_MEDIA_VIDEO_SIZE_2304X1296P30;
            break;
        default:
            MediaSize = CVI_MEDIA_VIDEO_SIZE_BUIT;
            CVI_LOGE("error ahd res and default setting 1080P25 !\n");
            break;
    }

    return MediaSize;
}

uint32_t CVI_MEDIA_Res2PhotoMediaMode(int32_t res)
{
    uint32_t MediaSize = 0;

    switch (res) {
        case AHD_MODE_1280X720H_NTSC:
        case AHD_MODE_1280X720H_PAL:
        case AHD_MODE_1280X720P25:
        case AHD_MODE_1280X720P30:
        case AHD_MODE_2304X1296P25:
        case AHD_MODE_2304X1296P30:
        default:
            MediaSize = CVI_MEDIA_PHOTO_SIZE_1920X1080P;
            CVI_LOGE("error ahd res and default setting 1080P25 !\n");
            break;
        case AHD_MODE_1920X1080P25:
        case AHD_MODE_1920X1080P30:
            MediaSize = CVI_MEDIA_PHOTO_SIZE_1920X1080P;
            break;
    }

    return MediaSize;
}

static int32_t CVI_MEDIA_SensorSetRes(int32_t snsid, int32_t mode)
{
    if (mode != AHD_MODE_NONE) {
        CVI_PARAM_CFG_S param;
        CVI_PARAM_GetParam(&param);
        param.WorkModeCfg.RecordMode.CamMediaInfo[snsid].CurMediaMode = CVI_MEDIA_Res2RecordMediaMode(mode);
        param.WorkModeCfg.PhotoMode.CamMediaInfo[snsid].CurMediaMode = CVI_MEDIA_Res2PhotoMediaMode(mode);
        CVI_PARAM_SetParam(&param);
    }
    return 0;
}

static int32_t CVI_MEDIA_SensorPlugCallback(int32_t snsid, int32_t mode)
{
    CVI_LOGI("Sensor %d mode :%d\n", snsid, mode);
    CVI_EVENT_S stEvent;
    stEvent.aszPayload[0] = mode;
    stEvent.aszPayload[1] = snsid;

    if (mode == AHD_MODE_NONE) {
        stEvent.arg1 = CVI_SENSOR_PLUG_OUT;
    } else {
        stEvent.arg1 = CVI_SENSOR_PLUG_IN;
    }
    stEvent.topic = CVI_EVENT_SENSOR_PLUG_STATUS;
    stEvent.arg2 = 1;
    CVI_EVENTHUB_Publish(&stEvent);
    return 0;
}

int32_t CVI_MEDIA_SensorDet(void)
{
    int32_t mode = AHD_MODE_NONE;
    int32_t status = 0;
    for(uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++){
        CVI_MAPI_VCAP_GetAhdMode(i, &mode, &status);
        CVI_PARAM_SetCamStatus(i, status);
        if (i == 0){
            CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SENSOR_PLUG_STATUS);
        }
        /*RGB SENSOR mode == AHD_MODE_BUIT*/
        if (mode != -1){/*YUV SENSOR*/
            if(mode == AHD_MODE_NONE){
                CVI_PARAM_GetAhdDefaultMode(i, &mode);
            }
            CVI_MEDIA_SensorSetRes(i, mode);
            if (MAX_DEV_INSTANCES == 1 && i >= 1){
                CVI_MAPI_VCAP_InitSensorDetect(i, (void *)CVI_MEDIA_SensorPlugCallback);
                continue;
            }
            CVI_MAPI_VCAP_SetAhdMode(i, mode);
            CVI_MAPI_VCAP_InitSensorDetect(i, (void *)CVI_MEDIA_SensorPlugCallback);
        }

    #ifdef SERVICES_LIVEVIEW_ON
        CVI_PARAM_WND_ATTR_S *WndParam = &CVI_PARAM_GetCtx()->pstCfg->MediaComm.Window;
        WndParam->Wnds[i].WndEnable = status;
    #endif
    }

    return 0;
}

static int32_t CVI_MEDIA_SensorInit(void)
{
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_MAPI_VCAP_ATTR_T vcap_attr;
    int32_t s32Ret = 0;
    int32_t status = 0;

    s32Ret = CVI_MAPI_VCAP_GetGeneralVcapAttr(&vcap_attr);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_GetGeneralVcapAttr fail");

    /*
     *  user can select diffrent params base on genral attribute,
     *  such as sensor size/wdr mode/compress mode/i2c addr...
     */
    for (uint32_t i = 0; i < MAX_DEV_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PARAM_MEDIA_SPEC_S params;
        memset(&params, 0x0, sizeof(CVI_PARAM_MEDIA_SPEC_S));
        CVI_PARAM_GetMediaMode(i, &params);
        memcpy(&vcap_attr.attr_chn[i], &(params.VcapAttr.VcapChnAttr), sizeof(CVI_MAPI_VCAP_CHN_ATTR_T));
        memcpy(&vcap_attr.attr_sns[i], &(params.SnsAttr.SnsChnAttr), sizeof(CVI_MAPI_VCAP_SENSOR_ATTR_T));
        status++;
    }
    vcap_attr.u8DevNum = status;

    for (uint32_t i = 0; i < MAX_DEV_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PARAM_MEDIA_SPEC_S params;
        CVI_PARAM_GetMediaMode(i, &params);

        s32Ret = CVI_MAPI_VCAP_InitSensor(&Syshdl->sns[i], params.SnsAttr.SnsChnAttr.u8SnsId, &vcap_attr);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_InitSensor fail");

        if(i == 0){
            s32Ret = CVI_MAPI_VCAP_SetPqBinPath(Syshdl->sns[i]);
            MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_SetPqBinPath fail");
        }

        s32Ret = CVI_MAPI_VCAP_StartDev(Syshdl->sns[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_StartDev fail");
    }

    for (uint32_t i = 0; i < MAX_DEV_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        s32Ret = CVI_MAPI_VCAP_StartPipe(Syshdl->sns[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_StartPipe fail");

        s32Ret = CVI_MAPI_VCAP_InitISP(Syshdl->sns[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_InitISP fail");

        s32Ret = CVI_MAPI_VCAP_StartChn(Syshdl->sns[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_StartChn fail");
    }

    for (uint32_t i = 0; i < MAX_DEV_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PARAM_MEDIA_SPEC_S params;
        CVI_PARAM_GetMediaMode(i, &params);
        if (params.VcapAttr.VcapChnAttr.bFlip == 1 || params.VcapAttr.VcapChnAttr.bMirror == 1) {
            CVI_MAPI_VCAP_MIRRORFLIP_ATTR_S Attr;
            Attr.bFlip = params.VcapAttr.VcapChnAttr.bFlip;
            Attr.bMirror = params.VcapAttr.VcapChnAttr.bMirror;
            CVI_MAPI_VCAP_SetAttrEx(Syshdl->sns[i], CVI_MAPI_VCAP_CMD_MirrorFlip, (void*)&Attr, sizeof(CVI_MAPI_VCAP_MIRRORFLIP_ATTR_S));
        }
        //set fps
        float fps = params.VcapAttr.VcapChnAttr.f32Fps;
        CVI_MAPI_VCAP_SetAttrEx(Syshdl->sns[i], CVI_MAPI_VCAP_CMD_Fps, (void*)&fps, sizeof(float));
    }

    for (uint32_t i = 0; i < MAX_DEV_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        status = 0;
        CVI_MAPI_VCAP_GetSensorPipeAttr(Syshdl->sns[i], &status);

        CVI_PARAM_SetCamIspInfoStatus(i, status);
    }
    CVI_MEDIA_SetAntiFlicker();
    return 0;
}

static int32_t CVI_MEDIA_SensorDeInit(void)
{
    int32_t s32Ret = 0;

    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    for (uint32_t i = 0; i < MAX_DEV_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        s32Ret = CVI_MAPI_VCAP_DeInitISP(Syshdl->sns[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_DeInitISP fail");

        s32Ret = CVI_MAPI_VCAP_StopChn(Syshdl->sns[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_StopChn fail");

        s32Ret = CVI_MAPI_VCAP_StopPipe(Syshdl->sns[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_StopPipe fail");

        s32Ret = CVI_MAPI_VCAP_StopDev(Syshdl->sns[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_StopDev fail");

        s32Ret = CVI_MAPI_VCAP_DeinitSensor(Syshdl->sns[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_DeinitSensor fail");
        Syshdl->sns[i] = NULL;
    }
    return 0;
}

static int32_t CVI_MEDIA_VprocInit(void)
{
    int32_t j = 0;
    int32_t s32Ret = 0;
    CVI_PARAM_WORK_MODE_S Workmode;
    CVI_PARAM_GetWorkModeParam(&Workmode);
    CVI_PARAM_VPSS_ATTR_S Vpssmode = {0};
    int32_t  s32CurMode = CVI_MODEMNG_GetCurWorkMode();
    if (CVI_WORK_MODE_MOVIE == s32CurMode) {
        memcpy(&Vpssmode, &Workmode.RecordMode.Vpss, sizeof(CVI_PARAM_VPSS_ATTR_S));
    } else if (CVI_WORK_MODE_PHOTO == s32CurMode) {
        memcpy(&Vpssmode, &Workmode.PhotoMode.Vpss, sizeof(CVI_PARAM_VPSS_ATTR_S));
    } else {
        CVI_LOGE("This mode has no vpss parameters!\n");
    }
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;

    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PARAM_MEDIA_SPEC_S params;
        CVI_PARAM_GetMediaMode(i, &params);
        CVI_MAPI_VPROC_ATTR_T vproc_attr;
        memset((void*)&vproc_attr, 0, sizeof(vproc_attr));

        s32Ret = CVI_MEDIA_InitVproc(&params.VprocAttr, &vproc_attr);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VCAP_SetDumpRawAttr fail");
        if (Vpssmode.stVIVPSSMode.aenMode[i] == VI_OFFLINE_VPSS_OFFLINE) {
            for(int32_t j = 0; j < VPSS_IP_NUM; j++) {
                if (Vpssmode.stVPSSMode.aenInput[j] == VPSS_INPUT_MEM) {
                    vproc_attr.attr_inp.u8VpssDev = 1;
                    break;
                }
            }
        }
        s32Ret = CVI_MAPI_VPROC_Init(&Syshdl->vproc[i], params.VprocAttr.Vprocid, &vproc_attr);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VPROC_Init fail");

        for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
            CVI_LOGD("ExtChnAttr %d\n", params.VprocAttr.ExtChnAttr[j].ChnEnable);
            if (params.VprocAttr.ExtChnAttr[j].ChnEnable == false) {
                continue;
            }
            CVI_MAPI_EXTCHN_ATTR_T extchn_attr = {0};
            memcpy(&extchn_attr, &params.VprocAttr.ExtChnAttr[j].ChnAttr, sizeof(CVI_MAPI_EXTCHN_ATTR_T));

            s32Ret = CVI_MAPI_VPROC_SetExtChnAttr(Syshdl->vproc[i], &extchn_attr);
            MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VPROC_SetExtChnAttr fail");
        }
        //offline
        if (Vpssmode.stVIVPSSMode.aenMode[i] == VI_OFFLINE_VPSS_OFFLINE) {
            uint8_t id = (MAX_DEV_INSTANCES == 1) ? 0 : i;
            s32Ret = CVI_MAPI_VPROC_BindVcap(Syshdl->vproc[i], Syshdl->sns[id], i);
            MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VPROC_BindVcap fail");
        }
    }

    return 0;
}

int32_t CVI_MEDIA_PhotoVprocInit(void)
{
    uint32_t Photowidth = 0;
    uint32_t Photoheight = 0;
    CVI_PARAM_MEDIA_COMM_S mediaparams;
    CVI_PARAM_GetMediaComm(&mediaparams);
    CVI_PARAM_PHOTO_ATTR_S  *Photo_attr = &mediaparams.Photo;
    CVI_PARAM_MEDIA_SPEC_S params;

    for(uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++){
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PARAM_GetMediaMode(i, &params);
        for (int32_t j = 0; j < MAX_VENC_CNT; j++){
            if (params.VencAttr.VencChnAttr[j].VencId == Photo_attr->ChnAttrs[i].BindVencId){
                if(params.VencAttr.VencChnAttr[j].MapiVencAttr.width > Photowidth){
                    Photowidth = params.VencAttr.VencChnAttr[j].MapiVencAttr.width;
                    Photoheight = params.VencAttr.VencChnAttr[j].MapiVencAttr.height;
                }
                break;
            }
        }
    }

    int32_t s32Ret = 0;

    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;

    CVI_MAPI_VPROC_ATTR_T g_vproc_attr = {0};
    g_vproc_attr = CVI_MAPI_VPROC_DefaultAttr_OneChn(
            1920, 1080, PIXEL_FORMAT_NV21,
            Photowidth, Photoheight, PIXEL_FORMAT_NV21);

    g_vproc_attr.attr_chn[0].stAspectRatio.enMode                = ASPECT_RATIO_MANUAL;
    g_vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32X      = 0;
    g_vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32Y      = 0;
    g_vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Width  = Photowidth;
    g_vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Height = Photoheight;
    g_vproc_attr.attr_chn[0].stAspectRatio.bEnableBgColor = CVI_TRUE;
    g_vproc_attr.attr_chn[0].stAspectRatio.u32BgColor = 0x00000000; // BLACK
    g_vproc_attr.attr_inp.u8VpssDev = Photo_attr->VprocDev_id;
    g_vproc_attr.chn_vbcnt[0] = 1;

    s32Ret = CVI_MAPI_VPROC_Init(&Syshdl->vproc[MAX_VPROC_CNT - 1], -1, &g_vproc_attr);
    if (s32Ret != CVI_MAPI_SUCCESS) {
        CVI_LOGE("CVI_MAPI_VPROC_Init Photo failed\n");
        return s32Ret;
    }
    CVI_LOGI("Window mode, g_vproc_attr photo Created with VPSS grp %d\n", CVI_MAPI_VPROC_GetGrp(Syshdl->vproc[MAX_VPROC_CNT - 1]));

    s32Ret = CVI_MAPI_VPROC_EnableTileMode();
    if (s32Ret != CVI_MAPI_SUCCESS) {
        CVI_LOGE("CVI_MAPI_VPROC_EanbleTileMode Photo failed\n");
        return s32Ret;
    }

    return 0;
}

static int32_t CVI_MEDIA_VprocExtDeInit(void)
{
    int32_t s32Ret = 0;
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        s32Ret = CVI_MAPI_VPROC_ExtChnStop(Syshdl->vproc[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VPROC_ExtChnStop fail");
    }

    return 0;
}

static int32_t CVI_MEDIA_VprocDeInit(void)
{
    int32_t s32Ret = 0;
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_PARAM_WORK_MODE_S Workmode = {0};
    CVI_PARAM_GetWorkModeParam(&Workmode);
    CVI_PARAM_VPSS_ATTR_S Vpssmode = {0};
    int32_t  s32CurMode = CVI_MODEMNG_GetCurWorkMode();
    if (CVI_WORK_MODE_MOVIE == s32CurMode) {
        memcpy(&Vpssmode, &Workmode.RecordMode.Vpss, sizeof(CVI_PARAM_VPSS_ATTR_S));
    } else if (CVI_WORK_MODE_PHOTO == s32CurMode) {
        memcpy(&Vpssmode, &Workmode.PhotoMode.Vpss, sizeof(CVI_PARAM_VPSS_ATTR_S));
    } else {
        CVI_LOGE("This mode has no vpss parameters!\n");
    }
    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
            //offline
        if(Vpssmode.stVIVPSSMode.aenMode[i] == VI_OFFLINE_VPSS_OFFLINE) {
            uint8_t id = (MAX_DEV_INSTANCES == 1) ? 0 : i;
            s32Ret = CVI_MAPI_VPROC_UnBindVcap(Syshdl->vproc[i], Syshdl->sns[id], i);
            MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VPROC_UnBindVcap fail");
        }
        s32Ret = CVI_MAPI_VPROC_Deinit(Syshdl->vproc[i]);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VPROC_Deinit fail");
        Syshdl->vproc[i] = NULL;
    }

    return 0;
}

int32_t CVI_MEDIA_VIDEOMD_Init(void)
{
#if defined (ENABLE_VIDEO_MD)
    cvi_insmod(IVE_KO_PATH, NULL);

    int32_t s32Ret = 0;
    CVI_MOTION_DETECT_ATTR_S mdAttr;
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_PARAM_MD_ATTR_S Md = {0};
    CVI_PARAM_GetMdConfigParam(&Md);
    CVI_LOGD(" Md->motionSensitivity = %d\n",  Md.motionSensitivity);

    mdAttr.threshold = Md.motionSensitivity;

    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PARAM_MEDIA_SPEC_S params;
        CVI_PARAM_GetMediaMode(i, &params);
        mdAttr.camid = i;

        for (int32_t j = 0; j < CVI_MAPI_VPROC_MAX_CHN_NUM; j++) {
            if(params.VprocAttr.VprocChnAttr[j].VprocChnEnable == true
            && params.VprocAttr.VprocChnAttr[j].VprocChnid == Md.ChnAttrs[i].BindVprocChnId)
            {
                mdAttr.isExtVproc = 0;
                mdAttr.w = params.VprocAttr.VprocChnAttr[j].VpssChnAttr.u32Width;
                mdAttr.h = params.VprocAttr.VprocChnAttr[j].VpssChnAttr.u32Height;
            }else if(params.VprocAttr.ExtChnAttr[j].ChnEnable == true
                &&  params.VprocAttr.ExtChnAttr[j].ChnAttr.ChnId == Md.ChnAttrs[i].BindVprocChnId)
            {
                mdAttr.isExtVproc = 1;
                mdAttr.w = params.VprocAttr.ExtChnAttr[j].ChnAttr.VpssChnAttr.u32Width;
                mdAttr.h = params.VprocAttr.ExtChnAttr[j].ChnAttr.VpssChnAttr.u32Height;
            }else{
                continue;
            }
        }

        mdAttr.vprocChnId = Md.ChnAttrs[i].BindVprocChnId;
        for (int32_t z = 0; z < MAX_VPROC_CNT; z++) {
            if(Syshdl->vproc[z] != NULL && Md.ChnAttrs[i].BindVprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(Syshdl->vproc[z])) {
                mdAttr.vprocHandle = Syshdl->vproc[z];
                break;
            }
        }

        mdAttr.state = Md.ChnAttrs[i].Enable;
        s32Ret = CVI_MOTION_DETECT_Init(&mdAttr);
        if (s32Ret != 0) {
            MEDIA_CHECK_RET(s32Ret, s32Ret, "CVI_MEDIA_VIDEOMD_Init fail");
            return s32Ret;
        }
    }
#endif

    return 0;
}

int32_t CVI_MEDIA_VIDEOMD_DeInit(void)
{
#if defined (ENABLE_VIDEO_MD)
    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_MOTION_DETECT_DeInit(i);
    }
    cvi_rmmod(IVE_KO_PATH);
#endif

    return 0;
}

int32_t CVI_MEDIA_PhotoVprocDeInit(void)
{
    int32_t s32Ret = 0;
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    s32Ret = CVI_MAPI_VPROC_DisableTileMode();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VPROC_DisableTileMode fail");

    s32Ret = CVI_MAPI_VPROC_Deinit(Syshdl->vproc[MAX_VPROC_CNT - 1]);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_PhotoVprocDeInit fail");

    Syshdl->vproc[MAX_VPROC_CNT - 1] = NULL;

    return s32Ret;
}

int32_t CVI_MEDIA_VencInit(void)
{
    int32_t j = 0;
    int32_t s32Ret = 0;
    CVI_PARAM_MEDIA_COMM_S mediaparams;
    CVI_PARAM_GetMediaComm(&mediaparams);

    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PARAM_MEDIA_SPEC_S params;
        memset(&params, 0x0, sizeof(CVI_PARAM_MEDIA_SPEC_S));
        CVI_MAPI_VENC_CHN_ATTR_T venc_attr = {0};
        CVI_PARAM_GetMediaMode(i, &params);
        for (j = 0; j < MAX_VENC_CNT; j++) {
            if (params.VencAttr.VencChnAttr[j].VencChnEnable == true) {
                 memcpy(&venc_attr.venc_param, &params.VencAttr.VencChnAttr[j].MapiVencAttr, sizeof(CVI_MAPI_VENC_CHN_PARAM_T));
#ifndef SERVICES_SUBVIDEO_ON
#ifdef SERVICES_RTSP_ON
                CVI_PARAM_RTSP_CHN_ATTR_S       *rtspattr = &mediaparams.Rtsp.ChnAttrs[i];
                if (rtspattr->BindVencId == params.VencAttr.VencChnAttr[j].VencId) {
                    memcpy(&rtsp_venc_ctx[i].venc_attr, &venc_attr, sizeof(CVI_MAPI_VENC_CHN_ATTR_T));
                } else
#endif
                {
                    CVI_MAPI_VENC_HANDLE_T *venc_hdl = &CVI_MEDIA_GetCtx()->SysHandle.venchdl[i][j];
                    s32Ret = CVI_MAPI_VENC_InitChn(venc_hdl, &venc_attr);
                    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VENC_InitChn fail");
                    if((params.VencAttr.VencChnAttr[j].MapiVencAttr.codec == CVI_MAPI_VCODEC_H264)
                    || (params.VencAttr.VencChnAttr[j].MapiVencAttr.codec == CVI_MAPI_VCODEC_H265)){
                        s32Ret = CVI_MAPI_VENC_SetDataFifoLen(*venc_hdl, params.VencAttr.VencChnAttr[j].MapiVencAttr.datafifoLen);
                        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VENC_SetDataFifoLen fail");
                        uint32_t datafifoLen = 0;
                        s32Ret = CVI_MAPI_VENC_GetDataFifoLen(*venc_hdl, &datafifoLen);
                        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VENC_GetDataFifoLen fail");
                        CVI_LOGI("snsid %u chnid %d datafifoLen %u", i, CVI_MAPI_VENC_GetChn(*venc_hdl), datafifoLen);
                    }
                }
#else
#ifdef SERVICES_RTSP_ON
                CVI_PARAM_RTSP_CHN_ATTR_S       *rtspattr = &mediaparams.Rtsp.ChnAttrs[i];
                if (rtspattr->BindVencId == params.VencAttr.VencChnAttr[j].VencId) {
                    memcpy(&rtsp_venc_ctx[i].venc_attr, &venc_attr, sizeof(CVI_MAPI_VENC_CHN_ATTR_T));
                }
#endif
                CVI_MAPI_VENC_HANDLE_T *venc_hdl = &CVI_MEDIA_GetCtx()->SysHandle.venchdl[i][j];
                s32Ret = CVI_MAPI_VENC_InitChn(venc_hdl, &venc_attr);
                MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VENC_InitChn fail");
                if((params.VencAttr.VencChnAttr[j].MapiVencAttr.codec == CVI_MAPI_VCODEC_H264)
                || (params.VencAttr.VencChnAttr[j].MapiVencAttr.codec == CVI_MAPI_VCODEC_H265)){
                    s32Ret = CVI_MAPI_VENC_SetDataFifoLen(*venc_hdl, params.VencAttr.VencChnAttr[j].MapiVencAttr.datafifoLen);
                    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VENC_SetDataFifoLen fail");
                    uint32_t datafifoLen = 0;
                    s32Ret = CVI_MAPI_VENC_GetDataFifoLen(*venc_hdl, &datafifoLen);
                    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VENC_GetDataFifoLen fail");
                    CVI_LOGI("snsid %u chnid %d datafifoLen %u", i, CVI_MAPI_VENC_GetChn(*venc_hdl), datafifoLen);
                }
#endif
            }
        }
    }

    return 0;
}

int32_t CVI_MEDIA_VencDeInit(void)
{
    int32_t j = 0;
    int32_t s32Ret = 0;
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;

    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PARAM_MEDIA_SPEC_S params;
        CVI_PARAM_GetMediaMode(i, &params);

        for (j = 0; j < MAX_VENC_CNT; j++) {
            if (Syshdl->venchdl[i][j] != NULL) {
                CVI_MAPI_VENC_StopRecvFrame(Syshdl->venchdl[i][j]);
                s32Ret = CVI_MAPI_VENC_DeinitChn(Syshdl->venchdl[i][j]);
                MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_VENC_DeinitChn fail");
                Syshdl->venchdl[i][j] = NULL;
            }
        }
    }

    return 0;
}

int32_t CVI_MEDIA_VcapInit(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_MEDIA_SensorInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_SensorInit fail");

    s32Ret = CVI_MEDIA_VprocInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_VprocInit fail");

    return 0;
}

int32_t CVI_MEDIA_VcapDeInit(void)
{
    int32_t s32Ret = 0;

    s32Ret = CVI_MEDIA_VprocExtDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_VprocExtDeInit fail");

    s32Ret = CVI_MEDIA_VprocDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_VprocDeInit fail");

    s32Ret = CVI_MEDIA_SensorDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_SensorDeInit fail");

    return 0;
}

int32_t CVI_MEDIA_VideoInit(void)
{
    int32_t s32Ret = 0;

    #ifdef ENABLE_ISP_PQ_TOOL
    CVI_MEDIA_DUMP_ReplayInit();
    #endif

    s32Ret = CVI_MEDIA_SensorInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_SensorInit fail");

    s32Ret = CVI_MEDIA_VprocInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_VprocInit fail");

    return 0;
}

int32_t CVI_MEDIA_VideoDeInit(void)
{
    int32_t s32Ret = 0;

    s32Ret = CVI_MEDIA_VprocExtDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_VprocExtDeInit fail");

    s32Ret = CVI_MEDIA_VprocDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_VprocDeInit fail");

    s32Ret = CVI_MEDIA_SensorDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_SensorDeInit fail");

    return 0;
}

int32_t CVI_MEDIA_VbInit(void)
{
    CVI_MAPI_MEDIA_SYS_ATTR_T sys_attr = {0};
    int32_t s32Ret = 0;

    CVI_PARAM_GetVbParam(&sys_attr);

    s32Ret = CVI_MAPI_Media_Init(&sys_attr);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_Media_Init fail");

    return 0;
}

int32_t CVI_MEDIA_VbInitPlayBack(void)
{
    CVI_MAPI_MEDIA_SYS_ATTR_T sys_attr = {0};
    CVI_PARAM_DISP_ATTR_S  disp_attr;
    int32_t s32Ret = 0;

    CVI_PARAM_GetVbParam(&sys_attr);
    CVI_PARAM_GetVoParam(&disp_attr);

    if (disp_attr.Rotate == 0) {
        sys_attr.vb_pool[0].vb_blk_size.frame.width = disp_attr.Width;
        sys_attr.vb_pool[0].vb_blk_size.frame.height = disp_attr.Height;
    } else {
        sys_attr.vb_pool[0].vb_blk_size.frame.width = disp_attr.Height;
        sys_attr.vb_pool[0].vb_blk_size.frame.height = disp_attr.Width;
    }
    sys_attr.vb_pool_num = 1;
    sys_attr.vb_pool[0].is_frame = true;
    sys_attr.vb_pool[0].vb_blk_size.frame.fmt = 13;
    sys_attr.vb_pool[0].vb_blk_num = 5;
    //set offline
    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        sys_attr.stVIVPSSMode.aenMode[i] = VI_OFFLINE_VPSS_OFFLINE;
    }
    sys_attr.stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
    sys_attr.stVPSSMode.aenInput[1] = VPSS_INPUT_MEM;
    s32Ret = CVI_MAPI_Media_Init(&sys_attr);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_Media_Init fail");

    return 0;
}

int32_t CVI_MEDIA_VbInitRecAndPlay(void) {
    CVI_MAPI_MEDIA_SYS_ATTR_T sys_attr = {0};
    CVI_PARAM_DISP_ATTR_S  disp_attr;
    int32_t s32Ret = 0;
    int32_t record_vb_num = 0;

    // vb configure from ini
    CVI_PARAM_GetVbParam(&sys_attr);
    CVI_PARAM_GetVoParam(&disp_attr);
    record_vb_num = sys_attr.vb_pool_num;

    // set vb attr for playback
    sys_attr.vb_pool_num  = record_vb_num + 1;
    if (disp_attr.Rotate == 0) {
        sys_attr.vb_pool[record_vb_num].vb_blk_size.frame.width = disp_attr.Width;
        sys_attr.vb_pool[record_vb_num].vb_blk_size.frame.height = disp_attr.Height;
    } else {
        sys_attr.vb_pool[record_vb_num].vb_blk_size.frame.width = disp_attr.Height;
        sys_attr.vb_pool[record_vb_num].vb_blk_size.frame.height = disp_attr.Width;
    }
    sys_attr.vb_pool[record_vb_num].is_frame = true;
    sys_attr.vb_pool[record_vb_num].vb_blk_size.frame.fmt = 13;
    sys_attr.vb_pool[record_vb_num].vb_blk_num = 5;
    //set vpss offline
    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        //sys_attr.stVIVPSSMode.aenMode[i] = VI_OFFLINE_VPSS_OFFLINE; // still remain online
    }
    sys_attr.stVPSSMode.aenInput[0] = VPSS_INPUT_MEM; // vpss dev0(1 in 1 out) from ddr
    sys_attr.stVPSSMode.aenInput[1] = VPSS_INPUT_ISP; // vpss dev1(1 in 3 out) online with isp

    s32Ret = CVI_MAPI_Media_Init(&sys_attr);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_Media_Init fail");

    return 0;
}

int32_t CVI_MEDIA_VbDeInit(void)
{
    int32_t s32Ret = 0;

    s32Ret = CVI_MAPI_Media_Deinit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_Media_Deinit fail");

    return 0;
}

int32_t CVI_MEDIA_DispInit(bool windowMode)
{
    int32_t ret = CVI_MAPI_SUCCESS;
#ifdef CONFIG_SCREEN_ON
    CVI_PARAM_DISP_ATTR_S params;
    int32_t s32Ret = 0;
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_PARAM_GetVoParam(&params);

    if (Syshdl->dispHdl == NULL) {
        MEDIA_DispCfg dispCfg;
        dispCfg.dispAttr.width = params.Width;
        dispCfg.dispAttr.height = params.Height;
        dispCfg.dispAttr.rotate = params.Rotate;
        dispCfg.dispAttr.fps    = params.Fps;
        dispCfg.dispAttr.window_mode = windowMode;
        dispCfg.dispAttr.stPubAttr.u32BgColor = COLOR_10_RGB_BLACK;
        dispCfg.dispAttr.stPubAttr.enIntfSync = VO_OUTPUT_USER;
        dispCfg.videoLayerAttr.u32BufLen = 3;
        // TODO cv182x
        dispCfg.videoLayerAttr.u32PixelFmt = PIXEL_FORMAT_NV21;

        (CVI_HAL_SCREEN_Register(CVI_HAL_SCREEN_IDX_0, &stHALSCREENObj));

        CVI_PARAM_PWM_S Param;
        s32Ret = CVI_PARAM_GetPWMParam(&Param);
        if(s32Ret == 0) {
            CVI_HAL_SCREEN_SetLuma(CVI_HAL_SCREEN_IDX_0, Param.PWMCfg);
        } else {
            CVI_LOGE("%s : CVI_PARAM_GetPWMParam failed\n",__func__);
        }
        CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_0, CVI_HAL_SCREEN_STATE_ON);

        CVI_LOGD("init panel app ======================");
        // CHECK_RET(CVI_HAL_SCREEN_Init(CVI_HAL_SCREEN_IDX_0));
        CVI_HAL_SCREEN_ATTR_S screenAttr = {0};
        CVI_HAL_SCREEN_GetAttr(CVI_HAL_SCREEN_IDX_0, &screenAttr);
        switch (screenAttr.enType) {
            case CVI_HAL_SCREEN_INTF_TYPE_MIPI:
                dispCfg.dispAttr.stPubAttr.enIntfType = VO_INTF_MIPI;
                break;
            case CVI_HAL_SCREEN_INTF_TYPE_LCD:
                // dispCfg.dispAttr.stPubAttr.enIntfType = VO_INTF_LCD;
                // dispCfg.dispAttr.stPubAttr.stLcdCfg = *(VO_LCD_CFG_S *)&screenAttr.unScreenAttr.stLcdAttr.stLcdCfg;
                break;
            case CVI_HAL_SCREEN_INTF_TYPE_I80:
                dispCfg.dispAttr.stPubAttr.enIntfType = VO_INTF_I80_HW;
                break;
            default:
                CVI_LOGD("Invalid screen type\n");
                return CVI_MAPI_ERR_FAILURE;
        }

        dispCfg.dispAttr.stPubAttr.stSyncInfo.bSynm   = 1; /**<sync mode: signal */
        dispCfg.dispAttr.stPubAttr.stSyncInfo.bIop    = 1; /**<progressive display */
        dispCfg.dispAttr.stPubAttr.stSyncInfo.u16FrameRate = screenAttr.stAttr.u32Framerate;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.u16Vact = screenAttr.stAttr.stSynAttr.u16Vact;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.u16Vbb  = screenAttr.stAttr.stSynAttr.u16Vbb;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.u16Vfb  = screenAttr.stAttr.stSynAttr.u16Vfb;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.u16Hact = screenAttr.stAttr.stSynAttr.u16Hact;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.u16Hbb  = screenAttr.stAttr.stSynAttr.u16Hbb;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.u16Hfb  = screenAttr.stAttr.stSynAttr.u16Hfb;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.u16Hpw  = screenAttr.stAttr.stSynAttr.u16Hpw;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.u16Vpw  = screenAttr.stAttr.stSynAttr.u16Vpw;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.bIdv    = screenAttr.stAttr.stSynAttr.bIdv;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.bIhs    = screenAttr.stAttr.stSynAttr.bIhs;
        dispCfg.dispAttr.stPubAttr.stSyncInfo.bIvs    = screenAttr.stAttr.stSynAttr.bIvs;

        dispCfg.dispAttr.pixel_format = dispCfg.videoLayerAttr.u32PixelFmt;

        dispCfg.videoLayerAttr.u32VLFrameRate = screenAttr.stAttr.u32Framerate;
        dispCfg.videoLayerAttr.stImageSize.u32Width  = screenAttr.stAttr.u32Width;
        dispCfg.videoLayerAttr.stImageSize.u32Height = screenAttr.stAttr.u32Height;

        s32Ret = CVI_MAPI_DISP_Init(&dispCfg.dispHdl, 0, &dispCfg.dispAttr);
	    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_DISP_Init fail");

        s32Ret = CVI_MAPI_DISP_Start(dispCfg.dispHdl, &dispCfg.videoLayerAttr);
	    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_DISP_Start fail");
        Syshdl->dispHdl = dispCfg.dispHdl;
    }
#endif
    return ret;
}

int32_t CVI_MEDIA_DispDeInit(void)
{
    #ifdef CONFIG_SCREEN_ON
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    int32_t s32Ret = 0;

    s32Ret = CVI_MAPI_DISP_Stop(Syshdl->dispHdl);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_DISP_Stop fail");

    s32Ret = CVI_MAPI_DISP_Deinit(Syshdl->dispHdl);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_DISP_Deinit fail");
    Syshdl->dispHdl = NULL;
    #endif
    return 0;
}

int32_t CVI_MEDIA_LiveViewSerInit(void)
{
    #ifdef CONFIG_SCREEN_ON
    uint32_t i = 0, j = 0;
    int32_t s32Ret = 0;
    CVI_LIVEVIEW_SERVICE_HANDLE_T *plvHdl           = &CVI_MEDIA_GetCtx()->SysServices.LvHdl;
    CVI_LIVEVIEW_SERVICE_PARAM_S *plvParams = &CVI_MEDIA_GetCtx()->SysServices.LvParams;
    CVI_MEDIA_SYSHANDLE_S *SysHandle        = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_PARAM_WND_ATTR_S *WndParam          = &CVI_PARAM_GetCtx()->pstCfg->MediaComm.Window;

    uint32_t wndNum = 0;
    for (i = 0; i < WndParam->WndCnt; i++) {
        WndParam->Wnds[i].SmallWndEnable = false;
        if(WndParam->Wnds[i].WndEnable == false){
            continue;
        }
        wndNum++;
    }

    plvParams->WndCnt = WndParam->WndCnt;
    for (i = 0; i < WndParam->WndCnt; i++) {
        if(wndNum > 1){
            WndParam->Wnds[i].SmallWndEnable = true;
        }
        memcpy(&plvParams->LiveviewService[i].wnd_attr, &WndParam->Wnds[i], sizeof(CVI_LIVEVIEW_SERVICE_WNDATTR_S));
        for (j = 0; j < MAX_VPROC_CNT; j++) {
            if ((SysHandle->vproc[j] != NULL) && (WndParam->Wnds[i].BindVprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(SysHandle->vproc[j]))) {
                plvParams->LiveviewService[i].vproc_hdl = SysHandle->vproc[j];
            }
        }
    }
    s32Ret = CVI_LIVEVIEW_SERVICE_Create(plvHdl, plvParams);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_LIVEVIEW_SERVICE_Create fail");
    #endif
    return 0;
}

int32_t CVI_MEDIA_LiveViewSerDeInit()
{
    #ifdef CONFIG_SCREEN_ON
    int32_t s32Ret = 0;

    s32Ret = CVI_LIVEVIEW_SERVICE_Destroy(CVI_MEDIA_GetCtx()->SysServices.LvHdl);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_LIVEVIEW_SERVICE_Destroy fail");
    #endif
    return 0;
}

int32_t CVI_MEDIA_AiInit(void)
{
    CVI_MEDIA_SYSHANDLE_S *SysHandle = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_MAPI_ACAP_ATTR_S attr = {0};
    int32_t s32Ret = 0;

    CVI_PARAM_GetAiParam(&attr);

    s32Ret = CVI_MAPI_ACAP_Init(&SysHandle->aihdl, &attr);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_ACAP_Init fail");

    s32Ret = CVI_MAPI_ACAP_Start(SysHandle->aihdl);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_ACAP_Start fail");

    return 0;
}

int32_t CVI_MEDIA_AiDeInit(void)
{
    int32_t s32Ret = 0;
    CVI_MEDIA_SYSHANDLE_S *SysHandle = &CVI_MEDIA_GetCtx()->SysHandle;

    s32Ret = CVI_MAPI_ACAP_Deinit(SysHandle->aihdl);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_ACAP_Deinit fail");

    return 0;
}

int32_t CVI_MEDIA_AencInit(void)
{
    CVI_MEDIA_SYSHANDLE_S *SysHandle = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_MAPI_AENC_ATTR_S attr = {0};
    int32_t s32Ret = 0;

    CVI_PARAM_GetAencParam(&attr);
    if (attr.enAencFormat >= CVI_MAPI_AUDIO_CODEC_BUTT) {
        return 0;
    }

    s32Ret = CVI_MAPI_AENC_Init(&SysHandle->aenchdl, &attr);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_AENC_Init fail");

    s32Ret = CVI_MAPI_AENC_Start(SysHandle->aenchdl);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_AENC_Start fail");

    s32Ret = CVI_MAPI_AENC_BindACap(0,0,0,0);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_AENC_BindACap failed");

    return 0;
}

int32_t CVI_MEDIA_AencDeInit(void)
{
    CVI_MEDIA_SYSHANDLE_S *SysHandle = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_MAPI_AENC_ATTR_S attr = {0};
    int32_t s32Ret = 0;

    CVI_PARAM_GetAencParam(&attr);
    if (attr.enAencFormat >= CVI_MAPI_AUDIO_CODEC_BUTT) {
        return 0;
    }

    s32Ret = CVI_MAPI_AENC_Deinit(SysHandle->aenchdl);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_AENC_Deinit fail");

    return 0;
}

int32_t CVI_MEDIA_AoInit(void)
{
    CVI_MEDIA_SYSHANDLE_S *SysHandle = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_MAPI_AO_ATTR_S ao_attr;
    int32_t s32Ret = 0;

    CVI_PARAM_GetAoParam(&ao_attr);

    ao_attr.u32PowerPinId = CVI_GPIOC_25;//CVI_GPIOA_10; // Default power pin for AO
    s32Ret = CVI_MAPI_AO_Init(&SysHandle->aohdl, &ao_attr);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_AO_Init fail");

    s32Ret = CVI_SYSTEM_BootSound(SysHandle->aohdl);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_SYSTEM_BootSound fail");

    return 0;
}

int32_t CVI_MEDIA_AoDeInit(void)
{
    CVI_MEDIA_SYSHANDLE_S *SysHandle = &CVI_MEDIA_GetCtx()->SysHandle;
    int32_t s32Ret = 0;

    s32Ret = CVI_MAPI_AO_Deinit(SysHandle->aohdl);
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MAPI_AO_Deinit fail");

    return 0;
}

#ifdef SERVICES_RTSP_ON
static void rtsp_play(int32_t references, void *arg)
{
    RTSP_VENC_CTX *ctx = (RTSP_VENC_CTX *)arg;
    int32_t sns_id = ctx->param->recorder_id;
    CVI_MEDIA_PARAM_INIT_S *media_params = CVI_MEDIA_GetCtx();
    CVI_RTSP_SERVICE_HANDLE_T hdl = media_params->SysServices.RtspHdl[sns_id];
    pthread_mutex_lock(&ctx->mutex);
    int32_t vpss_grp = -1;
    int32_t chnid = ctx->param->chn_id;
  if (CVI_MAPI_VPROC_IsExtChn(ctx->param->vproc, chnid)) {
        vpss_grp = CVI_MAPI_VPROC_GetExtChnGrp(ctx->param->vproc, chnid);
        if (vpss_grp < 0) {
            CVI_LOGE("invalid group for ext chn %d", chnid);
            pthread_mutex_unlock(&ctx->mutex);
            return;
        }
        chnid = 0;
    } else {
        vpss_grp = CVI_MAPI_VPROC_GetGrp(ctx->param->vproc);
        if (vpss_grp < 0) {
            CVI_LOGE("invalid group");
            pthread_mutex_unlock(&ctx->mutex);
            return;
        }
    }
    #ifndef SERVICES_SUBVIDEO_ON
    if (references == 0 && ctx->param->venc_hdl == NULL) {
        #ifdef ENABLE_ISP_PQ_TOOL
        ctx->venc_attr.venc_param.bitrate_kbps = 20000;
        ctx->param->bitrate_kbps = ctx->venc_attr.venc_param.bitrate_kbps;
        #endif
        CVI_MAPI_VENC_InitChn(&ctx->param->venc_hdl, &ctx->venc_attr);
        if(ctx->param->venc_hdl){
            if(ctx->venc_attr.venc_param.codec == CVI_MAPI_VCODEC_H264
            || ctx->venc_attr.venc_param.codec == CVI_MAPI_VCODEC_H265){
                CVI_MAPI_VENC_SetDataFifoLen(ctx->param->venc_hdl, ctx->venc_attr.venc_param.datafifoLen);
                uint32_t datafifoLen = 0;
                CVI_MAPI_VENC_GetDataFifoLen(ctx->param->venc_hdl, &datafifoLen);
                CVI_LOGD("datafifoLen %u", datafifoLen);
            }
            CVI_MAPI_VENC_BindVproc(ctx->param->venc_hdl, vpss_grp, chnid);
            CVI_MAPI_VENC_StartRecvFrame(ctx->param->venc_hdl, -1);
            CVI_RTSP_SERVICE_UpdateParam(hdl, ctx->param);
        }
    }
    #else
    ctx->param->venc_hdl = media_params->SysHandle.venchdl[vpss_grp][chnid];
    CVI_RTSP_SERVICE_UpdateParam(hdl, ctx->param);
    #endif
    pthread_mutex_unlock(&ctx->mutex);
}

static void rtsp_teardown(int32_t references, void *arg)
{
    RTSP_VENC_CTX *ctx = (RTSP_VENC_CTX *)arg;
    int32_t sns_id = ctx->param->recorder_id;
    CVI_MEDIA_PARAM_INIT_S *media_params = CVI_MEDIA_GetCtx();
    CVI_RTSP_SERVICE_HANDLE_T rtspHdl = media_params->SysServices.RtspHdl[sns_id];
    pthread_mutex_lock(&ctx->mutex);
    #ifndef SERVICES_SUBVIDEO_ON
    int32_t vpss_grp = -1;
    int32_t chnid = ctx->param->chn_id;
    if (CVI_MAPI_VPROC_IsExtChn(ctx->param->vproc, chnid)) {
        vpss_grp = CVI_MAPI_VPROC_GetExtChnGrp(ctx->param->vproc, chnid);
        if (vpss_grp < 0) {
            CVI_LOGE("invalid group for ext chn %d", chnid);
            pthread_mutex_unlock(&ctx->mutex);
            return;
        }
        chnid = 0;
    } else {
        vpss_grp = CVI_MAPI_VPROC_GetGrp(ctx->param->vproc);
        if (vpss_grp < 0) {
            CVI_LOGE("invalid group");
            pthread_mutex_unlock(&ctx->mutex);
            return;
        }
    }
    if (references == 0 && ctx->param->venc_hdl != NULL) {
        CVI_MAPI_VENC_HANDLE_T hdl = ctx->param->venc_hdl;
        ctx->param->venc_hdl = NULL;
        CVI_RTSP_SERVICE_UpdateParam(rtspHdl, ctx->param);

        CVI_MAPI_VENC_UnBindVproc(hdl, vpss_grp, chnid);
        CVI_MAPI_VENC_StopRecvFrame(hdl);
        CVI_MAPI_VENC_DeinitChn(hdl);
    }
    #else
    ctx->param->venc_hdl = NULL;
    CVI_RTSP_SERVICE_UpdateParam(rtspHdl, ctx->param);
    #endif
    pthread_mutex_unlock(&ctx->mutex);
}

int32_t CVI_MEDIA_RtspSerInit(void)
{
    int32_t i = 0 , j = 0, z = 0;
    int32_t s32Ret = 0;
    CVI_PARAM_MEDIA_COMM_S mediacomm;
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_GetMediaComm(&mediacomm);
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_RTSP_SERVICE_HANDLE_T       *rtspHdl = &MediaParams->SysServices.RtspHdl[i];
        CVI_RTSP_SERVICE_PARAM_S        *rtspParam = &MediaParams->SysServices.RtspParams[i];
        CVI_PARAM_RTSP_CHN_ATTR_S       *rtspattr = &mediacomm.Rtsp.ChnAttrs[i];
        CVI_PARAM_MEDIA_SPEC_S params;
        CVI_PARAM_GetMediaMode(i, &params);
        if (rtspattr->Enable == true) {
            rtspParam->recorder_id = i;
            rtspParam->acap_hdl = MediaParams->SysHandle.aihdl;
            rtspParam->aenc_hdl = MediaParams->SysHandle.aenchdl;
            rtspParam->audio_codec = CVI_RTSP_SERVICE_AUDIO_CODEC_PCM;
            rtspParam->audio_sample_rate = mediacomm.Ai.enSampleRate;
            if(mediacomm.Ai.bVqeOn == 1){
                rtspParam->audio_channels = 1;
            }else{
                rtspParam->audio_channels = mediacomm.Ai.AudioChannel;
            }
            rtspParam->audio_pernum = mediacomm.Ai.u32PtNumPerFrm;
            rtspParam->max_conn = rtspattr->MaxConn;
            rtspParam->timeout = rtspattr->Timeout;
            rtspParam->port = rtspattr->Port;
            #ifdef NETPROTOCOL_CGI
                snprintf(rtspParam->rtsp_name, sizeof(rtspParam->rtsp_name), "liveRTSP/av%d", i+4);
            #else
                strncpy(rtspParam->rtsp_name, rtspattr->Name, sizeof(rtspParam->rtsp_name) - 1);
            #endif

            for (j = 0; j < MAX_VENC_CNT; j++) {
                //ENABLE
                if (params.VencAttr.VencChnAttr[j].VencChnEnable == true) {
                    if (rtspattr->Enable == true && params.VencAttr.VencChnAttr[j].VencId == rtspattr->BindVencId) {
                        rtspParam->video_codec = params.VencAttr.VencChnAttr[j].MapiVencAttr.codec;
                        rtspParam->width = params.VencAttr.VencChnAttr[j].MapiVencAttr.width;
                        rtspParam->height = params.VencAttr.VencChnAttr[j].MapiVencAttr.height;
                        rtspParam->bitrate_kbps = params.VencAttr.VencChnAttr[j].MapiVencAttr.bitrate_kbps;
                        rtspParam->chn_id = params.VencAttr.VencChnAttr[j].BindVprocChnId;
                        rtspParam->framerate = params.VencAttr.VencChnAttr[j].framerate;
                        for (z = 0; z < MAX_VPROC_CNT; z++) {
                            if ((MediaParams->SysHandle.vproc[z] != NULL) &&
                                (params.VencAttr.VencChnAttr[j].BindVprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(MediaParams->SysHandle.vproc[z]))) {
                                rtspParam->vproc = MediaParams->SysHandle.vproc[z];
                                break;
                            }
                        }
                    }
                }
            }
            rtsp_venc_ctx[i].param = rtspParam;
            pthread_mutex_init(&rtsp_venc_ctx[i].mutex, NULL);
            rtspParam->rtsp_play = rtsp_play;
            rtspParam->rtsp_play_arg = (void *)&rtsp_venc_ctx[i];
            rtspParam->rtsp_teardown = rtsp_teardown;
            rtspParam->rtsp_teardown_arg = (void *)&rtsp_venc_ctx[i];
            s32Ret = CVI_RTSP_SERVICE_Create(rtspHdl, rtspParam);
	        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_RtspSerInit fail");

        }
    }

    return 0;
}

int32_t CVI_MEDIA_RtspSerDeInit(void)
{
    int32_t s32Ret = 0;
    for (int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_RTSP_SERVICE_HANDLE_T *rtspHdl = &CVI_MEDIA_GetCtx()->SysServices.RtspHdl[i];
        if (*rtspHdl != NULL) {
            s32Ret = CVI_RTSP_SERVICE_Destroy(*rtspHdl);
            MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_RTSP_SERVICE_Destroy fail");
            *rtspHdl = NULL;
            pthread_mutex_destroy(&rtsp_venc_ctx[i].mutex);
        }
    }

    return 0;
}

int32_t CVI_MEDIA_APP_RTSP_Init(uint32_t id, char *name)
{
    int32_t s32Ret = 0;
    CVI_PARAM_MEDIA_COMM_S mediacomm;
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_GetMediaComm(&mediacomm);
    CVI_RTSP_SERVICE_HANDLE_T *rtspHdl = &MediaParams->SysServices.RtspHdl[id];
    if (*rtspHdl == NULL)
    {
        CVI_RTSP_SERVICE_PARAM_S *rtspParam = &MediaParams->SysServices.RtspParams[id];
        memcpy(rtspParam, &MediaParams->SysServices.RtspParams[0], sizeof(CVI_RTSP_SERVICE_PARAM_S));
        rtspParam->recorder_id = id;
        strncpy(rtspParam->rtsp_name, name, strlen(name));

        rtsp_venc_ctx[id] = rtsp_venc_ctx[0];
        rtsp_venc_ctx[id].param = rtspParam;
        pthread_mutex_init(&rtsp_venc_ctx[id].mutex, NULL);
        rtspParam->rtsp_play_arg = (void *)&rtsp_venc_ctx[id];
        rtspParam->rtsp_teardown_arg = (void *)&rtsp_venc_ctx[id];
        s32Ret = CVI_RTSP_SERVICE_Create(rtspHdl, rtspParam);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_APP_RTSP_Init fail");
        usleep(200*1000);
    }
    return 0;
}

int32_t CVI_MEDIA_SwitchRTSPChanel(uint32_t value, uint32_t id, char *name)
{
    CVI_RTSP_SERVICE_StartStop(0, name); // 关闭直播流
    CVI_MEDIA_PARAM_INIT_S *media_params = CVI_MEDIA_GetCtx();
    CVI_RTSP_SERVICE_HANDLE_T hdl = media_params->SysServices.RtspHdl[id];
    CVI_RTSP_SERVICE_PARAM_S *rtspParam = &media_params->SysServices.RtspParams[id];
    rtspParam->vproc = media_params->SysHandle.vproc[value]; // 切换成设定通道
    CVI_RTSP_SERVICE_UpdateParam(hdl, rtspParam);
    CVI_RTSP_SERVICE_StartStop(1, name); // 开启直播流
    return 0;
}

int32_t CVI_MEDIA_APP_RTSP_DeInit()
{
    int32_t s32Ret = 0;
    for (int32_t i = MAX_CAMERA_INSTANCES; i < MAX_RTSP_CNT; i++) {
        CVI_RTSP_SERVICE_HANDLE_T *rtspHdl = &CVI_MEDIA_GetCtx()->SysServices.RtspHdl[i];
        if (*rtspHdl != NULL) {
            s32Ret = CVI_RTSP_SERVICE_Destroy(*rtspHdl);
            MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_RTSP_SERVICE_Destroy fail");
            *rtspHdl = NULL;
            pthread_mutex_destroy(&rtsp_venc_ctx[i].mutex);
        }
    }
    return 0;
}
#endif

static int32_t get_audio_codec_type(int32_t file_type) {
    if ((file_type == CVI_RECORD_SERVICE_FILE_TYPE_TS) || (file_type == CVI_RECORD_SERVICE_FILE_TYPE_MP4)) {
        return CVI_RECORD_SERVICE_AUDIO_CODEC_AAC;
    } else if (CVI_RECORD_SERVICE_FILE_TYPE_ES == file_type) {
        return CVI_RECORD_SERVICE_AUDIO_CODEC_NONE;
    }

    return CVI_RECORD_SERVICE_AUDIO_CODEC_PCM;
}

static int32_t CVI_RECORD_GetSubtitleCallBack(void *p, int32_t viPipe, char *str, int32_t str_len)
{
    if(viPipe >= MAX_DEV_INSTANCES){
        sprintf(str,"%s","");
        return 0;
    }
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    int32_t status = 0;
    CVI_MAPI_VCAP_GetSensorPipeAttr(Syshdl->sns[viPipe], &status);
    if (0 == status) {
        //CVI_LOGD("stViPipeAttr.bYuvBypassPath is true, yuv sensor skip isp ops");
        sprintf(str,"%s","");
        return 0;
    }

    ISP_EXP_INFO_S expInfo;
    ISP_WB_INFO_S wbInfo;
    memset(&expInfo, 0, sizeof(ISP_EXP_INFO_S));
    memset(&wbInfo, 0, sizeof(ISP_WB_INFO_S));
    CVI_ISP_QueryExposureInfo(viPipe, &expInfo);
    CVI_ISP_QueryWBInfo(viPipe, &wbInfo);
    snprintf(str, str_len, "#AE ExpT:%u SExpT:%u LExpT:%u AG:%u DG:%u IG:%u Exp:%u ExpIsMax:%d AveLum:%d PIrisFno:%d Fps:%u ISO:%u #AWB RG:%d BG:%d CT:%d",
                expInfo.u32ExpTime, expInfo.u32ShortExpTime, expInfo.u32LongExpTime, expInfo.u32AGain,\
                expInfo.u32DGain, expInfo.u32ISPDGain, expInfo.u32Exposure, expInfo.bExposureIsMAX, \
                expInfo.u8AveLum,expInfo.u32PirisFNO, expInfo.u32Fps, expInfo.u32ISO,\
                wbInfo.u16Rgain, wbInfo.u16Bgain, wbInfo.u16ColorTemp);

    return 0;
}

#ifdef SERVICES_SUBVIDEO_ON
int32_t CVI_MEDIA_StartVideoInTask(void)
{
    int s32Ret = 0;
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_MEDIA_COMM_S mediacomm;
    CVI_PARAM_GetMediaComm(&mediacomm);
    int i,j;
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PARAM_MEDIA_SPEC_S params;
        memset(&params, 0x0, sizeof(CVI_PARAM_MEDIA_SPEC_S));
        CVI_PARAM_GetMediaMode(i, &params);
        CVI_PARAM_RECORD_CHN_ATTR_S     *recattr = &mediacomm.Record.ChnAttrs[i];
        for (j = 0; j < MAX_VENC_CNT; j++)
        {
            if (params.VencAttr.VencChnAttr[j].VencChnEnable == true)
            {
                if (recattr->Enable == true && recattr->Subvideoen == true && params.VencAttr.VencChnAttr[j].VencId == recattr->SubBindVencId) {
                    s32Ret |= CVI_VIDEO_SERVICR_TaskStart(i,MediaParams->SysHandle.venchdl[i][j],params.VencAttr.VencChnAttr[j].BindVprocId,params.VencAttr.VencChnAttr[j].BindVprocChnId);
                }
            }
        }

    }
    return s32Ret;
}

int32_t CVI_MEDIA_StopVideoInTask(void)
{
    int s32Ret = 0;
    CVI_PARAM_MEDIA_COMM_S mediacomm;
    CVI_PARAM_GetMediaComm(&mediacomm);
    for (int i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PARAM_RECORD_CHN_ATTR_S     *recattr = &mediacomm.Record.ChnAttrs[i];
        if (recattr->Enable == true && recattr->Subvideoen == true){
            s32Ret |= CVI_VIDEO_SERVICR_TaskStop(i);
        }
    }
    return s32Ret;
}
#endif

int32_t CVI_MEDIA_StartAudioInTask(void)
{
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    return CVI_AUDIO_SERVICR_ACAP_TaskStart(MediaParams->SysHandle.aihdl, MediaParams->SysHandle.aenchdl);
}

int32_t CVI_MEDIA_StopAudioInTask(void)
{
    return CVI_AUDIO_SERVICR_ACAP_TaskStop();
}

#ifdef CONFIG_GPS_ON
static int32_t CVI_MEDIA_GPSCallBack(CVI_GPSMNG_MSG_PACKET *msgPacket, void* privateData)
{
    (void)privateData;
    CVI_RECORD_SERVICE_GPS_INFO_S gps_info;
    if(msgPacket){
        memset(&gps_info, 0x0, sizeof(CVI_RECORD_SERVICE_GPS_INFO_S));
        gps_info.rmc_info.Hour = msgPacket->gpsRMC.utc.hour;
        gps_info.rmc_info.Minute = msgPacket->gpsRMC.utc.min;
        gps_info.rmc_info.Second = msgPacket->gpsRMC.utc.sec;
        gps_info.rmc_info.Year = msgPacket->gpsRMC.utc.year;
        gps_info.rmc_info.Month = msgPacket->gpsRMC.utc.mon;
        gps_info.rmc_info.Day = msgPacket->gpsRMC.utc.day;
        gps_info.rmc_info.Status = msgPacket->gpsRMC.status;
        gps_info.rmc_info.NSInd = msgPacket->gpsRMC.ns;
        gps_info.rmc_info.EWInd = msgPacket->gpsRMC.ew;
        gps_info.rmc_info.reserved = 'A';
        gps_info.rmc_info.Latitude = msgPacket->gpsRMC.lat;
        gps_info.rmc_info.Longitude = msgPacket->gpsRMC.lon;
        gps_info.rmc_info.Speed = msgPacket->gpsRMC.speed;
        gps_info.rmc_info.Angle = msgPacket->gpsRMC.direction;
        // gps_info.rmc_info.ID[0] = 'G';
        strcpy(gps_info.rmc_info.ID, "1A9464FF740E215FASXT");
        gps_info.rmc_info.GsensorX = 10;
        gps_info.rmc_info.GsensorY = 20;
        gps_info.rmc_info.GsensorZ = 30;

        pthread_mutex_lock(&gGPSMutex);
        memcpy(&gstGPSInfo, &gps_info, sizeof(CVI_RECORD_SERVICE_GPS_INFO_S));
        pthread_mutex_unlock(&gGPSMutex);
        return 0;
    }
    return -1;
}

static int32_t CVI_MEDIA_GetGPSInfo(CVI_RECORD_SERVICE_GPS_INFO_S *info){
    pthread_mutex_lock(&gGPSMutex);
    memcpy(info, &gstGPSInfo, sizeof(CVI_RECORD_SERVICE_GPS_INFO_S));
    pthread_mutex_unlock(&gGPSMutex);
    return 0;
}
#endif

#ifdef SERVICES_PHOTO_ON
int32_t CVI_MEDIA_PhotoSerInit(void)
{
    int32_t i = 0 , j = 0, z = 0;
    int32_t s32Ret = 0;
    CVI_PARAM_MEDIA_COMM_S mediacomm;
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_GetMediaComm(&mediacomm);
    CVI_PARAM_FILEMNG_S FileMng;
    CVI_PARAM_GetFileMngParam(&FileMng);
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PHOTO_SERVICE_HANDLE_T     *photoSerhdl = &MediaParams->SysServices.PhotoHdl[i];
        CVI_PHOTO_SERVICE_PARAM_S      *param = &MediaParams->SysServices.PhotoParams[i];
        CVI_PARAM_PHOTO_ATTR_S     *photoattr = &mediacomm.Photo;
        CVI_PARAM_THUMBNAIL_CHN_ATTR_S  *thumbnail_attr = &mediacomm.Thumbnail.ChnAttrs[i];
        uint32_t                        prealloclen = FileMng.FileMngDtcf.u32PreAllocUnit[i];
        CVI_PARAM_MEDIA_SPEC_S params;

        param->photo_id = i;
        param->cont_photo_event_cb = CVI_PHOTOMNG_ContCallBack;
        param->prealloclen = prealloclen;
        param->scale_vproc = MediaParams->SysHandle.vproc[MAX_VPROC_CNT - 1];
        param->scale_vproc_chn_id = 0;
        for (int32_t n = 0; n < MAX_CAMERA_INSTANCES; n++){
        #ifdef RESET_MODE_AHD_HOTPLUG_ON
            if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
                continue;
            }
        #endif
            CVI_PARAM_GetMediaMode(n, &params);
            for (j = 0; j < MAX_VENC_CNT; j++)
            {
                if (photoattr->ChnAttrs[i].Enable && params.VencAttr.VencChnAttr[j].VencChnEnable == true)
                {
                    if (params.VencAttr.VencChnAttr[j].VencId == photoattr->ChnAttrs[i].BindVencId) {
                        param->src_vproc_chn_id = params.VencAttr.VencChnAttr[j].BindVprocChnId;
                        param->photo_venc_hdl = MediaParams->SysHandle.venchdl[n][j];
                        param->photo_bufsize = params.VencAttr.VencChnAttr[j].MapiVencAttr.bufSize;

                        for (z = 0; z < MAX_VPROC_CNT; z++) {
                            if ((MediaParams->SysHandle.vproc[z] != NULL) ){
                                if ((params.VencAttr.VencChnAttr[j].BindVprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(MediaParams->SysHandle.vproc[z]))) {
                                    param->src_vproc = MediaParams->SysHandle.vproc[z];
                                    break;
                                }
                            }
                        }
                    }

                    if (params.VencAttr.VencChnAttr[j].VencId == thumbnail_attr->BindVencId) {
                        param->thumbnail_venc_hdl = MediaParams->SysHandle.venchdl[n][j];
                        param->vproc_chn_id_thumbnail = params.VencAttr.VencChnAttr[j].BindVprocChnId;
                        param->thumbnail_bufsize = params.VencAttr.VencChnAttr[j].MapiVencAttr.bufSize;
                        for (z = 0; z < MAX_VPROC_CNT; z++) {
                            if ((MediaParams->SysHandle.vproc[z] != NULL) &&
                                (params.VencAttr.VencChnAttr[j].BindVprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(MediaParams->SysHandle.vproc[z]))) {
                                param->thumbnail_vproc = MediaParams->SysHandle.vproc[z];
                                break;
                            }
                        }
                    }
                }
            }
        }
        s32Ret = CVI_PHOTO_SERVICE_Create(photoSerhdl, param);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_PHOTO_SERVICE_Create fail");
    }

#ifdef CONFIG_GPS_ON
    if(gstGPSInfo.init == 0){
        memset(&gstGPSInfo, 0x0, sizeof(CVI_RECORD_SERVICE_GPS_INFO_S));
        gstGPSCallback.fnGpsDataCB = CVI_MEDIA_GPSCallBack;
        gstGPSCallback.privateData = NULL;
        CVI_GPSMNG_Register(&gstGPSCallback);
        gstGPSInfo.init = 1;
    }
#endif

    return 0;
}
#endif

#ifdef SERVICES_PHOTO_ON
int32_t CVI_MEDIA_PhotoSerDeInit(void)
{
    int32_t i = 0;
    int32_t s32Ret = 0;
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_PHOTO_SERVICE_HANDLE_T *photoSerhdl = &CVI_MEDIA_GetCtx()->SysServices.PhotoHdl[i];
        if (*photoSerhdl != NULL) {
            s32Ret = CVI_PHOTO_SERVICE_Destroy(*photoSerhdl);
            MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_PHOTO_SERVICE_Destroy fail");
            *photoSerhdl = NULL;
        }
    }

#ifdef CONFIG_GPS_ON
    if(gstGPSInfo.init == 1){
        CVI_GPSMNG_UnRegister(&gstGPSCallback);
        gstGPSInfo.init = 0;
    }
#endif

    return 0;
}
#endif

#ifdef SERVICES_ADAS_ON
int32_t CVI_MEDIA_ADASInit(void)
{
    int32_t s32Ret = 0;
    s32Ret = cvi_insmod(TPU_KO_PATH, NULL);
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_ADAS_ATTR_S ADASAttr = {0};
    CVI_PARAM_GetADASConfigParam(&ADASAttr);

    for (int32_t i = 0; i < ADASAttr.adas_cnt; i++) {
        if(ADASAttr.ChnAttrs[i].Enable == false) {
            continue;
        }
        // #ifdef RESET_MODE_AHD_HOTPLUG_ON
        //     if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
        //         continue;
        //     }
        // #endif
        CVI_ADAS_SERVICE_HANDLE_T     *ADASSerhdl = &MediaParams->SysServices.ADASHdl[i];
        CVI_ADAS_SERVICE_PARAM_S      *ADASParam = &MediaParams->SysServices.ADASParams[i];
        ADASParam->camid = i;
        ADASParam->adas_voice_event_cb = CVI_ADASMNG_VoiceCallback;
        ADASParam->adas_label_event_cb = CVI_ADASMNG_LabelCallback;
        ADASParam->adas_label_osdc_event_cb = CVI_ADASMNG_LabelOSDCCallback;
        memcpy(&ADASParam->stADASModelParam, &ADASAttr.stADASModelAttr, sizeof(ADASAttr.stADASModelAttr));

        // CVI_PARAM_MEDIA_SPEC_S params;
        // CVI_PARAM_GetMediaMode(i, &params);
        // for (int32_t j = 0; j < CVI_MAPI_VPROC_MAX_CHN_NUM; j++) {
        //     if(params.VprocAttr.VprocChnAttr[j].VprocChnid == ADASAttr.ChnAttrs[i].BindVprocChnId)
        //     {
        //         ADASParam->stVPSSParam.isExtVproc = 0;
        //         break;
        //     }else if(params.VprocAttr.ExtChnAttr[j].ChnAttr.ChnId == ADASAttr.ChnAttrs[i].BindVprocChnId)
        //     {
        //         ADASParam->stVPSSParam.isExtVproc = 1;
        //         break;
        //     }else{
        //         continue;
        //     }
        // }
        ADASParam->stVPSSParam.vprocChnId = ADASAttr.ChnAttrs[i].BindVprocChnId;
        ADASParam->stVPSSParam.vprocId = i;
        for (int32_t z = 0; z < MAX_VPROC_CNT; z++) {
            if(Syshdl->vproc[z] != NULL && ADASAttr.ChnAttrs[i].BindVprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(Syshdl->vproc[z])) {
                ADASParam->stVPSSParam.vprocHandle = Syshdl->vproc[z];
                break;
            }
        }

        s32Ret = CVI_ADAS_SERVICE_Create(ADASSerhdl, ADASParam);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_ADAS_SERVICE_Create fail");
    }

    return 0;
}

int32_t CVI_MEDIA_ADASDeInit(void)
{
    CVI_PARAM_ADAS_ATTR_S ADASAttr = {0};
    CVI_PARAM_GetADASConfigParam(&ADASAttr);
    int32_t s32Ret = 0;
    for (int32_t i = 0; i < ADASAttr.adas_cnt; i++) {
        // #ifdef RESET_MODE_AHD_HOTPLUG_ON
        //     if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
        //         continue;
        //     }
        // #endif
        CVI_ADAS_SERVICE_HANDLE_T *ADASSerhdl = &CVI_MEDIA_GetCtx()->SysServices.ADASHdl[i];
        if (*ADASSerhdl != NULL) {
            s32Ret = CVI_ADAS_SERVICE_Destroy(*ADASSerhdl);
            MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_ADAS_SERVICE_Destroy fail");
            *ADASSerhdl = NULL;
        }
    }
    cvi_rmmod(TPU_KO_PATH);
    return 0;
}
#endif

#ifdef SERVICES_QRCODE_ON

int32_t CVI_MEDIA_QRCodeInit(void)
{
    int32_t s32Ret = 0;
    CVI_MEDIA_SYSHANDLE_S *Syshdl = &CVI_MEDIA_GetCtx()->SysHandle;
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_QRCODE_ATTR_S QRCODEAttr = {0};
    CVI_PARAM_GetQRCodeConfigParam(&QRCODEAttr);
    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES && i < (uint32_t)QRCODEAttr.qrcode_cnt; i++) {
        CVI_PARAM_MEDIA_SPEC_S params;
        CVI_PARAM_GetMediaMode(i, &params);
        CVI_QRCODE_SERVICE_PARAM_S *attr = &MediaParams->SysServices.QRCodeParams[i];
        if(QRCODEAttr.ChnAttrs[i].Enable == 0){
            continue;
        }

        for (int32_t j = 0; j < CVI_MAPI_VPROC_MAX_CHN_NUM; j++) {
            if(params.VprocAttr.VprocChnAttr[j].VprocChnEnable == true
            && params.VprocAttr.VprocChnAttr[j].VprocChnid == QRCODEAttr.ChnAttrs[i].BindVprocChnId)
            {
                attr->w = params.VprocAttr.VprocChnAttr[j].VpssChnAttr.u32Width;
                attr->h = params.VprocAttr.VprocChnAttr[j].VpssChnAttr.u32Height;
            }else if(params.VprocAttr.ExtChnAttr[j].ChnEnable == true
                &&  params.VprocAttr.ExtChnAttr[j].ChnAttr.ChnId == QRCODEAttr.ChnAttrs[i].BindVprocChnId)
            {
                attr->w = params.VprocAttr.ExtChnAttr[j].ChnAttr.VpssChnAttr.u32Width;
                attr->h = params.VprocAttr.ExtChnAttr[j].ChnAttr.VpssChnAttr.u32Height;
            }else{
                continue;
            }
        }
        CVI_LOGI("attr %d %d", attr->w, attr->h);
        attr->vproc_chnid = QRCODEAttr.ChnAttrs[i].BindVprocChnId;
        for (int32_t z = 0; z < MAX_VPROC_CNT; z++) {
            if(Syshdl->vproc[z] != NULL && QRCODEAttr.ChnAttrs[i].BindVprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(Syshdl->vproc[z])) {
                attr->vproc = Syshdl->vproc[z];
                break;
            }
        }
        CVI_QRCODE_SERVICE_HANDLE_T *QRCodeHdl = &CVI_MEDIA_GetCtx()->SysServices.QRCodeHdl[i];
        s32Ret = CVI_QRCode_Service_Create(QRCodeHdl, attr);
        CVI_LOGI("attr %d %d", attr->w, attr->h);
        MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_QRCode_Service_Create fail");
    }
    return 0;
}

int32_t CVI_MEDIA_QRCodeDeInit(void)
{
    CVI_PARAM_QRCODE_ATTR_S QRCODEAttr = {0};
    CVI_PARAM_GetQRCodeConfigParam(&QRCODEAttr);
    for (uint32_t i = 0; i < MAX_CAMERA_INSTANCES && i < (uint32_t)QRCODEAttr.qrcode_cnt; i++){
        if(QRCODEAttr.ChnAttrs[i].Enable == 0){
            continue;
        }

        CVI_QRCODE_SERVICE_HANDLE_T QRCodeHdl = CVI_MEDIA_GetCtx()->SysServices.QRCodeHdl[i];
        CVI_QRCode_Service_Destroy(QRCodeHdl);
        QRCodeHdl = NULL;
    }
    return 0;
}

#endif

int32_t CVI_MEDIA_RecordSerInit(void)
{
    int32_t i = 0 , j = 0, z = 0;
    int32_t s32Ret = 0;
    CVI_PARAM_MEDIA_COMM_S mediacomm;
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_GetMediaComm(&mediacomm);
    CVI_PARAM_FILEMNG_S FileMng;
    CVI_PARAM_GetFileMngParam(&FileMng);
    STG_DEVINFO_S SDParam = {0};
    CVI_PARAM_GetStgInfoParam(&SDParam);
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_RECORD_SERVICE_HANDLE_T     *recSerhdl = &MediaParams->SysServices.RecordHdl[i];
        CVI_RECORD_SERVICE_PARAM_S      *param = &MediaParams->SysServices.RecordParams[i];
        CVI_PARAM_RECORD_CHN_ATTR_S     *recattr = &mediacomm.Record.ChnAttrs[i];
        CVI_PARAM_THUMBNAIL_CHN_ATTR_S  *thumbnail_attr = &mediacomm.Thumbnail.ChnAttrs[i];
        CVI_PARAM_PIV_CHN_ATTR_S        *pivattr = &mediacomm.Piv.ChnAttrs[i];
        uint32_t                        prealloclen = FileMng.FileMngDtcf.u32PreAllocUnit[i];
        CVI_PARAM_MEDIA_SPEC_S params;

        if (recattr->Enable == true) {
            param->enable_subvideo = recattr->Subvideoen;

            param->enable_record_on_start = false;
            param->enable_perf_on_start = false;
            param->enable_debug_on_start = false;
            param->recorder_id = i;
            param->recorder_file_type = recattr->FileType;
            param->recorder_audio_codec = get_audio_codec_type(recattr->FileType);
            param->audio_recorder_enable = recattr->AudioStatus;
            param->event_recorder_pre_recording_sec = recattr->PreTime;
            param->event_recorder_post_recording_sec = recattr->PostTime;
            param->recorder_split_interval_ms = recattr->SplitTime;
            param->timelapse_recorder_framerate = recattr->TimelapseFps;
            param->timelapse_recorder_gop_interval = recattr->TimelapseGop;
            param->normal_extend_video_buffer_sec = recattr->NormalExtendVideoBufferSec;
            param->event_extend_video_buffer_sec = recattr->EventExtendVideoBufferSec;
            param->extend_other_buffer_sec = recattr->ExtendOtherBufferSec;
            strncpy(param->devmodel, recattr->devmodel, sizeof(param->devmodel) -1);
            param->short_file_ms = recattr->ShortFileMs;
            param->memory_buffer_sec = recattr->MemoryBufferSec;
            param->cont_recorder_event_cb = CVI_RECORDMNG_ContCallBack;
            param->event_recorder_event_cb = CVI_RECORDMNG_EventCallBack;
            param->timelapse_recorder_event_cb = CVI_RECORDMNG_ContCallBack;
            param->enable_subtitle = true;
            param->get_subtitle_cb = CVI_RECORD_GetSubtitleCallBack;
            param->generate_filename_cb = CVI_FILEMNG_GenerateRecordName;
            param->get_rec_dir_type_cb = CVI_FILEMNG_GetDirType;
            param->enable_thumbnail = true;
            strncpy(param->mntpath, SDParam.aszMntPath, strlen(SDParam.aszMntPath));
        #ifdef CONFIG_GPS_ON
            param->get_gps_info_cb = (void *)CVI_MEDIA_GetGPSInfo;
            param->enable_subtitle = true;
        #endif
            //todo
            if (FileMng.FileMngDtcf.preAllocFilesEnable == true) {
                if (i == 0) {
                    param->pre_alloc_unit = (FileMng.FileMngDtcf.u32PreAllocFileUnit[DTCF_DIR_NORM_FRONT] / (1024*1024));
                } else {
                    param->pre_alloc_unit = (FileMng.FileMngDtcf.u32PreAllocFileUnit[DTCF_DIR_NORM_REAR] / (1024*1024));
                }
            } else {
                param->pre_alloc_unit = recattr->PreallocUnit;
            }

            param->prealloclen = prealloclen;
            if (param->recorder_audio_codec == CVI_RECORD_SERVICE_AUDIO_CODEC_PCM) {
                CVI_MAPI_ACAP_ATTR_S attr = {0};
                CVI_PARAM_GetAiParam(&attr);

                param->audio_sample_rate = attr.enSampleRate;
                if(attr.bVqeOn == 1) {
                    param->audio_channels = 1;
                } else {
                    param->audio_channels = attr.AudioChannel;
                }
                param->audio_num_per_frame = attr.u32PtNumPerFrm;
            } else if (param->recorder_audio_codec == CVI_RECORD_SERVICE_AUDIO_CODEC_AAC) {
                CVI_MAPI_AENC_ATTR_S attr = {0};
                CVI_PARAM_GetAencParam(&attr);

                param->audio_sample_rate = attr.src_samplerate;
                param->audio_channels = attr.channels;
                param->audio_num_per_frame = attr.u32PtNumPerFrm;
            }

            for (int32_t n = 0; n < MAX_CAMERA_INSTANCES; n++){
            #ifdef RESET_MODE_AHD_HOTPLUG_ON
                if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
                    continue;
                }
            #endif
                CVI_PARAM_GetMediaMode(n, &params);

                for (j = 0; j < MAX_VENC_CNT; j++)
                {
                    if (params.VencAttr.VencChnAttr[j].VencChnEnable == true)
                    {
                        if (recattr->Enable == true && params.VencAttr.VencChnAttr[j].VencId == recattr->BindVencId) {
                            param->rec_width = params.VencAttr.VencChnAttr[j].MapiVencAttr.width;
                            param->rec_height = params.VencAttr.VencChnAttr[j].MapiVencAttr.height;
                            param->vproc_chn_id_venc = params.VencAttr.VencChnAttr[j].BindVprocChnId;
                            param->venc_bind_mode = params.VencAttr.VencChnAttr[j].bindMode;
                            param->framerate = params.VencAttr.VencChnAttr[j].framerate;
                            param->gop = params.VencAttr.VencChnAttr[j].MapiVencAttr.gop;
                            param->bitrate_kbps = params.VencAttr.VencChnAttr[j].MapiVencAttr.bitrate_kbps;
                            param->rec_venc_hdl = MediaParams->SysHandle.venchdl[n][j];
                            param->vproc_id_rec = params.VencAttr.VencChnAttr[j].BindVprocId;
                            param->recorder_video_codec = params.VencAttr.VencChnAttr[j].MapiVencAttr.codec;
                            for (z = 0; z < MAX_VPROC_CNT; z++) {
                                if ((MediaParams->SysHandle.vproc[z] != NULL) &&
                                    (params.VencAttr.VencChnAttr[j].BindVprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(MediaParams->SysHandle.vproc[z]))) {
                                    param->rec_vproc = MediaParams->SysHandle.vproc[z];
                                    break;
                                }
                            }
                        }
                        if (recattr->Enable == true && recattr->Subvideoen == true && params.VencAttr.VencChnAttr[j].VencId == recattr->SubBindVencId){
                            param->sub_rec_venc_hdl = MediaParams->SysHandle.venchdl[n][j];/*new*/
                            param->sub_vproc_chn_id_venc = params.VencAttr.VencChnAttr[j].BindVprocChnId;
                            param->sub_bitrate_kbps = params.VencAttr.VencChnAttr[j].MapiVencAttr.bitrate_kbps;
                            param->sub_rec_width = params.VencAttr.VencChnAttr[j].MapiVencAttr.width;
                            param->sub_rec_height = params.VencAttr.VencChnAttr[j].MapiVencAttr.height;
                            param->sub_framerate = params.VencAttr.VencChnAttr[j].framerate;
                            param->sub_gop = params.VencAttr.VencChnAttr[j].MapiVencAttr.gop;
                            param->sub_recorder_video_codec = params.VencAttr.VencChnAttr[j].MapiVencAttr.codec;

                            for (z = 0; z < MAX_VPROC_CNT; z++) {
                                if ((MediaParams->SysHandle.vproc[z] != NULL) &&
                                    (params.VencAttr.VencChnAttr[j].BindVprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(MediaParams->SysHandle.vproc[z]))) {
                                    param->sub_rec_vproc = MediaParams->SysHandle.vproc[z];
                                    break;
                                }
                            }
                        }
                        if (recattr->Enable == true && params.VencAttr.VencChnAttr[j].VencId == thumbnail_attr->BindVencId) {
                            param->thumbnail_venc_hdl = MediaParams->SysHandle.venchdl[n][j];
                            param->vproc_chn_id_thumbnail = params.VencAttr.VencChnAttr[j].BindVprocChnId;
                            param->thumbnail_bufsize = params.VencAttr.VencChnAttr[j].MapiVencAttr.bufSize;
                            for (z = 0; z < MAX_VPROC_CNT; z++) {
                                if ((MediaParams->SysHandle.vproc[z] != NULL) &&
                                    (params.VencAttr.VencChnAttr[j].BindVprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(MediaParams->SysHandle.vproc[z]))) {
                                    param->thumbnail_vproc = MediaParams->SysHandle.vproc[z];
                                    break;
                                }
                            }
                        }

                        //printf("###### ------ ******en %d vencid %d pivbindid %d\n", recattr->Enable, params.VencAttr.VencChnAttr[j].VencId, pivattr->BindVencId);
                        if (recattr->Enable == true && params.VencAttr.VencChnAttr[j].VencId == pivattr->BindVencId) {
                            param->piv_venc_hdl = MediaParams->SysHandle.venchdl[n][j];
                            param->piv_bufsize = params.VencAttr.VencChnAttr[j].MapiVencAttr.bufSize;
                            //printf("###### ------ ******get piv_bufsize %d from params.VencChnAttr[%d]\n", param->piv_bufsize, j);
                        }
                    }
                }
            }
            s32Ret = CVI_RECORD_SERVICE_Create(recSerhdl, param);
            MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_RECORD_SERVICE_Create fail");
        }

    }

#ifdef CONFIG_GPS_ON
    if(gstGPSInfo.init == 0){
        memset(&gstGPSInfo, 0x0, sizeof(CVI_RECORD_SERVICE_GPS_INFO_S));
        gstGPSCallback.fnGpsDataCB = CVI_MEDIA_GPSCallBack;
        gstGPSCallback.privateData = NULL;
        CVI_GPSMNG_Register(&gstGPSCallback);
        gstGPSInfo.init = 1;
    }
#endif

    return 0;
}


int32_t CVI_MEDIA_RecordSerDeInit(void)
{
    int32_t i = 0;
    int32_t s32Ret = 0;
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        CVI_RECORD_SERVICE_HANDLE_T *recSerhdl = &CVI_MEDIA_GetCtx()->SysServices.RecordHdl[i];
        if (*recSerhdl != NULL) {
            s32Ret = CVI_RECORD_SERVICE_Destroy(*recSerhdl);
            MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_RECORD_SERVICE_Destroy fail");
            *recSerhdl = NULL;
        }
    }

#ifdef CONFIG_GPS_ON
    if(gstGPSInfo.init == 1){
        CVI_GPSMNG_UnRegister(&gstGPSCallback);
        gstGPSInfo.init = 0;
    }
#endif

    return 0;
}

int32_t CVI_MEDIA_PlayBackSerInit(void)
{
#ifdef SERVICES_PLAYER_ON
    CVI_PARAM_MEDIA_COMM_S        mediacomm;
    CVI_MEDIA_PARAM_INIT_S      *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PLAYER_SERVICE_HANDLE_T *PlaySerhdl = &MediaParams->SysServices.PsHdl;
    CVI_PLAYER_SERVICE_PARAM_S  *param = &MediaParams->SysServices.PsParam;

    CVI_PARAM_GetMediaComm(&mediacomm);
    param->chn_id = 0;
    param->repeat = false;
    param->x = 0;
    param->y = 0;
    param->width = mediacomm.Vo.Width;
    param->height = mediacomm.Vo.Height;
    param->disp_rotate = mediacomm.Vo.Rotate;
    // TODO cv182x
    param->disp_fmt = 19; //12 yuv422 //13 yuv420;
    param->disp = MediaParams->SysHandle.dispHdl;
    param->ao = MediaParams->SysHandle.aohdl;
    param->SampleRate = mediacomm.Ao.enSampleRate;
    param->AudioChannel = mediacomm.Ao.AudioChannel;

    CVI_PLAYER_SERVICE_Create(PlaySerhdl, param);
#endif

    return 0;
}

int32_t CVI_MEDIA_PlayBackSerDeInit(void)
{
#ifdef SERVICES_PLAYER_ON
    CVI_MEDIA_PARAM_INIT_S      *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PLAYER_SERVICE_HANDLE_T PlaySerhdl = MediaParams->SysServices.PsHdl;

    CVI_PLAYER_SERVICE_Destroy(&PlaySerhdl);
#endif
    return 0;
}

int32_t CVI_MEDIA_SetAntiFlicker(void)
{
    CVI_PARAM_MENU_S Param = {0};
    CVI_PARAM_GetMenuParam(&Param);
    for (int32_t i = 0; i < MAX_DEV_INSTANCES; i++) {
    #ifdef RESET_MODE_AHD_HOTPLUG_ON
        if (CVI_MEDIA_Is_CameraEnabled(i) == false) {
            continue;
        }
    #endif
        uint8_t enable, frequency;
        switch (Param.Frequence.Current)
        {
        case CVI_MENU_FREQUENCY_OFF:
            enable = 0;
            frequency = 0;
            break;
        case CVI_MENU_FREQUENCY_50:
            enable = 1;
            frequency = 50;
            break;
        case CVI_MENU_FREQUENCY_60:
            enable = 1;
            frequency = 60;
            break;
        default:
            enable = 0;
            frequency = 0;
            break;
        }
        CVI_ISP_SetAntiFlicker(i, enable, frequency);
    }

    return 0;
}
