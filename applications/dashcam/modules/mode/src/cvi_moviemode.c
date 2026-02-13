#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_modeinner.h"
#include "cvi_param.h"
#include "cvi_media_osd.h"
#include "cvi_ledmng.h"
//#ifdef USE_GUI_AWTK
#include "ui_common.h"
//#endif

#ifdef ENABLE_ISP_IRCUT
#include "cvi_ispircut.h"
#endif

#ifdef ENABLE_VIDEO_MD
#include "cvi_videomd.h"
#endif

int32_t CVI_MODEMNG_ResetMovieMode(CVI_PARAM_CFG_S *Param)
{
    int32_t s32Ret = 0;
#ifdef SERVICES_QRCODE_ON
    CVI_MEDIA_QRCodeDeInit();
#endif

#ifdef ENABLE_ISP_IRCUT
    CVI_ISPIR_DeInit();
#endif

#if defined (ENABLE_VIDEO_MD)
    CVI_MEDIA_VIDEOMD_DeInit();
#endif

#ifdef SERVICES_ADAS_ON
    s32Ret = CVI_MEDIA_ADASDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Init CVI_MEDIA_ADASDeInit");
#endif

    s32Ret = CVI_MEDIA_LiveViewSerDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Deinit liveview ser");

#ifdef ENABLE_ISP_PQ_TOOL
    bool en = false;
    CVI_MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        s32Ret = CVI_MEDIA_RecordSerDeInit();
        MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record deinit");
    }
#else
    s32Ret = CVI_MEDIA_RecordSerDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record deinit");
#endif

#ifdef SERVICES_RTSP_ON
    s32Ret = CVI_MEDIA_RtspSerDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Deinit rtsp ser");
#endif
    s32Ret = CVI_MEDIA_StopOsd();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Stop osd");
#ifdef SERVICES_SUBVIDEO_ON
    s32Ret = CVI_MEDIA_StopVideoInTask();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"StopVideoInTask init");
#endif
    s32Ret = CVI_MEDIA_StopAudioInTask();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"StopAudioInTask deinit");

    s32Ret = CVI_MEDIA_AiDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Ai deinit");

    s32Ret = CVI_MEDIA_AencDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Aenc deinit");

    s32Ret = CVI_MEDIA_VencDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_VencDeInit fail");

    s32Ret = CVI_MEDIA_VideoDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Deinit video");

    s32Ret = CVI_MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Deinit disp");

    s32Ret = CVI_MEDIA_VbDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Deinit vb");

    s32Ret = CVI_PARAM_SetParam(Param);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Setting video param");

    s32Ret = CVI_MEDIA_VbInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Init vb");

    s32Ret = CVI_MEDIA_VideoInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Init video");

    s32Ret = CVI_MEDIA_DispInit(true);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Init disp");

    s32Ret = CVI_MEDIA_LiveViewSerInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Liveview ser init");

    s32Ret = CVI_MEDIA_VencInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Venc init");

    s32Ret = CVI_MEDIA_AiInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Ai init");

    s32Ret = CVI_MEDIA_AencInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Aenc init");

    s32Ret = CVI_MEDIA_StartAudioInTask();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"StartAudioInTask init");
#ifdef SERVICES_SUBVIDEO_ON
    s32Ret = CVI_MEDIA_StartVideoInTask();/*rtsp rec get substream thread*/
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"StartVideoInTask init");
#endif
    s32Ret = CVI_MEDIA_StartOsd();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Start osd");

#ifdef ENABLE_ISP_PQ_TOOL
    CVI_MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        s32Ret = CVI_MEDIA_RecordSerInit();
        MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record init");
    }
#else
    s32Ret = CVI_MEDIA_RecordSerInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record init");
#endif

#ifdef SERVICES_RTSP_ON
    s32Ret = CVI_MEDIA_RtspSerInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Init rtsp ser");
#endif
#ifdef SERVICES_ADAS_ON
    s32Ret = CVI_MEDIA_ADASInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Init CVI_MEDIA_ADASInit");
#endif
#if defined (ENABLE_VIDEO_MD)
    CVI_MEDIA_VIDEOMD_Init();
#endif
#ifdef ENABLE_ISP_IRCUT
    CVI_PARAM_ISPIR_ATTR_S stISPIrAttrs = {0};
    CVI_PARAM_GetISPIrConfigParam(&stISPIrAttrs);
    CVI_ISPIR_Init(&stISPIrAttrs);
#endif
#ifdef SERVICES_QRCODE_ON
    CVI_MEDIA_QRCodeInit();
#endif
    return s32Ret;
}

int32_t CVI_MODEMNG_StartEventRec(void)
{
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();
    int32_t i = 0;
    int32_t isStartEventRec = 0;
    if((pstModeMngCtx->bInEmrRec != true)) {
        for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
            if (!CVI_MEDIA_Is_CameraEnabled(i)) {
                continue;
            }

            if(SysMediaParams->SysServices.RecordParams[i].timelapse_recorder_gop_interval != 0){
                continue;
            }

            CVI_RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
            if (pstModeMngCtx->u32ModeState == CVI_MEDIA_MOVIE_STATE_VIEW) {
                CVI_RECORD_SERVICE_StartRecord(rs_hdl);
            }
        }
        for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
            if (!CVI_MEDIA_Is_CameraEnabled(i)) {
                continue;
            }

            if(SysMediaParams->SysServices.RecordParams[i].timelapse_recorder_gop_interval != 0){
                continue;
            }

            CVI_RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
            CVI_RECORD_SERVICE_EventRecord(rs_hdl);
            isStartEventRec = 1;
        }
        if(isStartEventRec == 1){
            pstModeMngCtx->u32ModeState = CVI_MEDIA_MOVIE_STATE_REC;
            pstModeMngCtx->bInEmrRec = true;
        }
    }

    return 0;
}

int32_t CVI_MODEMNG_StartRec(void)
{
    uint32_t u32ModeState = 0;
    CVI_MODEMNG_GetModeState(&u32ModeState);
    if(u32ModeState == CVI_MEDIA_MOVIE_STATE_MENU){
        return 0;
    }
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();
    int32_t i = 0;
    if(pstModeMngCtx->u32ModeState != CVI_MEDIA_MOVIE_STATE_REC
    && pstModeMngCtx->u32ModeState != CVI_MEDIA_MOVIE_STATE_LAPSE_REC) {
        for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
            if (!CVI_MEDIA_Is_CameraEnabled(i)) {
                continue;
            }

            CVI_RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];

            if(SysMediaParams->SysServices.RecordParams[i].timelapse_recorder_gop_interval == 0) {
                CVI_RECORD_SERVICE_StartRecord(rs_hdl);
                pstModeMngCtx->u32ModeState = CVI_MEDIA_MOVIE_STATE_REC;
            } else {
                CVI_RECORD_SERVICE_StartTimelapseRecord(rs_hdl);
                pstModeMngCtx->u32ModeState = CVI_MEDIA_MOVIE_STATE_LAPSE_REC;
            }
        }
    } else {
        CVI_LOGI("Movie mode state is rec %u\n", pstModeMngCtx->u32ModeState);
    }
    if (pstModeMngCtx->bInParkingRec == true) {
        pstModeMngCtx->bInParkingRec = false;
        CVI_MODEMNG_StartEventRec();
    }
#ifdef CONFIG_LED_ON
    CVI_LEDMNG_Control(true);
#endif
    return 0;
}

int32_t CVI_MODEMNG_StopRec(void)
{
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();
    int32_t i = 0;
    if ((pstModeMngCtx->u32ModeState == CVI_MEDIA_MOVIE_STATE_REC ||
        pstModeMngCtx->u32ModeState == CVI_MEDIA_MOVIE_STATE_LAPSE_REC)) {
        for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
            if (!CVI_MEDIA_Is_CameraEnabled(i)) {
                continue;
            }

            CVI_RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];

            if (SysMediaParams->SysServices.RecordParams[i].timelapse_recorder_gop_interval == 0) {
                CVI_RECORD_SERVICE_StopRecord(rs_hdl);
            } else {
                CVI_RECORD_SERVICE_StopTimelapseRecord(rs_hdl);
            }
        }
        pstModeMngCtx->u32ModeState = CVI_MEDIA_MOVIE_STATE_VIEW;
        pstModeMngCtx->bInEmrRec = false;
    }
#ifdef CONFIG_LED_ON
    CVI_LEDMNG_Control(false);
#endif
    return 0;
}

int32_t CVI_MODEMNG_StartPiv(void)
{
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    int32_t i = 0;
    int32_t s32Ret = 0;
    int32_t dir_type = 0;
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!CVI_MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        CVI_RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
        char filename[128] = {0};
        dir_type = CVI_FILEMNG_GetDirType(i, DTCF_DIR_PHOTO_FRONT);
        s32Ret = CVI_FILEMNG_GeneratePhotoName(CVI_DTCF_FILE_TYPE_JPG, dir_type, true, filename);
        if(0 == s32Ret){
            s32Ret = CVI_RECORD_SERVICE_PivCapture(rs_hdl, filename);
            CVI_LOGI("[### PIV ###] PivCapture done, ret %d, waiting for finish\n",s32Ret);
            CVI_RECORD_SERVICE_WaitPivFinish(rs_hdl);
            CVI_LOGI("[### PIV ###] PivCapture finish, ret %d\n",s32Ret);
        }
    }
    // for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
    //     if (!is_camera_enabled(i)) {
    //         continue;
    //     }

    //     CVI_RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
    //     CVI_RECORD_SERVICE_WaitPivFinish(rs_hdl);
    // }

    return s32Ret;
}


static int32_t CVI_MODEMNG_LiveViewAdjustFocus(uint32_t wndid , char* ratio)
{
#ifdef SERVICES_LIVEVIEW_ON
    CVI_PARAM_WND_ATTR_S WndParam;
    CVI_PARAM_GetWndParam(&WndParam);

    if(wndid < WndParam.WndCnt && WndParam.Wnds[wndid].WndEnable == true){
        CVI_LIVEVIEWMNG_AdjustFocus(wndid , ratio);
    }
#endif

    return 0;
}

int32_t CVI_MODEMNG_LiveViewUp(uint32_t viewwin)
{
#ifdef SERVICES_LIVEVIEW_ON
    CVI_PARAM_WND_ATTR_S WndParam;
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_GetWndParam(&WndParam);

    if(viewwin < WndParam.WndCnt && WndParam.Wnds[viewwin].WndEnable == true && WndParam.Wnds[viewwin].SmallWndEnable == false){
        CVI_LIVEVIEWMNG_MoveUp(viewwin);
        CVI_LIVEVIEW_SERVICE_GetParam(SysMediaParams->SysServices.LvHdl, viewwin, &WndParam.Wnds[viewwin]);
        CVI_PARAM_SetWndParam(&WndParam);
    }
#endif

    return 0;
}

int32_t CVI_MODEMNG_LiveViewDown(uint32_t viewwin)
{
#ifdef SERVICES_LIVEVIEW_ON
    CVI_PARAM_WND_ATTR_S WndParam;
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_GetWndParam(&WndParam);
    if(viewwin < WndParam.WndCnt && WndParam.Wnds[viewwin].WndEnable == true && WndParam.Wnds[viewwin].SmallWndEnable == false){
        CVI_LIVEVIEWMNG_MoveDown(viewwin);
        CVI_LIVEVIEW_SERVICE_GetParam(SysMediaParams->SysServices.LvHdl, viewwin, &WndParam.Wnds[viewwin]);
        CVI_PARAM_SetWndParam(&WndParam);
    }
#endif

    return 0;
}

static int32_t CVI_MODEMNG_APP_RTSP_INIT(uint32_t id, uint8_t *name)
{
#ifdef SERVICES_RTSP_ON
    CVI_MEDIA_APP_RTSP_Init(id, (char *)name);
#endif
    return 0;
}

static int32_t CVI_MODEMNG_APP_RTSP_SWITCH(uint32_t value, uint32_t id, uint8_t *name)
{
#ifdef SERVICES_RTSP_ON
    CVI_MEDIA_SwitchRTSPChanel(value, id, (char *)name);
#endif
    return 0;
}

static int32_t CVI_MODEMNG_APP_RTSP_DEINIT()
{
#ifdef SERVICES_RTSP_ON
    CVI_MEDIA_APP_RTSP_DeInit();
#endif
    return 0;
}

int32_t CVI_MODEMNG_LiveViewMirror(uint32_t CamID, bool en)
{
#ifdef SERVICES_LIVEVIEW_ON
    CVI_PARAM_WND_ATTR_S WndParam;
    CVI_PARAM_GetWndParam(&WndParam);
    uint32_t val = (CamID << 1) | (en & 0x1);
    CVI_LIVEVIEWMNG_Mirror(val);
    WndParam.Wnds[CamID].WndMirror = en;
    CVI_PARAM_SetWndParam(&WndParam);
#endif

    return 0;
}

static void CVI_MODEMNG_SetMediaVideoSize(uint32_t CamID, int32_t value)
{
    CVI_PARAM_CFG_S param;
    CVI_PARAM_GetParam(&param);
    param.CamCfg[CamID].CamMediaInfo.CurMediaMode = value;
    param.WorkModeCfg.RecordMode.CamMediaInfo[CamID].CurMediaMode = value;
    CVI_MODEMNG_ResetMovieMode(&param);

    CVI_PARAM_SetMenuParam(CamID, CVI_PARAM_MENU_VIDEO_SIZE, value);
}

static void CVI_MODEMNG_SetMediaRecEn(uint32_t value)
{
    for(int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++){
        uint32_t val = (1 << i) & value;
        val = (val > 0)?1:0;
        CVI_PARAM_SetMenuParam(i, CVI_PARAM_MENU_REC_INX, val);
    }
}

static void CVI_MODEMNG_SetGsensorSetSensitity(uint32_t CamID, int32_t value)
{
#ifdef CONFIG_GSENSOR_ON
    CVI_GSENSORMNG_MenuSetSensitity(value);
    CVI_PARAM_SetMenuParam(CamID, CVI_PARAM_MENU_GSENSOR, value);
#endif
}

static void CVI_MODEMNG_SetMediaLoopTime(int32_t value)
{
    int32_t i = 0;

#ifdef ENABLE_ISP_PQ_TOOL
    bool en = false;
    CVI_MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        CVI_MEDIA_RecordSerDeInit();
    }
#else
    CVI_MEDIA_RecordSerDeInit();
#endif

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        CVI_RECORD_SERVICE_PARAM_S *RecParam = &CVI_MEDIA_GetCtx()->SysServices.RecordParams[i];
        switch (value) {
            case CVI_MEDIA_VIDEO_LOOP_1MIN:
                RecParam->recorder_split_interval_ms = 60000; //msec
                break;
            case CVI_MEDIA_VIDEO_LOOP_3MIN:
                RecParam->recorder_split_interval_ms = 180000; //msec
                break;
            case CVI_MEDIA_VIDEO_LOOP_5MIN:
                RecParam->recorder_split_interval_ms = 300000; //msec
                break;
            default:
                CVI_LOGE("value is invalid");
                break;
        }
        CVI_PARAM_SetMenuParam(i, CVI_PARAM_MENU_VIDEO_LOOP, value);
    }

#ifdef ENABLE_ISP_PQ_TOOL
    CVI_MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        CVI_MEDIA_RecordSerInit();
    }
#else
    CVI_MEDIA_RecordSerInit();
#endif

}

void CVI_MODEMNG_SetMediaLapseTime(int32_t value)
{
    int32_t i = 0, gop = 0;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (CVI_MEDIA_Is_CameraEnabled(i) == true) {
            CVI_RECORD_SERVICE_PARAM_S *RecParam = &CVI_MEDIA_GetCtx()->SysServices.RecordParams[i];
            CVI_RECORD_SERVICE_HANDLE_T RecdHdl = CVI_MEDIA_GetCtx()->SysServices.RecordHdl[i];
            switch (value) {
                case CVI_MEDIA_VIDEO_LAPSETIME_OFF:
                    gop = 0;
                    break;
                case CVI_MEDIA_VIDEO_LAPSETIME_1S:
                    gop = 1; //sec
                    break;
                case CVI_MEDIA_VIDEO_LAPSETIME_2S:
                    gop = 2; //sec
                    break;
                case CVI_MEDIA_VIDEO_LAPSETIME_3S:
                    gop = 3; //sec
                    break;
                default:
                    CVI_LOGE("value is invalid");
                    break;
            }
            RecParam->timelapse_recorder_gop_interval = gop; //sec
            // CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();
            if(gop > 0){
                RecParam->rec_mode = 1;
                // pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_LAPSE;
            }else {
                RecParam->rec_mode = 0;
                // pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_MOVIE;
            }
            CVI_RECORD_SERVICE_UpdateParam(RecdHdl, RecParam);
            CVI_PARAM_SetMenuParam(i, CVI_PARAM_MENU_LAPSE_TIME, value);
        }
    }
}

static void CVI_MODEMNG_SetMediaAudio(int32_t value)
{
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    int32_t i = 0;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!CVI_MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        CVI_RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
    #ifdef SERVICES_RTSP_ON
        CVI_RTSP_SERVICE_HANDLE_T rtsp_hdl = SysMediaParams->SysServices.RtspHdl[i];
    #endif
        switch (value) {
            case CVI_MEDIA_VIDEO_AUDIO_OFF:
                CVI_RECORD_SERVICE_StartMute(rs_hdl);
            #ifdef SERVICES_RTSP_ON
                CVI_RTSP_SERVICE_StartMute(rtsp_hdl);
            #endif
                break;
            case CVI_MEDIA_VIDEO_AUDIO_ON:
                CVI_RECORD_SERVICE_StopMute(rs_hdl);
            #ifdef SERVICES_RTSP_ON
                CVI_RTSP_SERVICE_StopMute(rtsp_hdl);
            #endif
                break;
            default:
                CVI_LOGE("value is invalid");
                break;
        }

        CVI_PARAM_SetMenuParam(i, CVI_PARAM_MENU_AUDIO_STATUS, value);
    }
}

static void CVI_MODEMNG_SetMediaOsd(int32_t value)
{
    uint32_t i = 0, z = 0;

    CVI_PARAM_MEDIA_OSD_ATTR_S OsdParam;
    CVI_PARAM_GetOsdParam(&OsdParam);
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for(int32_t j = 0; j < OsdParam.OsdCnt; j++){
            for(z = 0; z < OsdParam.OsdAttrs[j].u32DispNum; z++){
                if (OsdParam.OsdAttrs[j].astDispAttr[z].u32Batch == i &&
                    OsdParam.OsdAttrs[j].stContent.enType == CVI_MAPI_OSD_TYPE_TIME){
                    CVI_MAPI_OSD_Show(j, z, value);
                    OsdParam.OsdAttrs[j].astDispAttr[z].bShow = value;
                }
            }
        }
    }

    CVI_PARAM_CONTEXT_S *pstParamCtx = CVI_PARAM_GetCtx();
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (uint32_t j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < CVI_PARAM_MEDIA_CNT)); j++) {
            for (int32_t k = 0; k < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdCnt; k++) {
                for(z = 0; z < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].u32DispNum; z++){
                    if (pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.enType == CVI_MAPI_OSD_TYPE_TIME){
                        pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].astDispAttr[z].bShow = value;
                    }
                }
            }
        }
    }

    CVI_PARAM_SetOsdParam(&OsdParam);
    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_OSD_STATUS, value);
}

static void CVI_MODEMNG_SetMenuSpeedStamp(int32_t value)
{
    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_SPEED_STAMP, value);
}

static void CVI_MODEMNG_SetMenuGpsStamp(int32_t value)
{
    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_GPS_STAMP, value);
}
#if CONFIG_PWM_ON
static void CVI_MODEMNG_SetMenuPwmBri(int32_t value)
{
    int32_t  pwm_level = 0, ret = 0;
    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_PWM_BRI_STATUS, value);
    switch(value) {
        case CVI_MEDIA_PWM_BRI_LOW:
            pwm_level = 10;
            break;
        case CVI_MEDIA_PWM_BRI_MID:
            pwm_level = 55;
            break;
        case CVI_MEDIA_PWM_BRI_HIGH:
            pwm_level = 95;
            break;
        default:
            break;
    }
    ret = CVI_PWM_Set_Percent(pwm_level);
    if(-1 == ret) {
        CVI_LOGD("error the rate, need input correct parm\n");
    }
    CVI_PARAM_PWM_S Param;
    ret = CVI_PARAM_GetPWMParam(&Param);
    if(ret == 0) {
        Param.PWMCfg.duty_cycle = Param.PWMCfg.period * pwm_level / 100;
        CVI_HAL_SCREEN_SetLuma(CVI_HAL_SCREEN_IDX_0, Param.PWMCfg);
    } else {
        CVI_LOGE("%s : CVI_PARAM_GetPWMParam failed\n",__func__);
    }
    ret = CVI_PARAM_SetPWMParam(&Param);
    if(ret != 0) {
        CVI_LOGE("%s : CVI_PARAM_SetPWMParam failed\n",__func__);
    }
}
#endif
static void CVI_MODEMNG_SetMenuFrequency(int32_t value)
{
    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_FREQUENCY, value);
    CVI_MEDIA_SetAntiFlicker();
}

static void CVI_MODEMNG_SetMenuTimeFormat(int32_t value)
{
    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_TIME_FORMAT, value);
}

static void CVI_MODEMNG_SetMenuCarNumStamp(int32_t value)
{
    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_CARNUM, value);
    CVI_MEDIA_UpdateCarNumOsd();
}

static void CVI_MODEMNG_SetMenuCarNumOsd(uint8_t *car_name)
{
    const uint8_t *CarNum = car_name;
    char *string_carnum_stamp = {0};
    string_carnum_stamp = (char *)CarNum;
    CVI_LOGD("in move mode :string_carnum_stamp= %s, car_name = %s\n", string_carnum_stamp, car_name);
    CVI_PARAM_SetOsdCarNameParam(string_carnum_stamp);
    CVI_MEDIA_UpdateCarNumOsd();
}

void CVI_MODEMNG_SetMenuMotionDet(int32_t value)
{
#ifdef ENABLE_VIDEO_MD
    for(int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++){
        CVI_PARAM_SetMenuParam(i, CVI_PARAM_MENU_MOTION_DET, value);
        CVI_MOTION_DETECT_SetState(i, value);
    }
#endif
}

#if 0
void CVI_MODEMNG_SetMediaVencFormat(int32_t value)
{
    int32_t i = 0;
    CVI_MAPI_VENC_CHN_ATTR_T attr[MAX_CAMERA_INSTANCES];
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    if(SysMediaParams->SysModeAttr.ModeState == CVI_MEDIA_MOVIE_STATE_REC) {
        CVI_LOGE("Movie mode is rec not set value\n");
        return;
    }
    /* reset movie mode */
    MODE_CHECK_RET_NOR(CVI_MEDIA_RecordSerDeInit());
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (CVI_MEDIA_Is_CameraEnabled(i) == true) {
            CVI_MAPI_VENC_HANDLE_T *VencHdl = &CVI_MEDIA_GetCtx()->SysHandle.venchdl[i][0];
            if (*VencHdl != NULL) {
                CVI_MAPI_VENC_GetAttr(*VencHdl, &attr[i]);
                CVI_S32 rc = CVI_MAPI_VENC_DeinitChn(*VencHdl);
                if(rc != 0) {
                    CVI_LOGE("VENC_Deinit failed\n");
                    break;
                }
            } else {
                CVI_LOGE("venc is not init! \n");
            }
        }
    }
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (CVI_MEDIA_Is_CameraEnabled(i) == true) {
            switch (value) {
                case CVI_MEDIA_VIDEO_VENCTYPE_H264:
                    attr[i].codec = CVI_MAPI_VCODEC_H264;
                    break;
                case CVI_MEDIA_VIDEO_VENCTYPE_H265:
                    attr[i].codec = CVI_MAPI_VCODEC_H265;
                    break;
                default:
                    CVI_LOGE("value is invalid");
                    break;
            }
            CVI_PARAM_SetMenuParam(i, CVI_PARAM_MENU_VIDEO_CODEC, value);
        }
    }
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (CVI_MEDIA_Is_CameraEnabled(i) == true) {
            if(CVI_MAPI_VENC_InitChn(&CVI_MEDIA_GetCtx()->SysHandle.venchdl[i][0], &attr[i]) != 0) {
                CVI_LOGE("VENC Init Channel failed!\n");
                break;
            }
        }
    }

    MODE_CHECK_RET_NOR(CVI_MEDIA_RecordSerInit());

}
#else
void CVI_MODEMNG_SetMediaVencFormat(int32_t value)
{
    int32_t i = 0;
    /* reset movie mode */
    CVI_PARAM_CFG_S param;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (CVI_MEDIA_Is_CameraEnabled(i) == true) {
            CVI_PARAM_SetMenuParam(i, CVI_PARAM_MENU_VIDEO_CODEC, value);
        }
    }
    CVI_PARAM_GetParam(&param);
    CVI_MODEMNG_ResetMovieMode(&param);
}
#endif

