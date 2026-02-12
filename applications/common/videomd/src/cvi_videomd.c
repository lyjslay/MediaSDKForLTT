#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <pthread.h>
#include "cvi_mapi.h"
#include "cvi_videomd.h"
#include "cvi_appcomm.h"
#include "cvi_comm_video.h"
#include "cvi_eventhub.h"
#include "cvi_sys.h"
#include "cvi_ive.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum{
    CVI_MOTION_DETECT_STOP = 0,
    CVI_MOTION_DETECT_RUN,
    CVI_MOTION_DETECT_PAUSE,
    CVI_MOTION_DETECT_BUTT
}CVI_MOTION_DETECT_STATE;

typedef struct _CVI_MOTION_DETECT_VPROC_ATTR_S{
    CVI_MAPI_VPROC_HANDLE_T    vprocHandle;
    uint32_t                   vprocChnId;
    uint32_t                   isExtVproc;
    uint32_t                   w;
    uint32_t                   h;
}CVI_MOTION_DETECT_VPROC_ATTR_S;

typedef struct _CVI_MOTION_DETECT_IVE_CTX_S{
    IVE_HANDLE    hdl;
    IVE_IMAGE_S   srcImage0;
    IVE_IMAGE_S   srcImage1;
    IVE_IMAGE_S   dstImage;
}CVI_MOTION_DETECT_IVE_CTX_S;

typedef struct _CVI_MOTION_DETECT_CTX_S{
    int32_t        run;
    int32_t        id;
    int32_t        threshold;
    CVI_MOTION_DETECT_VPROC_ATTR_S vprocAttr;
    CVI_MOTION_DETECT_IVE_CTX_S    iveHdl;
    cvi_osal_task_handle_t      task;
    pthread_mutex_t mutex;
}CVI_MOTION_DETECT_CTX_S;

static CVI_MOTION_DETECT_CTX_S *gstMotionDetCtx[MAX_CAMERA_INSTANCES];
static pthread_mutex_t md_mutex = PTHREAD_MUTEX_INITIALIZER;


static void CVI_MOTION_DETECT_Frame2Image(VIDEO_FRAME_INFO_S *frame, IVE_IMAGE_S *image) {
    CVI_IVE_VideoFrameInfo2Image(frame, image);
}

static int32_t CVI_MOTION_DETECT_Proc(IVE_IMAGE_S *image, int32_t threshold)
{
    uint32_t len = (int32_t)(image->u32Stride[0] * image->u32Height);
    uint32_t diff = 0;

    uint32_t *q = (uint32_t *)(image->u64VirAddr[0]);
    uint32_t n = 0;
    for(uint32_t i = 0; i < len / sizeof(uint32_t); i++){
        n = (uint32_t)(q[i]);
        switch (n)
        {
            case 0x1010101: diff += 4; break;
            case 0x10101:   diff += 3; break;
            case 0x101:     diff += 2; break;
            case 0x1:       diff += 1; break;
            default:
                break;
        }
    }

    uint32_t percent = diff * 100 / len;
    if (percent >= (uint32_t)threshold) {
        return 1;
    } else {
        return 0;
    }

    return 0;
}


void CVI_MOTION_DETECT_SetState(int32_t id, int32_t en)
{
    if(gstMotionDetCtx[id] == NULL){
        return;
    }
    if(en == 1){
        gstMotionDetCtx[id]->run = CVI_MOTION_DETECT_RUN;
    }else if(en == 0){
        gstMotionDetCtx[id]->run = CVI_MOTION_DETECT_PAUSE;
    }
}

static void CVI_MOTION_DETECT_Image2Data(IVE_IMAGE_S *image, IVE_DATA_S *data)
{
    data->u32Height = image->u32Height;
    data->u32Width = image->u32Width;
    data->u32Stride = image->u32Stride[0];
    data->u64PhyAddr = image->u64PhyAddr[0];
    data->u64VirAddr = image->u64VirAddr[0];
}

