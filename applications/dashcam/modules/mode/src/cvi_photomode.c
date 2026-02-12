#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_modeinner.h"
#include "cvi_param.h"
#include "cvi_ledmng.h"
//#ifdef USE_GUI_AWTK
#include "ui_common.h"
//#endif

#ifdef ENABLE_ISP_IRCUT
#include "cvi_ispircut.h"
#endif

int32_t CVI_MODEMNG_ResetPhotoMode(CVI_PARAM_CFG_S *Param)
{
    int32_t s32Ret = 0;

#ifdef ENABLE_ISP_IRCUT
    CVI_ISPIR_DeInit();
#endif
    s32Ret = CVI_MEDIA_LiveViewSerDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Liveview deinit");

    s32Ret = CVI_MEDIA_PhotoSerDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record deinit");

    s32Ret = CVI_MEDIA_StopOsd();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Stop osd");

    s32Ret = CVI_MEDIA_VencDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_VencDeInit fail");

    s32Ret = CVI_MEDIA_PhotoVprocDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_PhotoVprocDeInit fail");

    s32Ret = CVI_MEDIA_VideoDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Video deinit");

    s32Ret = CVI_MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Disp deinit");

    s32Ret = CVI_MEDIA_VbDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Vb deinit");

    s32Ret = CVI_PARAM_SetParam(Param);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Setting video param");

    s32Ret = CVI_MEDIA_VbInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Vb init");

    //original here CVI_MEDIA_PhotoVprocInit();
    s32Ret = CVI_MEDIA_VideoInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Video init");

    // now here
    s32Ret = CVI_MEDIA_PhotoVprocInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"CVI_MEDIA_PhotoVprocInit init");

    #ifdef CONFIG_SCREEN_ON
    // screen init if not screen not init
    s32Ret = CVI_MEDIA_DispInit(true);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Disp init");

    s32Ret = CVI_MEDIA_LiveViewSerInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Liveview init");
    #endif

    s32Ret = CVI_MEDIA_VencInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Venc init");

    s32Ret = CVI_MEDIA_StartOsd();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Start osd");

    s32Ret = CVI_MEDIA_PhotoSerInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record init");

#ifdef ENABLE_ISP_IRCUT
    CVI_PARAM_ISPIR_ATTR_S stISPIrAttrs = {0};
    CVI_PARAM_GetISPIrConfigParam(&stISPIrAttrs);
    CVI_ISPIR_Init(&stISPIrAttrs);
#endif

    return s32Ret;
}

//PHOTO MODE
int32_t CVI_MODEMNG_OpenPhotoMode(void)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();
    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_PHOTO;

    s32Ret = CVI_MODEMNG_SetCurModeMedia(CVI_WORK_MODE_PHOTO);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Set Cur Mode Media");

    s32Ret = CVI_MEDIA_VbInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Vb init");

    s32Ret = CVI_MEDIA_VideoInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Video init");

    s32Ret = CVI_MEDIA_PhotoVprocInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"CVI_MEDIA_PhotoVprocInit init");

    #ifdef CONFIG_SCREEN_ON
    // screen init if not screen not init
    s32Ret = CVI_MEDIA_DispInit(true);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Disp init");

    s32Ret = CVI_MEDIA_LiveViewSerInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Liveview init");
    #endif

    // s32Ret = CVI_UIAPP_Start();
    // MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"CVI_UIAPP_Start");

    s32Ret = CVI_MEDIA_VencInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Venc init");

    s32Ret = CVI_MEDIA_StartOsd();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Start osd");

    s32Ret = CVI_MEDIA_PhotoSerInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Photo init");

    /** open movie ui */
#ifdef ENABLE_ISP_IRCUT
    CVI_PARAM_ISPIR_ATTR_S stISPIrAttrs = {0};
    CVI_PARAM_GetISPIrConfigParam(&stISPIrAttrs);
    CVI_ISPIR_Init(&stISPIrAttrs);
#endif

    /** open photo ui */
    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = CVI_WORK_MODE_PHOTO;
    s32Ret = CVI_EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Publish");

    return s32Ret;
}