int32_t isStopRec(int32_t arg1) {
    switch(arg1)
    {
        case CVI_PARAM_MENU_VIDEO_SIZE:
        case CVI_PARAM_MENU_VIDEO_LOOP:
        case CVI_PARAM_MENU_GSENSOR:
        case CVI_PARAM_MENU_VIDEO_CODEC:
        case CVI_PARAM_MENU_LAPSE_TIME:
        case CVI_PARAM_MENU_LANGUAGE:
        case CVI_PARAM_MENU_PARKING:
        case CVI_PARAM_MENU_KEYTONE:
        case CVI_PARAM_MENU_OSD_STATUS:
        case CVI_PARAM_MENU_SPEED_STAMP:
        case CVI_PARAM_MENU_GPS_STAMP:
        case CVI_PARAM_MENU_PWM_BRI_STATUS:
        case CVI_PARAM_MENU_VIEW_WIN_STATUS:
        case CVI_PARAM_MENU_TIME_FORMAT:
        case CVI_PARAM_MENU_DEFAULT:
        case CVI_PARAM_MENU_FREQUENCY:
        case CVI_PARAM_MENU_REARCAM_MIRROR:
        case CVI_PARAM_MENU_SCREENDORMANT:
        case CVI_PARAM_MENU_REC_INX:
        case CVI_PARAM_MENU_CARNUM:
        case CVI_PARAM_MENU_SET_CARNUM_OSD:
            return 1;
        case CVI_PARAM_MENU_WIFI_STATUS:
        case CVI_PARAM_MENU_AUDIO_STATUS:
            return  0;
        default:
            CVI_LOGE("not support param type(%d)\n\n", arg1);
            return -1;
    }
}
static void CVI_MODEMNG_SetScreenDormant(int32_t value)
{
    uint32_t s32Ret = 0, time_sec = 0;
    CVI_TIMEDTASK_ATTR_S s_ScreenDormantAttr = {0};
    switch (value)
    {
    case CVI_MENU_SCREENDORMANT_OFF:
        time_sec = 0;
        break;
    case CVI_MENU_SCREENDORMANT_1MIN:
        time_sec = 60;
        break;
    case CVI_MENU_SCREENDORMANT_3MIN:
        time_sec = 180;
        break;
    case CVI_MENU_SCREENDORMANT_5MIN:
        time_sec = 300;
        break;
    default:
        time_sec = 0;
        break;
    }
    s_ScreenDormantAttr.u32Time_sec = time_sec;
    s_ScreenDormantAttr.bEnable = (bool)(s_ScreenDormantAttr.u32Time_sec > 0U ? true : false);
    s_ScreenDormantAttr.periodic = false;

    CVI_PWRCTRL_TASK_E enPwrCtrlType = CVI_PWRCTRL_TASK_SCREENDORMANT;
    s32Ret = CVI_POWERCTRL_SetTaskAttr(enPwrCtrlType, &s_ScreenDormantAttr);
    if (s32Ret) {
        CVI_LOGE("SetTaskAttr screen_dormant failed");
        return;
    }

    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_SCREENDORMANT, value);
}

static void CVI_MODEMNG_SetRearCamMirror(uint32_t camid, int32_t value)
{
    CVI_MODEMNG_LiveViewMirror(camid, value);
    CVI_PARAM_SetMenuParam(camid, CVI_PARAM_MENU_REARCAM_MIRROR, value);
}

static void CVI_MODEMNG_SetMenuLanguage(int32_t value)
{
    int32_t s32Ret = 0;
    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_SETTING_LANGUAGE;
    stEvent.arg1 = value;
    s32Ret = CVI_EVENTHUB_Publish(&stEvent);
    if (s32Ret != 0) {
        CVI_LOGE("Publish CVI_EVENT_MODEMNG_SETTING_LANGUAGE failed !\n");
        return;
    }
    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_LANGUAGE, value);
}

static void CVI_MODEMNG_SetMenuKeyTone(int32_t value)
{
    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_KEYTONE, value);
}

static void CVI_MODEMNG_SetMenuParking(int32_t value)
{
    CVI_PARAM_SetMenuParam(0, CVI_PARAM_MENU_PARKING, value);
}

static void CVI_MODEMNG_SetMenuWifiStatus(int32_t value)
{
#ifdef CONFIG_WIFI_ON
    CVI_PARAM_WIFI_S WifiParam = {0};
    int32_t s32Ret = 0;
    s32Ret = CVI_PARAM_GetWifiParam(&WifiParam);
    WifiParam.Enable = value;
    if (true == value) {
        s32Ret = CVI_WIFIMNG_Start(WifiParam.WifiCfg, WifiParam.WifiDefaultSsid);
    } else {
        s32Ret = CVI_WIFIMNG_Stop();
    }
    if (s32Ret != 0) {
        CVI_LOGE("CVI_MODEMNG_SetMenuWifiStatus failed !\n");
        return;
    }
    CVI_PARAM_SetWifiParam(&WifiParam);
#endif

}

