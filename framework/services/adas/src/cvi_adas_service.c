#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include "cvi_adas_service.h"
#include "cvi_log.h"
#include "cvi_vpss.h"
#include "cvi_buffer.h"
#include "cvi_vb.h"

#define ADAS_LABEL_CAR_MAX_CNT 8
#define ADAS_LABEL_LANE_MAX_CNT 2
#define VOICE_PLAY_INTERVAL 10
#define VOICE_PLAY_INTERVAL_LANE 5

typedef enum{
    CVI_ADAS_TASK_STOP = 0,
    CVI_ADAS_TASK_RUN,
    CVI_ADAS_TASK_PAUSE,
    CVI_ADAS_TASK_BUTT
}CVI_ADAS_TASK_STATE;

static CVI_ADAS_SERVICE_CTX_S gstADASCtx[MAX_CAMERA_INSTANCES];
static CVI_ADAS_SERVICE_ATTR_S gst_cvi_adas_attr[MAX_CAMERA_INSTANCES];

static uint32_t get_time_in_ms() {
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0) {
    return 0;
  }
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void adas_detect_task(void *arg)
{
    ADAS_CONTEXT_HANDLE_S ctx = (ADAS_CONTEXT_HANDLE_S) arg;
    CVI_ADAS_SERVICE_ATTR_S *pstADASAttr = (CVI_ADAS_SERVICE_ATTR_S *)ctx->attr;
    int32_t s32Ret = 0;
    VIDEO_FRAME_INFO_S src_frame;
    size_t counter = 0;
    uint32_t last_time_ms = get_time_in_ms();
    size_t last_counter = 0;
    time_t last_time_move_s = time(0), last_time_close_s = time(0), last_time_collision_s = time(0), last_time_lane_s = time(0);
    while(ctx->state != CVI_ADAS_TASK_STOP){
        if(ctx->state == CVI_ADAS_TASK_PAUSE){
            cvi_osal_task_sleep(200 * 1000);
            continue;
        }

        pthread_mutex_lock(&ctx->adas_mutex);
        counter += 1;
        s32Ret = CVI_MAPI_VPROC_GetChnFrame(pstADASAttr->stVprocAttr.vprocHandle, pstADASAttr->stVprocAttr.vprocChnId, &src_frame);
        if (s32Ret != 0) {
            CVI_LOGE("CVI_MAPI_VPROC_GetChnFrame chn0 failed with %#x\n", s32Ret);
            cvi_osal_task_sleep(100 * 1000);
            continue;
        }

        // Step 1: Object detect inference.
        int32_t frm_diff = counter - last_counter;
        if (frm_diff > 20) {
            uint32_t cur_ts_ms = get_time_in_ms();
            float fps = frm_diff * 1000.0 / (cur_ts_ms - last_time_ms);
            last_time_ms = cur_ts_ms;
            last_counter = counter;
            printf("++++++++++++ frame:%d,fps:%.2f\n", (int32_t)counter, fps);
            ctx->app_handle->adas_info->FPS = fps;
        }

#ifdef CONFIG_GSENSOR_ON
        //用于静止判断前车启动，需考虑问题：gsensor 1秒更新3次，模型帧率为11.5FPS，gsensor是否更新不及时？
        ctx->app_handle->adas_info->gsensor_data.y = 0;
        ctx->app_handle->adas_info->gsensor_data.z = 0;
        ctx->app_handle->adas_info->gsensor_data.counter += 1;
#endif

        s32Ret = CVI_TDL_APP_ADAS_Run(ctx->app_handle, &src_frame);
        if (s32Ret != CVI_SUCCESS) {
            printf("inference failed!, ret=%x\n", s32Ret);
            CVI_MAPI_ReleaseFrame(&src_frame);
            continue;
        }

        time_t now_time_s = time(0);
        cvtdl_object_t *obj_meta = &ctx->app_handle->adas_info->last_objects;
        uint32_t car_count = obj_meta->size;
#if defined(SERVICES_ADAS_LABEL_CAR_ON) || defined(SERVICES_ADAS_LABEL_OSD_ON)
        char car_coordinates[128];//128 = ADAS_LABEL_CAR_MAX_CNT * 4(x1,y1,w,h) * 4(int32_t:4 bytes)
#endif
        if ((car_count > 0)) {
            uint32_t i = 0, count = 0;

            for (i = 0; i < car_count && count < ADAS_LABEL_CAR_MAX_CNT; i++) {
#if defined(SERVICES_ADAS_LABEL_CAR_ON) || defined(SERVICES_ADAS_LABEL_OSD_ON)
                int32_t x1 = ceil(obj_meta-> info[i].bbox.x1);
                int32_t y1 = ceil(obj_meta-> info[i].bbox.y1);
                int32_t x2 = ceil(obj_meta-> info[i].bbox.x2);
                int32_t y2 = ceil(obj_meta-> info[i].bbox.y2);
                int32_t offset = count << 4;
                count++;
                *((int32_t*)(car_coordinates + 0 + offset)) = x1;
                *((int32_t*)(car_coordinates + 4 + offset)) = y1;
                *((int32_t*)(car_coordinates + 8 + offset)) = x2;
                *((int32_t*)(car_coordinates + 12 + offset)) = y2;
#endif

                switch (obj_meta-> info[i].adas_properity.state)
                {
                    case CVI_ADAS_SERVICE_CAR_MOVING:
                    {
                        if (now_time_s - last_time_move_s > VOICE_PLAY_INTERVAL) {
                            ((CVI_ADAS_SERVICE_VOICE_CALLBACK)pstADASAttr->stADASCallback.pfnVoiceCb)(CVI_ADAS_SERVICE_CAR_MOVING);
                            last_time_move_s = now_time_s;
                        }
                        break;
                    }
                    case CVI_ADAS_SERVICE_CAR_CLOSING:
                    {
                        if (now_time_s - last_time_close_s > VOICE_PLAY_INTERVAL) {
                            ((CVI_ADAS_SERVICE_VOICE_CALLBACK)pstADASAttr->stADASCallback.pfnVoiceCb)(CVI_ADAS_SERVICE_CAR_CLOSING);
                            last_time_close_s = now_time_s;
                        }
                        break;
                    }
                    case CVI_ADAS_SERVICE_CAR_COLLISION:
                    {
                        if (now_time_s - last_time_collision_s > VOICE_PLAY_INTERVAL) {
                            ((CVI_ADAS_SERVICE_VOICE_CALLBACK)pstADASAttr->stADASCallback.pfnVoiceCb)(CVI_ADAS_SERVICE_CAR_COLLISION);
                            last_time_collision_s = now_time_s;
                        }
                        break;
                    }
                    default:
                        break;
                }

            }
#ifdef SERVICES_ADAS_LABEL_CAR_ON
            ((CVI_ADAS_SERVICE_LABEL_CALLBACK)pstADASAttr->stADASCallback.pfnLabelCb)(pstADASAttr->stVprocAttr.vprocId, CVI_ADAS_SERVICE_LABEL_CAR, count, car_coordinates);
#endif
        }

        uint32_t lane_count = 0;
        cvtdl_lane_t *lane_meta = &ctx->app_handle->adas_info->lane_meta;
        lane_count = lane_meta->size;
#if defined(SERVICES_ADAS_LABEL_LANE_ON) || defined(SERVICES_ADAS_LABEL_OSD_ON)
        char lane_coordinates[32];//32 = ADAS_LABEL_LANE_MAX_CNT * 4(x1,y1,w,h) * 4(int32_t:4 bytes)
#endif
        if ((lane_count > 0)) {
#if defined(SERVICES_ADAS_LABEL_LANE_ON) || defined(SERVICES_ADAS_LABEL_OSD_ON)
            uint32_t count = 0;
            for (uint32_t i = 0; i < lane_count && i < ADAS_LABEL_LANE_MAX_CNT; i++) {
                int32_t x1 = ceil(lane_meta->lane[i].x[0]);
                int32_t y1 = ceil(lane_meta->lane[i].y[0]);
                int32_t x2 = ceil(lane_meta->lane[i].x[1]);
                int32_t y2 = ceil(lane_meta->lane[i].y[1]);
                if ((x1<=0) || (y1<=0) || (x2<=0) || (y2<=0))
                    continue;
                int32_t offset = count << 4;
                count++;
                *((int32_t*)(lane_coordinates + 0 + offset)) = x1;
                *((int32_t*)(lane_coordinates + 4 + offset)) = y1;
                *((int32_t*)(lane_coordinates + 8 + offset)) = x2;
                *((int32_t*)(lane_coordinates + 12 + offset)) = y2;
            }
            lane_count = count;
#endif
#ifdef SERVICES_ADAS_LABEL_LANE_ON
            if (count > 0)
                ((CVI_ADAS_SERVICE_LABEL_CALLBACK)pstADASAttr->stADASCallback.pfnLabelCb)(pstADASAttr->stVprocAttr.vprocId, CVI_ADAS_SERVICE_LABEL_LANE, count, lane_coordinates);
#endif
            int32_t lane_state = 0;
            lane_state = ctx->app_handle->adas_info->lane_state;
            if (lane_state) {
                if (now_time_s - last_time_lane_s > VOICE_PLAY_INTERVAL_LANE) {
                    ((CVI_ADAS_SERVICE_VOICE_CALLBACK)pstADASAttr->stADASCallback.pfnVoiceCb)(CVI_ADAS_SERVICE_CAR_LANE);
                    last_time_lane_s = now_time_s;
                }
            }
        }
#ifdef SERVICES_ADAS_LABEL_OSD_ON
        if ((lane_count + car_count) > 0) {
            ((CVI_ADAS_SERVICE_LABEL_OSDC_CALLBACK)pstADASAttr->stADASCallback.pfnLabelOsdcCb)(ctx->id, car_count, car_coordinates, lane_count, lane_coordinates);
        }
#endif
        CVI_MAPI_ReleaseFrame(&src_frame);
        pthread_mutex_unlock(&ctx->adas_mutex);
        cvi_osal_task_sleep(10 * 1000);
    }
}

static void set_sample_mot_config(cvtdl_deepsort_config_t *ds_conf) {
  ds_conf->ktracker_conf.P_beta[2] = 0.01;
  ds_conf->ktracker_conf.P_beta[6] = 1e-5;

  // ds_conf.kfilter_conf.Q_beta[2] = 0.1;
  ds_conf->kfilter_conf.Q_beta[2] = 0.01;
  ds_conf->kfilter_conf.Q_beta[6] = 1e-5;
  ds_conf->kfilter_conf.R_beta[2] = 0.1;
}

static void adas_param_2_attr(CVI_ADAS_SERVICE_PARAM_S *ADASParam, CVI_ADAS_SERVICE_ATTR_S *ADASAttr)
{
    ADASAttr->stADASCallback.pfnVoiceCb = ADASParam->adas_voice_event_cb;
    ADASAttr->stADASCallback.pfnLabelCb = ADASParam->adas_label_event_cb;
    ADASAttr->stADASCallback.pfnLabelOsdcCb = ADASParam->adas_label_osdc_event_cb;
    memcpy(&ADASAttr->stVprocAttr, &ADASParam->stVPSSParam, sizeof(CVI_ADAS_SERVICE_VPROC_ATTR_S));
    memcpy(&ADASAttr->stADASModelAttr, &ADASParam->stADASModelParam, sizeof(CVI_ADAS_SERVICE_MODEL_ATTR_S));
}

static int32_t tdl_model_init(ADAS_CONTEXT_HANDLE_S handle)
{
    int32_t s32Ret = 0;
    CVI_ADAS_SERVICE_ATTR_S *pstADASAttr = (CVI_ADAS_SERVICE_ATTR_S *)handle->attr;
    cvitdl_handle_t stTDLHandle = NULL;
    int32_t vproc_id = CVI_VPSS_GetAvailableGrp();
    if (vproc_id < 0) {
        CVI_LOGE("CVI_VPSS_GetAvailableGrp failed grpid = %d!", vproc_id);
        s32Ret = -1;
        return s32Ret;
    }

    s32Ret = CVI_TDL_CreateHandle2(&stTDLHandle, vproc_id, 0);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_TDL_CreateHandle2 failed  with ret=%#x!", s32Ret);
        return s32Ret;
    }
    handle->tdl_handle = stTDLHandle;

    {
        VB_POOL_CONFIG_S stVbPoolCfg;
        VB_POOL chnVbPool;
        CVI_U32 u32BlkSize = 0;

        u32BlkSize = COMMON_GetPicBufferSize(pstADASAttr->stADASModelAttr.width, pstADASAttr->stADASModelAttr.height, PIXEL_FORMAT_BGR_888_PLANAR,
            DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

        memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
        stVbPoolCfg.u32BlkSize	= u32BlkSize;
        stVbPoolCfg.u32BlkCnt	= 1;
        stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
        chnVbPool = CVI_VB_CreatePool(&stVbPoolCfg);
        if (chnVbPool == VB_INVALID_POOLID) {
            CVI_LOGE("CVI_VB_CreatePool failed.\n");
            goto create_service_fail;
        } else {
            GOTO_IF_FAILED(CVI_TDL_SetVBPool(stTDLHandle, 0, chnVbPool), s32Ret, create_service_fail);
        }
    }

    CVI_TDL_SetVpssTimeout(stTDLHandle, 1000);

    uint32_t buffer_size = 20;
    int8_t det_type = 1; /* 0: only object, 1: object and lane */
    cvitdl_app_handle_t app_handle = NULL;
    s32Ret |= CVI_TDL_APP_CreateHandle(&app_handle, stTDLHandle);
    s32Ret |= CVI_TDL_APP_ADAS_Init(app_handle, (uint32_t)buffer_size, det_type);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_TDL_APP_ADAS_Init failed with %#x!\n", s32Ret);
        goto create_service_fail;
    }
    // app_handle->adas_info->FPS = 25;
    app_handle->adas_info->FPS = pstADASAttr->stADASModelAttr.fps;
    app_handle->adas_info->sence_type = 0;  // 0 for car, 1 for motorbike, defult 1
    app_handle->adas_info->location_type = 1;  // camera location, 0 for left, 1 for middle, 2 for
                                             // right, defult 1, only work when sence_type=1
    handle->app_handle = app_handle;

    // setup yolo algorithm preprocess
    cvtdl_det_algo_param_t yolov8_param = CVI_TDL_GetDetectionAlgoParam(stTDLHandle, CVI_TDL_SUPPORTED_MODEL_YOLOV8_DETECTION);
    yolov8_param.cls = 7;
    printf("setup yolov8 algorithm param \n");
    GOTO_IF_FAILED(CVI_TDL_SetDetectionAlgoParam(stTDLHandle, CVI_TDL_SUPPORTED_MODEL_YOLOV8_DETECTION, yolov8_param),
    s32Ret, create_service_fail);

    GOTO_IF_FAILED(CVI_TDL_OpenModel(stTDLHandle, CVI_TDL_SUPPORTED_MODEL_YOLOV8_DETECTION, pstADASAttr->stADASModelAttr.CarModelPath),
        s32Ret, create_app_fail);

    app_handle->adas_info->lane_model_type = 1;
    CVI_TDL_SUPPORTED_MODEL_E lane_model_id;
    if (app_handle->adas_info->lane_model_type == 0) {
        lane_model_id = CVI_TDL_SUPPORTED_MODEL_LANE_DET;
    } else if (app_handle->adas_info->lane_model_type == 1) {
        lane_model_id = CVI_TDL_SUPPORTED_MODEL_LSTR;
    } else {
        printf(" err lane_model_type !\n");
        s32Ret = -1;
        goto create_app_fail;
    }
    GOTO_IF_FAILED(CVI_TDL_OpenModel(stTDLHandle, lane_model_id, pstADASAttr->stADASModelAttr.LaneModelPath), s32Ret,
        create_app_fail);

    // Init DeepSORT
    CVI_TDL_DeepSORT_Init(stTDLHandle, true);
    cvtdl_deepsort_config_t ds_conf;
    CVI_TDL_DeepSORT_GetDefaultConfig(&ds_conf);
    set_sample_mot_config(&ds_conf);
    CVI_TDL_DeepSORT_SetConfig(stTDLHandle, &ds_conf, -1, false);

    if (s32Ret != 0) {
create_app_fail:
        CVI_TDL_APP_DestroyHandle(app_handle);
create_service_fail:
        CVI_TDL_DestroyHandle(stTDLHandle);
    }

    return s32Ret;
}