int32_t CVI_MODEMNG_ClosePhotoMode(void)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();
    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_BUTT;

#ifdef ENABLE_ISP_IRCUT
    CVI_ISPIR_DeInit();
#endif

    s32Ret = CVI_MEDIA_LiveViewSerDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Liveview deinit");

    s32Ret = CVI_MEDIA_PhotoSerDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Record deinit");

    s32Ret = CVI_MEDIA_StopOsd();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Stop osd");

    s32Ret = CVI_MEDIA_VencDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_VencDeInit fail");

    s32Ret = CVI_MEDIA_PhotoVprocDeInit();
    MEDIA_CHECK_RET(s32Ret, CVI_MEDIA_EINVAL, "CVI_MEDIA_PhotoVprocDeInit fail");

    s32Ret = CVI_MEDIA_VideoDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Video deinit");

    s32Ret = CVI_MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Disp deinit");

    s32Ret = CVI_MEDIA_VbDeInit();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Vb deinit");

    /** close photo ui */
    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODECLOSE;
    stEvent.arg1 = CVI_WORK_MODE_PHOTO;
    s32Ret = CVI_EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"Publish");

    return s32Ret;
}

int32_t CVI_MODEMNG_StartPhoto(void)
{
    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    int32_t i = 0;
    int32_t s32Ret = 0;
    int32_t dir_type = 0;
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!CVI_MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        CVI_PHOTO_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.PhotoHdl[i];
        char filename[128] = {0};
        dir_type = CVI_FILEMNG_GetDirType(i, DTCF_DIR_PHOTO_FRONT);
        s32Ret = CVI_FILEMNG_GeneratePhotoName(CVI_DTCF_FILE_TYPE_JPG, dir_type, true, filename);
        if(0 == s32Ret){
            s32Ret = CVI_PHOTO_SERVICE_PivCapture(rs_hdl, filename);
            CVI_PHOTO_SERVICE_WaitPivFinish(rs_hdl);
        }
    }

    return s32Ret;
}

static int32_t CVI_MODEMNG_AdjustFocus(uint32_t wndid , char* ratio)
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

int32_t CVI_MODEMNG_PLiveViewUp(uint32_t viewwin)
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

int32_t CVI_MODEMNG_PLiveViewDown(uint32_t viewwin)
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

void CVI_MODEMNG_PhotoSetMediaSize(int32_t value)
{
    CVI_PARAM_CFG_S param;
    CVI_PARAM_GetParam(&param);
    uint32_t PWidth = 0;
    uint32_t PHeight = 0;
    switch (value) {
        case CVI_MEDIA_PHOTO_SIZE_VGA:
            PWidth = 640;
            PHeight = 480;
            break;
        case CVI_MEDIA_PHOTO_SIZE_2M:
            PWidth = 1920;
            PHeight = 1080;
            break;
        case CVI_MEDIA_PHOTO_SIZE_5M:
            PWidth = 3072;
            PHeight = 1728;
            break;
        case CVI_MEDIA_PHOTO_SIZE_8M:
            PWidth = 3840;
            PHeight = 2160;
            break;
        case CVI_MEDIA_PHOTO_SIZE_10M:
            PWidth = 4224;
            PHeight = 2376;
            break;
        case CVI_MEDIA_PHOTO_SIZE_12M:
            PWidth = 4608;
            PHeight = 2592;
            break;
        default:
            CVI_LOGE("Value is invalid");
            break;
    }

    uint32_t i = 0, j = 0, z = 0;
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (param.CamCfg[i].CamMediaInfo.CamID == i) {
            for (j = 0; ((j < param.CamCfg[i].MediaModeCnt)&&(j < CVI_PARAM_MEDIA_CNT)); j++) {
                for (z = 0; z < MAX_VENC_CNT; z++) {
                    if (param.CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[z].VencChnEnable == true) {
                        if (param.MediaComm.Photo.ChnAttrs[i].BindVencId == param.CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[z].VencId) {
                            CVI_MAPI_VENC_CHN_PARAM_T *attr = &param.CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[z].MapiVencAttr;
                            if ((attr->width != PWidth) || (attr->height != PHeight)) {
                                param.CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[z].MapiVencAttr.width = PWidth;
                                param.CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[z].MapiVencAttr.height = PHeight;
                            }
                        }
                    }
                }
            }
        }
    }

    CVI_MODEMNG_ResetPhotoMode(&param);
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++){
        CVI_PARAM_SetMenuParam(i, CVI_PARAM_MENU_PHOTO_SIZE, value);
    }
}

static int32_t CVI_MODEMNG_PhotoStatesSettingMsgProc(CVI_MESSAGE_S* pstMsg)
{
    int32_t s32Ret = 0;

    switch (pstMsg->arg1)
    {
        case CVI_PARAM_MENU_VIDEO_SIZE:
            CVI_MODEMNG_PhotoSetMediaSize(pstMsg->arg2);
            break;

        case CVI_EVENT_MODEMNG_PHOTO_SET:
            CVI_MODEMNG_SetModeState(CVI_MEDIA_MOVIE_STATE_MENU);
            break;

        default:
            CVI_LOGE("not support param type(%d)\n\n", pstMsg->arg1);
            return -1;
    }

    return s32Ret;
}

int32_t CVI_MODEMNG_PhotoModeMsgProc(CVI_MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
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
        case CVI_EVENT_SENSOR_PLUG_STATUS:
        {
#ifdef SERVICES_LIVEVIEW_ON
            CVI_PARAM_CFG_S Param;
            CVI_PARAM_GetParam(&Param);
            int32_t snsid = pstMsg->aszPayload[1];
            int32_t mode = pstMsg->aszPayload[0];
            uint32_t lastmode = Param.WorkModeCfg.PhotoMode.CamMediaInfo[snsid].CurMediaMode;
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
                if(lastmode != Param.WorkModeCfg.PhotoMode.CamMediaInfo[snsid].CurMediaMode){
                    CVI_MAPI_VCAP_SetAhdMode(snsid, mode);
                #ifndef RESET_MODE_AHD_HOTPLUG_ON
                    CVI_MODEMNG_ResetPhotoMode(&Param);
                #endif
                }
            } else if (CVI_SENSOR_PLUG_OUT == pstMsg->arg1) {
                CVI_LOGD("sensor %d plug out\n", snsid);
                Param.CamCfg[snsid].CamEnable = false;
                Param.MediaComm.Window.Wnds[snsid].WndEnable = false;
            }
        #ifdef RESET_MODE_AHD_HOTPLUG_ON
            CVI_MODEMNG_ResetPhotoMode(&Param);
        #endif
            CVI_PARAM_SetParam(&Param);
            CVI_MODEMNG_MonitorStatusNotify(pstMsg);
#endif
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_MODEMNG_SETTING:
            {
                CVI_MODEMNG_PhotoStatesSettingMsgProc(pstMsg);
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
                CVI_MODEMNG_AdjustFocus(pstMsg->arg1 , (char *)pstMsg->aszPayload);
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        case CVI_EVENT_MODEMNG_START_PIV:
            {
                CVI_MODEMNG_StartPhoto();
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
        default:
            return CVI_PROCESS_MSG_UNHANDLER;
            break;
    }

    return CVI_PROCESS_MSG_RESULTE_OK;
}

int32_t CVI_MODEMNG_PhotoStatesInit(const CVI_STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static CVI_STATE_S stPhotoState =
    {
        CVI_WORK_MODE_PHOTO,
        MODEEMNG_STATE_PHOTO,
        CVI_MODEMNG_OpenPhotoMode,
        CVI_MODEMNG_ClosePhotoMode,
        CVI_MODEMNG_PhotoModeMsgProc,
        NULL
    };
    stPhotoState.argv = &stPhotoState;
    s32Ret = CVI_HFSM_AddState(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stPhotoState,
                              (CVI_STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "HFSM add NormalRec state");

    return s32Ret;
}

/** deinit Photo mode */
int32_t CVI_MODEMNG_PhotoStatesDeinit(void)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_ClosePhotoMode();
    return s32Ret;
}