int32_t CVI_MODEMNG_SetDefaultParam(void)
{
#ifdef SERVICES_LIVEVIEW_ON
    int32_t ret;
    CVI_PARAM_CFG_S param;
    CVI_PARAM_CFG_S param_stash = {0};

    // get AHD param
    CVI_PARAM_GetParam(&param);
    param_stash.MediaComm.Window.WndCnt = param.MediaComm.Window.WndCnt;
    for(uint32_t i = 0; i < param_stash.MediaComm.Window.WndCnt; i++){
        param_stash.MediaComm.Window.Wnds[i].WndEnable = param.MediaComm.Window.Wnds[i].WndEnable;
        param_stash.MediaComm.Window.Wnds[i].SmallWndEnable = param.MediaComm.Window.Wnds[i].SmallWndEnable;
    }

    // load default pararm
    ret = CVI_PARAM_LoadDefaultParamFromFlash(&param);
    MODEMNG_CHECK_RET(ret,CVI_MODE_EINVAL,"reset param");

    for(uint32_t i = 0; i < param.MediaComm.Window.WndCnt; i++){
        param.MediaComm.Window.Wnds[i].WndEnable = param_stash.MediaComm.Window.Wnds[i].WndEnable;
        param.MediaComm.Window.Wnds[i].SmallWndEnable = param_stash.MediaComm.Window.Wnds[i].SmallWndEnable;
    }

    // save param
    CVI_PARAM_SetParam(&param);
    CVI_PARAM_GetParam(&param);
    CVI_PARAM_SaveParam();

    CVI_MODEMNG_SetScreenDormant(param.Menu.ScreenDormant.Current);
#ifdef CONFIG_GSENSOR_ON
    CVI_GSENSORMNG_MenuSetSensitity(param.DevMng.Gsensor.enSensitity);
#endif

    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_SETTING_LANGUAGE;
    stEvent.arg1 = param.Menu.Language.Current;
    ret = CVI_EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(ret,CVI_MODE_EINVAL,"set ui language");

    ret = CVI_MODEMNG_ResetMovieMode(&param);
    MODEMNG_CHECK_RET(ret,CVI_MODE_EINVAL,"reset movie mode");

    CVI_LOGI("reset param finish\n");
#endif

    return 0;
}

int32_t CVI_MODEMNG_StartMemoryBufferRec(void)
{
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    int32_t i = 0;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!CVI_MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        CVI_RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
        CVI_RECORD_SERVICE_StartMemoryBuffer(rs_hdl);
    }

    return 0;
}

int32_t CVI_MODEMNG_StopMemoryBufferRec(void)
{
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    int32_t i = 0;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!CVI_MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        CVI_RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
        CVI_RECORD_SERVICE_StopMemoryBuffer(rs_hdl);
    }

    return 0;
}

static int32_t CVI_MODEMNG_RecStatesSettingMsgProc(CVI_MESSAGE_S* pstMsg)
{
    int32_t s32Ret = 0;

    switch (pstMsg->arg1)
    {
        case CVI_PARAM_MENU_VIDEO_SIZE:
            CVI_MODEMNG_SetMediaVideoSize(0, pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_VIDEO_LOOP:
            CVI_MODEMNG_SetMediaLoopTime(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_GSENSOR:
            CVI_MODEMNG_SetGsensorSetSensitity(0, pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_VIDEO_CODEC:
            CVI_MODEMNG_SetMediaVencFormat(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_LAPSE_TIME:
            CVI_MODEMNG_SetMediaLapseTime(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_WIFI_STATUS:
#ifdef CONFIG_WIFI_ON
            CVI_MODEMNG_SetMenuWifiStatus(pstMsg->arg2);
#endif
            break;

        case CVI_PARAM_MENU_AUDIO_STATUS:
            CVI_MODEMNG_SetMediaAudio(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_LANGUAGE:
            CVI_MODEMNG_SetMenuLanguage(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_PARKING:
            CVI_MODEMNG_SetMenuParking(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_KEYTONE:
            CVI_MODEMNG_SetMenuKeyTone(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_OSD_STATUS:
            CVI_MODEMNG_SetMediaOsd(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_SPEED_STAMP:
            CVI_MODEMNG_SetMenuSpeedStamp(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_GPS_STAMP:
            CVI_MODEMNG_SetMenuGpsStamp(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_PWM_BRI_STATUS:
        #if CONFIG_PWM_ON
            CVI_MODEMNG_SetMenuPwmBri(pstMsg->arg2);
        #endif
            break;

        case CVI_PARAM_MENU_VIEW_WIN_STATUS:
            CVI_MODEMNG_LiveViewSwitch(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_TIME_FORMAT:
            CVI_MODEMNG_SetMenuTimeFormat(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_DEFAULT:
            CVI_MODEMNG_SetDefaultParam();
            break;

        case CVI_PARAM_MENU_FREQUENCY:
            CVI_MODEMNG_SetMenuFrequency(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_REARCAM_MIRROR:
            CVI_MODEMNG_SetRearCamMirror(pstMsg->aszPayload[0], pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_SCREENDORMANT:
            CVI_MODEMNG_SetScreenDormant(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_REC_INX:
            CVI_MODEMNG_SetMediaRecEn(pstMsg->arg2);
            break;
        case CVI_PARAM_MENU_CARNUM: //stamp
            CVI_MODEMNG_SetMenuCarNumStamp(pstMsg->arg2);
            break;

        case CVI_PARAM_MENU_SET_CARNUM_OSD:
            CVI_MODEMNG_SetMenuCarNumOsd(pstMsg->aszPayload);
            break;
        default:
            CVI_LOGE("not support param type(%d)\n\n", pstMsg->arg1);
            return -1;
    }

    return s32Ret;
}

/** Movie Mode message process */
int32_t CVI_MODEMNG_MovieModeMsgProc(CVI_MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
    /** check parameters */
    CVI_STATE_S* pstStateAttr = (CVI_STATE_S*)pvArg;
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return CVI_PROCESS_MSG_RESULTE_OK;
    }

    CVI_LOGD("current mode(%s)\n\n", pstStateAttr->name);
    CVI_LOGD(" will process message topic(%x) \n\n", pstMsg->topic);

    switch (pstMsg->topic)
    {
        case CVI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            {
                CVI_MODEMNG_StopRec();
                // CVI_MODEMNG_StartMemoryBufferRec();
                return CVI_PROCESS_MSG_UNHANDLER;
            }
        case CVI_EVENT_MODEMNG_POWEROFF:
            {
                CVI_MODEMNG_StopRec();
                return CVI_PROCESS_MSG_UNHANDLER;
            }
        case CVI_EVENT_SENSOR_PLUG_STATUS:
        {
#ifdef SERVICES_LIVEVIEW_ON
            CVI_PARAM_CFG_S Param;
            CVI_PARAM_GetParam(&Param);
            CVI_MODEMNG_StopRec();
            int32_t snsid = pstMsg->aszPayload[1];
            int32_t mode = pstMsg->aszPayload[0];
            uint32_t lastmode = Param.WorkModeCfg.RecordMode.CamMediaInfo[snsid].CurMediaMode;
            if (CVI_SENSOR_PLUG_IN == pstMsg->arg1) {
                CVI_LOGD("sensor %d plug in\n", snsid);
                CVI_LOGD("sensor %d resolution=%d\n", snsid, mode);
                if(MAX_CAMERA_INSTANCES > 1 && MAX_DEV_INSTANCES == 1){
                    for(int i = 0;i < MAX_CAMERA_INSTANCES;i++){
                        Param.CamCfg[i].CamMediaInfo.CurMediaMode = CVI_MEDIA_Res2RecordMediaMode(mode);
                        Param.WorkModeCfg.RecordMode.CamMediaInfo[i].CurMediaMode = CVI_MEDIA_Res2RecordMediaMode(mode);
                        Param.WorkModeCfg.PhotoMode.CamMediaInfo[i].CurMediaMode = CVI_MEDIA_Res2PhotoMediaMode(mode);
                    }
                } else {
                    Param.CamCfg[snsid].CamMediaInfo.CurMediaMode = CVI_MEDIA_Res2RecordMediaMode(mode);
                    Param.WorkModeCfg.RecordMode.CamMediaInfo[snsid].CurMediaMode = CVI_MEDIA_Res2RecordMediaMode(mode);
                    Param.WorkModeCfg.PhotoMode.CamMediaInfo[snsid].CurMediaMode = CVI_MEDIA_Res2PhotoMediaMode(mode);
                }
                Param.CamCfg[snsid].CamEnable = true;
                Param.MediaComm.Window.Wnds[snsid].WndEnable = true;
                if(lastmode != Param.WorkModeCfg.RecordMode.CamMediaInfo[snsid].CurMediaMode){
                    CVI_MAPI_VCAP_SetAhdMode(snsid, mode);
                #ifndef RESET_MODE_AHD_HOTPLUG_ON
                    CVI_MODEMNG_ResetMovieMode(&Param);
                #endif
                }
            } else if (CVI_SENSOR_PLUG_OUT == pstMsg->arg1) {
                CVI_LOGD("sensor %d plug out\n", snsid);
                Param.CamCfg[snsid].CamEnable = false;
                Param.MediaComm.Window.Wnds[snsid].WndEnable = false;
            }
        #ifdef RESET_MODE_AHD_HOTPLUG_ON
            CVI_MODEMNG_ResetMovieMode(&Param);
        #endif
            CVI_PARAM_SetParam(&Param);
            CVI_MODEMNG_MonitorStatusNotify(pstMsg);
#endif
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_RECMNG_EVENTREC_END:
        case CVI_EVENT_RECMNG_EMRREC_END:
            {
                CVI_MODEMNG_SetEmrState(false);
                return CVI_PROCESS_MSG_UNHANDLER;
            }
        case CVI_EVENT_MODEMNG_START_REC:
            {
                CVI_MODEMNG_StartRec();
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_STOP_REC:
            {
                CVI_MODEMNG_StopRec();
                if (CVI_MEDIA_MOVIE_STATE_MENU == pstMsg->arg1) {
                    CVI_MODEMNG_SetModeState(CVI_MEDIA_MOVIE_STATE_MENU);
                }
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_SETTING:
            {
                if (pstModeMngCtx->u32ModeState == CVI_MEDIA_MOVIE_STATE_REC && isStopRec(pstMsg->arg1)) {
                    CVI_MODEMNG_StopRec();
                }

                CVI_MODEMNG_RecStatesSettingMsgProc(pstMsg);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_RTSP_INIT:
            {
                CVI_MODEMNG_APP_RTSP_INIT(pstMsg->arg1, pstMsg->aszPayload);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_RTSP_SWITCH:
            {
                CVI_MODEMNG_APP_RTSP_SWITCH(pstMsg->arg1, pstMsg->arg2, pstMsg->aszPayload);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_RTSP_DEINIT:
            {
                CVI_MODEMNG_APP_RTSP_DEINIT();
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_LIVEVIEW_UPORDOWN:
            {
                CVI_PARAM_MENU_S param;
                CVI_PARAM_GetMenuParam(&param);
                if (0 == pstMsg->arg1) {
                    CVI_MODEMNG_LiveViewDown(param.ViewWin.Current);
                } else {
                    CVI_MODEMNG_LiveViewUp(param.ViewWin.Current);
                }
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS:
            {
                CVI_MODEMNG_LiveViewAdjustFocus(pstMsg->arg1 , (char *)pstMsg->aszPayload);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_START_PIV:
            {
                CVI_MODEMNG_StartPiv();
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_START_EMRREC:
            {
                CVI_MODEMNG_StartEventRec();
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        default:
            return CVI_PROCESS_MSG_UNHANDLER;
            break;
    }

    return CVI_PROCESS_MSG_RESULTE_OK;
}

//MOVIE MODE
int32_t CVI_MODEMNG_OpenMovieMode(void)
{
    int32_t s32Ret = 0;
    CVI_LOGD("#########################CVI_MODEMNG_OpenMovieMode#############################\n");

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();
    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_MOVIE;

    s32Ret = CVI_MODEMNG_SetCurModeMedia(CVI_WORK_MODE_MOVIE);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Set Cur Mode Media");

    s32Ret = CVI_MEDIA_VbInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Vb init");

    s32Ret = CVI_MEDIA_VideoInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Video init");

#ifdef CONFIG_SCREEN_ON
    // screen init if not screen not init
    s32Ret = CVI_MEDIA_DispInit(false);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Disp init");

    s32Ret = CVI_MEDIA_LiveViewSerInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Liveview init");

    /*s32Ret = CVI_UIAPP_Start();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"CVI_UIAPP_Start");*/
#endif

    // s32Ret = CVI_UIAPP_Start();
    // MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"CVI_UIAPP_Start");

    s32Ret = CVI_MEDIA_VencInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Venc init");

    s32Ret = CVI_MEDIA_AiInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Ai init");

    s32Ret = CVI_MEDIA_AencInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Aenc init");

    s32Ret = CVI_MEDIA_StartAudioInTask();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"StartAudioInTask init");
#ifdef SERVICES_SUBVIDEO_ON
    s32Ret = CVI_MEDIA_StartVideoInTask();/*rtsp rec get substream thread*/
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"StartVideoInTask init");
#endif
    s32Ret = CVI_MEDIA_StartOsd();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Start osd");
#ifdef SERVICES_ADAS_ON
    s32Ret = CVI_MEDIA_ADASInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Init CVI_MEDIA_ADASInit");
#endif

#ifdef ENABLE_ISP_PQ_TOOL
    bool en = false;
    CVI_MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        s32Ret = CVI_MEDIA_RecordSerInit();
        MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record init");
    }
#else
    s32Ret = CVI_MEDIA_RecordSerInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record init");
#endif

    /** open movie ui */
    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = CVI_WORK_MODE_MOVIE;
    s32Ret = CVI_EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Publish");

#ifdef SERVICES_RTSP_ON
    s32Ret = CVI_MEDIA_RtspSerInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Rtsp init");
#endif

#ifdef ENABLE_VIDEO_MD
    CVI_MEDIA_VIDEOMD_Init();
#endif

#ifdef ENABLE_ISP_IRCUT
    CVI_PARAM_ISPIR_ATTR_S stISPIrAttrs = {0};
    CVI_PARAM_GetISPIrConfigParam(&stISPIrAttrs);
    CVI_ISPIR_Init(&stISPIrAttrs);
#endif

#ifdef SERVICES_QRCODE_ON
    CVI_MEDIA_QRCodeInit();
#endif
    // isMovieModeOpen = true;
    CVI_LOGD("############################# CVI_MODEMNG_OpenMovieMode done! #############################\n");

    return s32Ret;
}

int32_t CVI_MODEMNG_CloseMovieMode(void)
{
    int32_t s32Ret = 0;
    CVI_LOGD("#########################CVI_MODEMNG_CloseMovieMode#############################\n");
    // isMovieModeOpen = false;

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_BUTT;
    /*stop rec*/
    CVI_MODEMNG_StopRec();
#ifdef SERVICES_QRCODE_ON
    CVI_MEDIA_QRCodeDeInit();
#endif

#ifdef ENABLE_ISP_IRCUT
    CVI_ISPIR_DeInit();
#endif

#if defined (ENABLE_VIDEO_MD)
    CVI_MEDIA_VIDEOMD_DeInit();
#endif
#ifdef SERVICES_ADAS_ON
    s32Ret = CVI_MEDIA_ADASDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Init CVI_MEDIA_ADASDeInit");
#endif

    s32Ret = CVI_MEDIA_LiveViewSerDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Liveview deinit");
#ifdef ENABLE_ISP_PQ_TOOL
    bool en = false;
    CVI_MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        s32Ret = CVI_MEDIA_RecordSerDeInit();
        MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record deinit");
    }
#else
    s32Ret = CVI_MEDIA_RecordSerDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record deinit");
#endif

#ifdef SERVICES_RTSP_ON
    s32Ret = CVI_MEDIA_RtspSerDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Rtsp deinit");
#endif
    s32Ret = CVI_MEDIA_StopOsd();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Stop osd");
#ifdef SERVICES_SUBVIDEO_ON
    s32Ret = CVI_MEDIA_StopVideoInTask();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"StopVideoInTask init");
#endif
    s32Ret = CVI_MEDIA_StopAudioInTask();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"StopAudioInTask deinit");

    s32Ret = CVI_MEDIA_AiDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Ai deinit");

    s32Ret = CVI_MEDIA_AencDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Aenc deinit");

    s32Ret = CVI_MEDIA_VencDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_VencDeInit fail");

    s32Ret = CVI_MEDIA_VideoDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Video deinit");

    s32Ret = CVI_MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Disp deinit");

    s32Ret = CVI_MEDIA_VbDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Vb deinit");

    /** close movie ui */
    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODECLOSE;
    stEvent.arg1 = CVI_WORK_MODE_MOVIE;
    s32Ret = CVI_EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Publish");


    return s32Ret;
}


int32_t CVI_MODEMNG_MovieStatesInit(const CVI_STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static CVI_STATE_S stMovieState =
    {
        CVI_WORK_MODE_MOVIE,
        MODEEMNG_STATE_REC,
        CVI_MODEMNG_OpenMovieMode,
        CVI_MODEMNG_CloseMovieMode,
        CVI_MODEMNG_MovieModeMsgProc,
        NULL
    };
    stMovieState.argv = &stMovieState;
    s32Ret = CVI_HFSM_AddState(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stMovieState,
                              (CVI_STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "HFSM add NormalRec state");
    return s32Ret;
}

/** deinit movie mode */
int32_t CVI_MODEMNG_MovieStatesDeinit(void)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_CloseMovieMode();
    return s32Ret;
}