static void *adas_ctx_create(int32_t camid, CVI_ADAS_SERVICE_ATTR_S *ADASAttr)
{
    memset(&gstADASCtx[camid], 0x0, sizeof(gstADASCtx[camid]));
    ADAS_CONTEXT_HANDLE_S handle = &gstADASCtx[camid];
    handle->attr = ADASAttr;
    gstADASCtx[camid].id = camid;
    gstADASCtx[camid].state = CVI_ADAS_TASK_RUN;
    pthread_mutex_init(&gstADASCtx[camid].adas_mutex, NULL);

    tdl_model_init(handle);

    cvi_osal_task_attr_t adas_ta;
    char adas_name[16] = {0};
    snprintf(adas_name, sizeof(adas_name), "ADAS_%d", camid);
    adas_ta.name = adas_name;
    adas_ta.entry = adas_detect_task;
    adas_ta.param = (void *)handle;
    adas_ta.priority = CVI_OSAL_PRI_NORMAL;
    adas_ta.detached = false;
    int32_t pc = cvi_osal_task_create(&adas_ta, &handle->adas_task);
    if (pc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("adas task create failed, %d", pc);
        return NULL;
    }

    return (void *)handle;
}

int32_t CVI_ADAS_SERVICE_Create(CVI_ADAS_SERVICE_HANDLE_T *hdl, CVI_ADAS_SERVICE_PARAM_S *ADASParam)
{
    if(ADASParam == NULL){
        CVI_LOGE("ADASParam is NULL");
        return -1;
    }

    int32_t camid= ADASParam->camid;
    CVI_ADAS_SERVICE_ATTR_S *ADASAttr = &gst_cvi_adas_attr[camid];
    adas_param_2_attr(ADASParam, ADASAttr);
    ADASAttr->ADASHdl = adas_ctx_create(camid, ADASAttr);
    *hdl = ADASAttr->ADASHdl;

    return (*hdl != NULL) ? 0 : -1;
}

static int32_t cvi_adas_get_id(CVI_ADAS_SERVICE_HANDLE_T hdl)
{
    for(int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++)
    {
        if(gst_cvi_adas_attr[i].ADASHdl == hdl)
        {
            return i;
        }
    }
    return MAX_CAMERA_INSTANCES;
}

static int32_t cvi_adas_destroy(int32_t id) {
    if(id >= MAX_CAMERA_INSTANCES){
        return 0;
    }
    ADAS_CONTEXT_HANDLE_S ctx = &gstADASCtx[id];
    ctx->state = CVI_ADAS_TASK_STOP;
    int32_t ret = cvi_osal_task_join(ctx->adas_task);
    if (ret != CVI_OSAL_SUCCESS) {
        CVI_LOGE("adas task join failed, %d", ret);
        return -1;
    }
    cvi_osal_task_destroy(&ctx->adas_task);
    pthread_mutex_destroy(&ctx->adas_mutex);
    CVI_TDL_DestroyHandle(ctx->tdl_handle);
    CVI_TDL_APP_DestroyHandle(ctx->app_handle);

    return ret;
}

int32_t CVI_ADAS_SERVICE_Destroy(CVI_ADAS_SERVICE_HANDLE_T hdl) {
    return cvi_adas_destroy(cvi_adas_get_id(hdl));
}

void CVI_ADAS_SERVICE_SetState(int32_t id, int32_t en)
{
    if(en == 1){
        gstADASCtx[id].state = CVI_ADAS_TASK_RUN;
    }else if(en == 0){
        gstADASCtx[id].state = CVI_ADAS_TASK_PAUSE;
    }
}