static void CVI_MOTION_DETECT_Task(void *arg)
{
    CVI_MOTION_DETECT_CTX_S *ctx = (CVI_MOTION_DETECT_CTX_S *)arg;

    int32_t s32Ret = 0;
    // interval for updating background
    int32_t update_interval = 15; // Update Map Threshold

    VIDEO_FRAME_INFO_S frame;
    int32_t frameCnt = 0;
    CVI_MOTION_DETECT_IVE_CTX_S *iveCtx = &ctx->iveHdl;

    while(ctx->run != CVI_MOTION_DETECT_STOP){
        if(ctx->run == CVI_MOTION_DETECT_PAUSE){
            cvi_osal_task_sleep(200 * 1000);
            continue;
        }

        s32Ret = CVI_MAPI_VPROC_GetChnFrame(ctx->vprocAttr.vprocHandle, ctx->vprocAttr.vprocChnId, &frame);
        if (s32Ret != 0) {
            CVI_LOGE("CVI_MAPI_VPROC_GetChnFrame chn0 failed with %#x\n", s32Ret);
            cvi_osal_task_sleep(100 * 1000);
            continue;
        }

        if(frameCnt % update_interval == 0){
            IVE_IMAGE_S image;
            IVE_DATA_S src, dst;
            CVI_MOTION_DETECT_Frame2Image(&frame, &image);
            CVI_MOTION_DETECT_Image2Data(&image, &src);
            CVI_MOTION_DETECT_Image2Data(&iveCtx->srcImage0, &dst);

            pthread_mutex_lock(&md_mutex);
            IVE_DMA_CTRL_S dmactrl;
            memset(&dmactrl, 0x0, sizeof(IVE_DMA_CTRL_S));
            CVI_IVE_DMA(iveCtx->hdl, &src, &dst, &dmactrl, false);
            pthread_mutex_unlock(&md_mutex);
            CVI_MAPI_VPROC_ReleaseFrame(ctx->vprocAttr.vprocHandle, ctx->vprocAttr.vprocChnId, &frame);
            frameCnt = 1;
            cvi_osal_task_sleep(100 * 1000);
            continue;
        }

        CVI_MOTION_DETECT_Frame2Image(&frame, &iveCtx->srcImage1);

        IVE_FRAME_DIFF_MOTION_CTRL_S ctrl = {
            .enSubMode = IVE_SUB_MODE_ABS,
            .enThrMode = IVE_THRESH_MODE_BINARY,
            .u8ThrLow = 30,
            .u8ThrHigh = 0,
            .u8ThrMinVal = 0,
            .u8ThrMidVal = 0,
            .u8ThrMaxVal = 255,
            .au8ErodeMask = {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
            .au8DilateMask = {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
        };

        pthread_mutex_lock(&md_mutex);
        memset((char *)iveCtx->dstImage.u64VirAddr[0], 0x0, iveCtx->dstImage.u32Width * iveCtx->dstImage.u32Height);
        s32Ret = CVI_IVE_FrameDiffMotion(iveCtx->hdl, &iveCtx->srcImage1, &iveCtx->srcImage0, &iveCtx->dstImage, &ctrl, false);
        if(s32Ret != 0){
            CVI_MAPI_VPROC_ReleaseFrame(ctx->vprocAttr.vprocHandle, ctx->vprocAttr.vprocChnId, &frame);
            pthread_mutex_unlock(&md_mutex);
            cvi_osal_task_sleep(150 * 1000);
            continue;
        }

        unsigned char finished = 0;
        while(1){
            CVI_IVE_QUERY(iveCtx->hdl, &finished, 0);
            if(finished == true){
                break;
            }
            cvi_osal_task_sleep(20 * 1000);
        }
        pthread_mutex_unlock(&md_mutex);
        CVI_MAPI_VPROC_ReleaseFrame(ctx->vprocAttr.vprocHandle, ctx->vprocAttr.vprocChnId, &frame);

        int32_t isTrig = CVI_MOTION_DETECT_Proc(&iveCtx->dstImage, ctx->threshold);
        if (isTrig == 1) {
            CVI_EVENT_S stEvent;
            memset(&stEvent, 0x0, sizeof(stEvent));
            stEvent.topic = CVI_EVENT_VIDEOMD_CHANGE;
            stEvent.arg1 = ctx->id;
            CVI_EVENTHUB_Publish(&stEvent);
        }

        frameCnt++;
        cvi_osal_task_sleep(150 * 1000);
    }
}

void CVI_MOTION_DETECT_DeInit(int32_t id)
{
    CVI_MOTION_DETECT_CTX_S *ctx = gstMotionDetCtx[id];
    if(ctx == NULL){
        return;
    }

    ctx->run = CVI_MOTION_DETECT_STOP;
    cvi_osal_task_join(ctx->task);
    cvi_osal_task_destroy(&ctx->task);

    if(ctx->iveHdl.hdl){
        CVI_SYS_FreeI(ctx->iveHdl.hdl, &ctx->iveHdl.dstImage);
        CVI_SYS_FreeI(ctx->iveHdl.hdl, &ctx->iveHdl.srcImage0);
        CVI_IVE_DestroyHandle(ctx->iveHdl.hdl);
        ctx->iveHdl.hdl = NULL;
    }

    pthread_mutex_destroy(&ctx->mutex);
    free(ctx);
    gstMotionDetCtx[id] = NULL;
    CVI_LOGD("gstMotionDetCtx %d init", id);
}

int32_t CVI_MOTION_DETECT_Init(CVI_MOTION_DETECT_ATTR_S *attr)
{
    if(attr == NULL){
        return -1;
    }

    if(gstMotionDetCtx[attr->camid]){
        return 0;
    }

    gstMotionDetCtx[attr->camid] = (CVI_MOTION_DETECT_CTX_S *)malloc(sizeof(CVI_MOTION_DETECT_CTX_S));
    if(gstMotionDetCtx[attr->camid] == NULL){
        return -1;
    }
    memset(gstMotionDetCtx[attr->camid], 0x0, sizeof(CVI_MOTION_DETECT_CTX_S));

    gstMotionDetCtx[attr->camid]->id = attr->camid;
    gstMotionDetCtx[attr->camid]->threshold = attr->threshold;
    gstMotionDetCtx[attr->camid]->vprocAttr.vprocHandle = attr->vprocHandle;
    gstMotionDetCtx[attr->camid]->vprocAttr.vprocChnId = attr->vprocChnId;
    gstMotionDetCtx[attr->camid]->vprocAttr.isExtVproc = attr->isExtVproc;
    gstMotionDetCtx[attr->camid]->vprocAttr.w = attr->w;
    gstMotionDetCtx[attr->camid]->vprocAttr.h = attr->h;

    int32_t s32Ret = 0;
    IVE_HANDLE iveHdl = CVI_IVE_CreateHandle();
    if(iveHdl == NULL){
        free(gstMotionDetCtx[attr->camid]);
        gstMotionDetCtx[attr->camid] = NULL;
        CVI_LOGD("Motion Detect %d create failed", attr->camid);
        return -1;
    }

    s32Ret = CVI_IVE_CreateImage(iveHdl, &gstMotionDetCtx[attr->camid]->iveHdl.dstImage, IVE_IMAGE_TYPE_U8C1, attr->w, attr->h);
    if(s32Ret != 0){
        CVI_IVE_DestroyHandle(iveHdl);
        free(gstMotionDetCtx[attr->camid]);
        gstMotionDetCtx[attr->camid] = NULL;
        CVI_LOGD("Motion Detect %d CVI_IVE_CreateImage failed", attr->camid);
        return -1;
    }

    s32Ret = CVI_IVE_CreateImage(iveHdl, &gstMotionDetCtx[attr->camid]->iveHdl.srcImage0, IVE_IMAGE_TYPE_U8C1, attr->w, attr->h);
    if(s32Ret != 0){
        CVI_SYS_FreeI(iveHdl, &gstMotionDetCtx[attr->camid]->iveHdl.dstImage);
        CVI_IVE_DestroyHandle(iveHdl);
        free(gstMotionDetCtx[attr->camid]);
        gstMotionDetCtx[attr->camid] = NULL;
        CVI_LOGD("Motion Detect %d CVI_IVE_CreateImage failed", attr->camid);
        return -1;
    }

    gstMotionDetCtx[attr->camid]->iveHdl.hdl = iveHdl;

    pthread_mutex_init(&gstMotionDetCtx[attr->camid]->mutex, NULL);

    if(attr->state == 1){
        gstMotionDetCtx[attr->camid]->run = CVI_MOTION_DETECT_RUN;
    }else if(attr->state == 0){
        gstMotionDetCtx[attr->camid]->run = CVI_MOTION_DETECT_PAUSE;
    }

    cvi_osal_task_attr_t md;
    static char md_name[MAX_CAMERA_INSTANCES][16] = {0};
    snprintf(md_name[attr->camid], sizeof(md_name[attr->camid]), "motionDet_%d", attr->camid);
    md.name = md_name[attr->camid];
    md.entry = CVI_MOTION_DETECT_Task;
    md.param = (void *)gstMotionDetCtx[attr->camid];
    md.priority = CVI_OSAL_PRI_NORMAL;
    md.detached = false;
    s32Ret = cvi_osal_task_create(&md, &gstMotionDetCtx[attr->camid]->task);
    if (s32Ret != CVI_OSAL_SUCCESS) {
        CVI_SYS_FreeI(iveHdl, &gstMotionDetCtx[attr->camid]->iveHdl.dstImage);
        CVI_IVE_DestroyHandle(iveHdl);
        free(gstMotionDetCtx[attr->camid]);
        gstMotionDetCtx[attr->camid] = NULL;
        CVI_LOGE("CVI_VIDEOMD_Run task create failed, %d", attr->camid);
        return -1;
    }
    CVI_LOGD("gstMotionDetCtx %d init", attr->camid);
    return 0;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */