
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <inttypes.h>

#include "cvi_log.h"
#include "cvi_osal.h"

#include "cvi_recorder.h"
#include "cvi_muxer.h"
#include "filesync.h"
#include "cvi_rbuf.h"

#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#define CVI_CHECK_CTX_NULL(x) \
do{\
    if(!x) return -1; \
}while(0)

typedef struct cviTASK_INFO_T{
    cvi_osal_task_handle_t task;
    int32_t exitflag;
}CVI_TASK_INFO_T;


typedef struct cviRECORDER_CTX_S{
    CVI_TASK_INFO_T task[CVI_RECORDER_TYPE_BUTT_INDEX];
    CVI_MUXER_ATTR_S stMuxerAttr;
    CVI_RECORDER_ATTR_S stRecAttr;
    void *rbuf[CVI_MUXER_FRAME_TYPE_BUTT];
    volatile int64_t ptsInBuf[CVI_MUXER_FRAME_TYPE_BUTT];
    volatile int64_t ptsInLastfile[CVI_RECORDER_TYPE_BUTT_INDEX][CVI_MUXER_FRAME_TYPE_BUTT];
    volatile int64_t ptsInfile[CVI_RECORDER_TYPE_BUTT_INDEX][CVI_MUXER_FRAME_TYPE_BUTT];
    volatile int64_t ptsPerfile[CVI_RECORDER_TYPE_BUTT_INDEX][CVI_MUXER_FRAME_TYPE_BUTT];
    volatile int32_t isFirstFrame[CVI_RECORDER_TYPE_BUTT_INDEX][CVI_MUXER_FRAME_TYPE_BUTT];
    int32_t gopInx;
    volatile int32_t dropFrameFlag[CVI_MUXER_FRAME_TYPE_AUDIO];
    char filename[CVI_RECORDER_TYPE_BUTT_INDEX][128];
    char nextFilename[CVI_RECORDER_TYPE_BUTT_INDEX][128];
    int32_t fileCnt[CVI_RECORDER_TYPE_BUTT_INDEX];
    volatile int32_t recStartFlag[CVI_RECORDER_TYPE_BUTT_INDEX];
    volatile int32_t recMemStartFlag;
    void *muxer[CVI_RECORDER_TYPE_BUTT_INDEX];
    volatile int32_t recSplit[CVI_RECORDER_TYPE_BUTT_INDEX];
    int32_t postStopFlag;
    volatile int32_t manualStopFlag;
    volatile int32_t fillFlag;
    void *packet[CVI_RECORDER_TYPE_BUTT_INDEX][CVI_MUXER_FRAME_TYPE_BUTT];
    int32_t packetSize[CVI_RECORDER_TYPE_BUTT_INDEX][CVI_MUXER_FRAME_TYPE_BUTT];
    uint64_t firstFramePts[CVI_MUXER_FRAME_TYPE_BUTT];
    uint64_t curFramePts[CVI_RECORDER_TYPE_BUTT_INDEX][CVI_MUXER_FRAME_TYPE_BUTT];
    uint64_t targetFramePts[CVI_RECORDER_TYPE_BUTT_INDEX][CVI_MUXER_FRAME_TYPE_BUTT];
    double fduration[CVI_MUXER_FRAME_TYPE_BUTT];
    uint64_t recStartTime[CVI_RECORDER_TYPE_BUTT_INDEX];
    pthread_mutex_t mutex[CVI_RECORDER_TYPE_BUTT_INDEX];
    int64_t splitTimeOut[CVI_RECORDER_TYPE_BUTT_INDEX];
    uint64_t timelapsePts[CVI_MUXER_VIDEO_TRACK_IDX_BUTT];
    uint64_t timelapseIntervalNs;
    int32_t timelapseLessCnt[CVI_MUXER_VIDEO_TRACK_IDX_BUTT];
    int32_t reqIdrFlag;
    uint64_t reqIdrPts;
}CVI_RECORDER_CTX_S;

#define CVI_REC_ALIGN(s, n) (((s) + (n) - 1) & (~((n) - 1)))
#define CVI_REC_ALIGN_LEN (8)
#define CVI_REC_FRAME_COMPENSATION (0.8)
#define RECORDER_PRESTORE_TIME 20 * 1000
#define SUPPORT_POST_STOP 1

static uint64_t g_RecStartTime = 0;
static float g_rec0Fps = 0.0;

static int32_t recorder_Event_Callback(CVI_MUXER_EVENT_E event_type, const char *filename, void *p, void *extend)
{
    CVI_RECORDER_ATTR_S *attr = (CVI_RECORDER_ATTR_S *)p;
    CVI_RECORDER_EVENT_CALLBACK pfn_callback = NULL;
    void *param = NULL;
    if(attr->enRecType == CVI_RECORDER_TYPE_LAPSE) {
        pfn_callback = attr->fncallback.pfn_event_cb[CVI_RECORDER_TYPE_LAPSE_INDEX];
    }else {
        pfn_callback = attr->fncallback.pfn_event_cb[CVI_RECORDER_TYPE_NORMAL_INDEX];
    }

    if (!pfn_callback) {
        return 0;
    }

    param = attr->fncallback.pfn_event_cb_param;

    if (event_type == CVI_MUXER_SEND_FRAME_FAILED) {
        pfn_callback(CVI_RECORDER_EVENT_WRITE_FRAME_FAILED, filename, param);
    } else if (event_type == CVI_MUXER_SEND_FRAME_TIMEOUT) {
        CVI_RECORDER_EVENT_WRITE_FRAME_TIMEOUT_S timeout = {
            .timeout_ms = *(int32_t *)extend,
            .param = param
        };
        pfn_callback(CVI_RECORDER_EVENT_WRITE_FRAME_TIMEOUT, filename, &timeout);
    } else if (event_type == CVI_MUXER_PTS_JUMP) {
        pfn_callback(CVI_RECORDER_EVENT_WRITE_FRAME_DROP, filename, param);
    } else if (event_type == CVI_MUXER_STOP) {
        pfn_callback(CVI_RECORDER_EVENT_SYNC_DONE, filename, param);
    } else if (event_type == CVI_MUXER_OPEN_FILE_FAILED) {
        pfn_callback(CVI_RECORDER_EVENT_OPEN_FILE_FAILED, filename, param);
    }
    return 0;
}

uint64_t CVI_RECORDER_GetUs(void)
{
    struct timespec tv;
    // CLOCK_THREAD_CPUTIME_ID CLOCK_BOOTTIME CLOCK_MONOTONIC
    clock_gettime(CLOCK_BOOTTIME, &tv);
    uint64_t s = tv.tv_sec;
    return s * 1000 * 1000 + tv.tv_nsec / 1000;
}

static uint64_t CVI_RECORDER_GetNs(void)
{
    struct timespec tv;
    // CLOCK_THREAD_CPUTIME_ID CLOCK_BOOTTIME CLOCK_MONOTONIC
    clock_gettime(CLOCK_BOOTTIME, &tv);
    uint64_t s = tv.tv_sec;
    return s * 1000 * 1000 * 1000 + tv.tv_nsec;
}
static int32_t recorder_is_ShortFile(void *recorder, CVI_RECORDER_TYPE_INDEX_E type)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    if(ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_VIDEO] * ctx->fduration[CVI_MUXER_FRAME_TYPE_VIDEO] <= ctx->stRecAttr.short_file_ms * 1000) {
        return 1;
    }
    return 0;
}

static int32_t recorder_AudioEn(CVI_RECORDER_ATTR_S attr)
{
    return attr.astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_AUDIO].enable;
}

static int32_t recorder_SubVideoEn(CVI_RECORDER_ATTR_S attr)
{
    return attr.astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO].enable;
}

static int32_t recorder_SubtitleEn(CVI_RECORDER_ATTR_S attr)
{
    return attr.enable_subtitle;
}

static void recorder_ReqIdr(CVI_RECORDER_CTX_S *ctx, CVI_MUXER_FRAME_TYPE_E type)
{
    if(ctx->reqIdrFlag == 0){
        if(ctx->stRecAttr.fncallback.pfn_request_idr){
            ctx->stRecAttr.fncallback.pfn_request_idr(ctx->stRecAttr.fncallback.pfn_request_idr_param, type);
            ctx->reqIdrFlag = 1;
        }
    }
}
static int32_t recorder_is_Split(CVI_RECORDER_CTX_S *ctx, CVI_RECORDER_TYPE_INDEX_E type)
{
    if(type == CVI_RECORDER_TYPE_EVENT_INDEX || ctx->stRecAttr.enRecType == CVI_RECORDER_TYPE_LAPSE){
        if(ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_VIDEO] >= ctx->ptsPerfile[type][CVI_MUXER_FRAME_TYPE_VIDEO]
        && ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_SUB_VIDEO] >= ctx->ptsPerfile[type][CVI_MUXER_FRAME_TYPE_SUB_VIDEO]
        && ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_AUDIO] >= ctx->ptsPerfile[type][CVI_MUXER_FRAME_TYPE_AUDIO]
        && ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_SUBTITLE] >= ctx->ptsPerfile[type][CVI_MUXER_FRAME_TYPE_SUBTITLE]){
            return 1;
        }
        return 0;
    }

    int32_t isSplit = 0;
    uint64_t cur = CVI_RECORDER_GetUs();
    uint64_t u64SplitTimeLenUSec = ctx->stRecAttr.stSplitAttr.u64SplitTimeLenMSec * 1000;
    uint64_t end = (ctx->fileCnt[type] + 1) * u64SplitTimeLenUSec + g_RecStartTime;

    if(ctx->targetFramePts[type][CVI_MUXER_FRAME_TYPE_VIDEO] == 0){
        isSplit = 0;
    }else if(recorder_AudioEn(ctx->stRecAttr) && ctx->targetFramePts[type][CVI_MUXER_FRAME_TYPE_AUDIO] == 0){
        isSplit = 0;
    }else{
        uint64_t videoT = ctx->targetFramePts[type][CVI_MUXER_FRAME_TYPE_VIDEO] - ctx->fduration[CVI_MUXER_FRAME_TYPE_VIDEO];
        uint64_t audioT = ctx->targetFramePts[type][CVI_MUXER_FRAME_TYPE_AUDIO] - ctx->fduration[CVI_MUXER_FRAME_TYPE_AUDIO];
        if(ctx->curFramePts[type][CVI_MUXER_FRAME_TYPE_VIDEO] >= videoT
        && ctx->curFramePts[type][CVI_MUXER_FRAME_TYPE_AUDIO] >= audioT){
            CVI_LOGI("[%d.%d] %"PRIu64" %"PRIu64"", ctx->stRecAttr.id, type,
                ctx->curFramePts[type][CVI_MUXER_FRAME_TYPE_VIDEO], ctx->targetFramePts[type][CVI_MUXER_FRAME_TYPE_VIDEO]);
            if(ctx->stRecAttr.enRecType != CVI_RECORDER_TYPE_LAPSE){
                CVI_LOGI("[%d.%d] %"PRIu64" %"PRIu64"", ctx->stRecAttr.id, type,
                    ctx->curFramePts[type][CVI_MUXER_FRAME_TYPE_AUDIO], ctx->targetFramePts[type][CVI_MUXER_FRAME_TYPE_AUDIO]);
            }
            isSplit = 1;
        }
    }

    if(cur >= end - ctx->splitTimeOut[type]){
        CVI_LOGI("[%d.%d] %"PRIu64" %"PRIu64" %"PRId64"", ctx->stRecAttr.id, type, cur, end, ctx->splitTimeOut[type]);
        isSplit = 1;
    }

    if(isSplit == 1){
        if(ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_VIDEO] < ctx->ptsPerfile[type][CVI_MUXER_FRAME_TYPE_VIDEO]){
            ctx->fillFlag = 1;
        }

        if(ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_AUDIO] < ctx->ptsPerfile[type][CVI_MUXER_FRAME_TYPE_AUDIO]){
            ctx->fillFlag = 1;
        }

        if(ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_SUBTITLE] < ctx->ptsPerfile[type][CVI_MUXER_FRAME_TYPE_SUBTITLE]){
            ctx->fillFlag = 1;
        }
        ctx->splitTimeOut[type] = cur - end;
    }
    return isSplit;
}

static int32_t recorder_PacketRealloc(CVI_RECORDER_CTX_S *ctx, CVI_MUXER_FRAME_TYPE_E ftype, CVI_RECORDER_TYPE_INDEX_E type, int32_t size)
{
    if(size > ctx->packetSize[type][ftype]) {
        size = CVI_REC_ALIGN(size, 4096);
        CVI_LOGD("%d totalSize %d packetSize %d", ctx->stRecAttr.id, size, ctx->packetSize[type][ftype]);
        ctx->packet[type][ftype] = realloc(ctx->packet[type][ftype], size);
        if(ctx->packet[type][ftype] == NULL){
            CVI_LOGE("%d out of memory %d !!!!!!!", ctx->stRecAttr.id, size);
            ctx->packetSize[type][ftype] = 0;
            return -1;
        }
        memset(ctx->packet[type][ftype], 0x0, size);
        ctx->packetSize[type][ftype] = size;
    }
    return 0;
}

static CVI_MUXER_FRAME_INFO_S *recorder_GetFrame(CVI_RECORDER_CTX_S *ctx, CVI_MUXER_FRAME_TYPE_E ftype, CVI_RECORDER_TYPE_INDEX_E type, int32_t isRefresh)
{
    CVI_MUXER_FRAME_INFO_S frame;
    void *f = NULL;
    int32_t ret = 0;
    ret = CVI_RBUF_Copy_Out(ctx->rbuf[ftype], (void *)&frame, sizeof(CVI_MUXER_FRAME_INFO_S), 0, type);
    if(ret != 0) {
        return f;
    }

    if(frame.hmagic != 0x5a5a5a5a || frame.tmagic != 0x5a5a5a5a) {
        CVI_LOGE("mem of crossing the line %#x %#x %d", frame.hmagic, frame.tmagic, frame.dataLen);
        CVI_RBUF_ShowLog(ctx->rbuf[ftype]);
        CVI_RBUF_Reset(ctx->rbuf[ftype]);
        return f;
    }

    if(ftype == CVI_MUXER_FRAME_TYPE_VIDEO || (ctx->stRecAttr.enable_subvideo == 1 && ftype == CVI_MUXER_FRAME_TYPE_SUB_VIDEO)){
        if(ctx->isFirstFrame[type][ftype] == 1 && frame.isKey != 1){
            CVI_RBUF_Refresh_Out(ctx->rbuf[ftype], frame.totalSize, type);
            CVI_LOGD("%d %d first frame must be Iframe %"PRId64"", ctx->stRecAttr.id, ftype, frame.pts);
            return f;
        }
    }
    if(ftype == CVI_MUXER_FRAME_TYPE_AUDIO || ftype == CVI_MUXER_FRAME_TYPE_SUBTITLE){
        if(ctx->isFirstFrame[type][CVI_MUXER_FRAME_TYPE_VIDEO] == 1 || (ctx->stRecAttr.enable_subvideo == 1 && ctx->isFirstFrame[type][CVI_MUXER_FRAME_TYPE_SUB_VIDEO] == 1)){
            CVI_RBUF_Refresh_Out(ctx->rbuf[ftype], frame.totalSize, type);
            CVI_LOGD("%d %d first frame must be VIDEO %"PRId64"", ctx->stRecAttr.id, ftype, frame.pts);
            return f;
        }
    }

    ctx->isFirstFrame[type][ftype] = 0;

    if(frame.dataLen > 0) {
        int32_t totalSize = frame.totalSize;
    #if CVI_MUXER_EXT_DATA_LEN > 0
        if(ftype == CVI_MUXER_FRAME_TYPE_AUDIO){
            totalSize += CVI_MUXER_EXT_DATA_LEN;
        }
    #endif
        if(recorder_PacketRealloc(ctx, ftype, type, totalSize) != 0){
            return f;
        }

        frame.pts = ctx->ptsInfile[type][ftype];
        if((ftype == CVI_MUXER_FRAME_TYPE_VIDEO || (ctx->stRecAttr.enable_subvideo == 1 && ftype == CVI_MUXER_FRAME_TYPE_SUB_VIDEO)) && ctx->stMuxerAttr.stthumbnailcodec.en && frame.pts == 0 && frame.extraLen == 0){
            if(ctx->stRecAttr.fncallback.pfn_request_idr){
                ctx->stRecAttr.fncallback.pfn_request_idr(ctx->stRecAttr.fncallback.pfn_request_idr_param, ftype);
            }
        }

        if(ctx->targetFramePts[type][ftype] == 0 && type != CVI_RECORDER_TYPE_EVENT_INDEX){
            uint64_t u64SplitTimeLenUSec = ctx->stRecAttr.stSplitAttr.u64SplitTimeLenMSec * 1000;
            ctx->targetFramePts[type][ftype] = u64SplitTimeLenUSec + ctx->firstFramePts[ftype];
        }
        int32_t off = 0;
        char *buf = (char *)(ctx->packet[type][ftype]);
        memcpy(buf + off, &frame, sizeof(CVI_MUXER_FRAME_INFO_S));
        off += sizeof(CVI_MUXER_FRAME_INFO_S);
    #if CVI_MUXER_EXT_DATA_LEN > 0
        if(ftype == CVI_MUXER_FRAME_TYPE_AUDIO){
            memcpy(buf + off, g_ext_audio_data, CVI_MUXER_EXT_DATA_LEN);
            ret = CVI_RBUF_Copy_Out(ctx->rbuf[ftype], buf + off + CVI_MUXER_EXT_DATA_LEN, frame.dataLen, off, type);
        }else{
            ret = CVI_RBUF_Copy_Out(ctx->rbuf[ftype], buf + off, frame.totalSize - off, off, type);
        }
    #else
        ret = CVI_RBUF_Copy_Out(ctx->rbuf[ftype], buf + off, frame.totalSize - off, off, type);
    #endif

        if(isRefresh){
            CVI_RBUF_Refresh_Out(ctx->rbuf[ftype], frame.totalSize, type);
        }

        f = buf;
    }else{
        CVI_RBUF_Refresh_Out(ctx->rbuf[ftype], frame.totalSize, type);
    }
    return f;
}

static CVI_MUXER_FRAME_TYPE_E recorder_GetFrameType(CVI_RECORDER_CTX_S *ctx, CVI_RECORDER_TYPE_INDEX_E type)
{
    // if(!recorder_AudioEn(ctx->stRecAttr) && !recorder_SubtitleEn(ctx->stRecAttr)){
    //     return CVI_MUXER_FRAME_TYPE_VIDEO;
    // }

    double duration[CVI_MUXER_FRAME_TYPE_BUTT] = {0, 0, 0, 0, 0};

    if(ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_VIDEO] == 0){
        duration[CVI_MUXER_FRAME_TYPE_VIDEO] = 0;
        return CVI_MUXER_FRAME_TYPE_VIDEO;
    }else{
        duration[CVI_MUXER_FRAME_TYPE_VIDEO] =
            ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_VIDEO] * ctx->fduration[CVI_MUXER_FRAME_TYPE_VIDEO];
    }

    if(recorder_SubVideoEn(ctx->stRecAttr)){
        if(ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_SUB_VIDEO] == 0){
            duration[CVI_MUXER_FRAME_TYPE_SUB_VIDEO] = 0;
            return CVI_MUXER_FRAME_TYPE_SUB_VIDEO;
        }else{
            duration[CVI_MUXER_FRAME_TYPE_SUB_VIDEO] =
                ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_SUB_VIDEO] * ctx->fduration[CVI_MUXER_FRAME_TYPE_SUB_VIDEO];
        }
    }

    if(recorder_AudioEn(ctx->stRecAttr)){
        if(ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_AUDIO] == 0){
            duration[CVI_MUXER_FRAME_TYPE_AUDIO] = 0;
        }else{
            duration[CVI_MUXER_FRAME_TYPE_AUDIO] =
                ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_AUDIO] * ctx->fduration[CVI_MUXER_FRAME_TYPE_AUDIO];
        }
    }

    if(recorder_SubtitleEn(ctx->stRecAttr)){
        if(ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_SUBTITLE] == 0){
            duration[CVI_MUXER_FRAME_TYPE_SUBTITLE] = 0;
        }else{
            duration[CVI_MUXER_FRAME_TYPE_SUBTITLE] =
                ctx->ptsInfile[type][CVI_MUXER_FRAME_TYPE_SUBTITLE] * ctx->fduration[CVI_MUXER_FRAME_TYPE_SUBTITLE];
        }
    }
    if(recorder_SubVideoEn(ctx->stRecAttr) && duration[CVI_MUXER_FRAME_TYPE_SUB_VIDEO] < duration[CVI_MUXER_FRAME_TYPE_VIDEO]){
        return CVI_MUXER_FRAME_TYPE_SUB_VIDEO;
    }else if(recorder_AudioEn(ctx->stRecAttr) && duration[CVI_MUXER_FRAME_TYPE_AUDIO] < duration[CVI_MUXER_FRAME_TYPE_VIDEO]){
        return CVI_MUXER_FRAME_TYPE_AUDIO;
    }else if(recorder_SubtitleEn(ctx->stRecAttr) && duration[CVI_MUXER_FRAME_TYPE_SUBTITLE] < duration[CVI_MUXER_FRAME_TYPE_VIDEO]){
        return CVI_MUXER_FRAME_TYPE_SUBTITLE;
    }
    return CVI_MUXER_FRAME_TYPE_VIDEO;
}

static void recorder_SyncCb(void *cb, char *filename, uint32_t event, void *argv0, uint32_t argv1)
{
    CVI_RECORDER_EVENT_CALLBACK callback = (CVI_RECORDER_EVENT_CALLBACK)cb;
    callback(event, filename, argv0);
    CVI_LOGD("id %d %s send event %d\n", argv1, filename, event);
}

static int32_t recorder_GetNextFilename(CVI_RECORDER_CTX_S *ctx, CVI_RECORDER_TYPE_INDEX_E type)
{
    memset(ctx->nextFilename[type], 0x0, sizeof(ctx->nextFilename[type]));
    if(ctx->manualStopFlag == 0){
        if(ctx->stRecAttr.enRecType == CVI_RECORDER_TYPE_NORMAL)
            ctx->stRecAttr.fncallback.pfn_get_filename(ctx->stRecAttr.fncallback.pfn_get_filename_param[CVI_RECORDER_CALLBACK_TYPE_NORMAL], ctx->nextFilename[type], sizeof(ctx->nextFilename[type]) - 1);
        else if(ctx->stRecAttr.enRecType == CVI_RECORDER_TYPE_LAPSE)
            ctx->stRecAttr.fncallback.pfn_get_filename(ctx->stRecAttr.fncallback.pfn_get_filename_param[CVI_RECORDER_CALLBACK_TYPE_LAPSE], ctx->nextFilename[type], sizeof(ctx->nextFilename[type]) - 1);
        CVI_LOGI("[%d.%d] next filename:%s", ctx->stRecAttr.id, type, ctx->nextFilename[type]);
    }
    return 0;
}

static int32_t recorder_AutoSplit(CVI_RECORDER_CTX_S *ctx, CVI_RECORDER_TYPE_INDEX_E type)
{
    CVI_RECORDER_EVENT_CALLBACK callback = ctx->stRecAttr.fncallback.pfn_event_cb[type];
    if(ctx->filename[type][0] == '\0'){
        if(ctx->manualStopFlag == 1 && callback){
            callback(CVI_RECORDER_EVENT_STOP, "", ctx->stRecAttr.fncallback.pfn_event_cb_param);
        }
        return 0;
    }
    ctx->fileCnt[type]++;

    CVI_MUXER_Stop(ctx->muxer[type]);
    CVI_FILESYNC_EVENT_CB_S cb;
    cb.cb = (void *)recorder_SyncCb;
    cb.hdl = ctx->stRecAttr.fncallback.pfn_event_cb[type];
    cb.event = CVI_RECORDER_EVENT_SYNC_DONE;
    cb.argv0 = ctx->stRecAttr.fncallback.pfn_event_cb_param;
    cb.argv1 = ctx->stRecAttr.id;
    CVI_FILESYNC_Push(ctx->filename[type], &cb);

    if(recorder_is_ShortFile((void *)ctx, type)) {
        callback(CVI_RECORDER_EVENT_SHORT_FILE, ctx->filename[type], ctx->stRecAttr.fncallback.pfn_event_cb_param);
    }

    uint64_t u64SplitTimeLenUSec = ctx->stRecAttr.stSplitAttr.u64SplitTimeLenMSec * 1000;
    for(int32_t i = 0; i < CVI_MUXER_FRAME_TYPE_BUTT; i++) {
        if(CVI_MUXER_FRAME_TYPE_THUMBNAIL == i
        || (CVI_MUXER_FRAME_TYPE_SUBTITLE == i && recorder_SubtitleEn(ctx->stRecAttr) == 0)
        || (CVI_MUXER_FRAME_TYPE_AUDIO == i && recorder_AudioEn(ctx->stRecAttr) == 0)
        || (CVI_MUXER_FRAME_TYPE_SUB_VIDEO == i && recorder_SubVideoEn(ctx->stRecAttr) == 0)){
            continue;
        }
        if(CVI_MUXER_FRAME_TYPE_VIDEO == i){
            CVI_LOGI("[VIDEO] %d type %d ptsInfile %"PRId64" ptsInLastfile %"PRId64", ptsPerfile %"PRId64"",
                ctx->stRecAttr.id, type, ctx->ptsInfile[type][i], ctx->ptsInLastfile[type][i],
                ctx->ptsPerfile[type][i]);
        } else if(recorder_SubVideoEn(ctx->stRecAttr) && CVI_MUXER_FRAME_TYPE_SUB_VIDEO == i){
            CVI_LOGI("[SUB VIDEO] %d type %d ptsInfile %"PRId64" ptsInLastfile %"PRId64", ptsPerfile %"PRId64"",
                ctx->stRecAttr.id, type, ctx->ptsInfile[type][i], ctx->ptsInLastfile[type][i],
                ctx->ptsPerfile[type][i]);
        } else if(CVI_MUXER_FRAME_TYPE_AUDIO == i && ctx->stRecAttr.enRecType != CVI_RECORDER_TYPE_LAPSE){
            CVI_LOGI("[AUDIO] %d type %d ptsInfile %"PRId64" ptsInLastfile %"PRId64", ptsPerfile %"PRId64"",
                ctx->stRecAttr.id, type, ctx->ptsInfile[type][i], ctx->ptsInLastfile[type][i],
                ctx->ptsPerfile[type][i]);
        }
        ctx->ptsInLastfile[type][i] += ctx->ptsInfile[type][i];
        ctx->ptsInfile[type][i] = 0;
        ctx->isFirstFrame[type][i] = 1;
        ctx->targetFramePts[type][i] = u64SplitTimeLenUSec * (ctx->fileCnt[type] + 1) + ctx->firstFramePts[i];
    }

    if(type != CVI_RECORDER_TYPE_EVENT_INDEX) {
        if(ctx->stRecAttr.enable_emrfile_from_normfile && ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] == 1){
            callback = ctx->stRecAttr.fncallback.pfn_event_cb[CVI_RECORDER_CALLBACK_TYPE_NORMAL];
            callback(CVI_RECORDER_EVENT_END_EMR, ctx->filename[type], ctx->stRecAttr.fncallback.pfn_event_cb_param);
            ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] = 0;
        }else{
            if(ctx->manualStopFlag == 1){
                callback(CVI_RECORDER_EVENT_STOP, ctx->filename[type], ctx->stRecAttr.fncallback.pfn_event_cb_param);
            }else{
                callback(CVI_RECORDER_EVENT_SPLIT, ctx->filename[type], ctx->stRecAttr.fncallback.pfn_event_cb_param);
            }
        }
        ctx->reqIdrFlag = 0;
    }
    else {
        callback(CVI_RECORDER_EVENT_STOP, ctx->filename[type], ctx->stRecAttr.fncallback.pfn_event_cb_param);
    }

    ctx->recSplit[type] = 1;
    CVI_LOGI("stop fileCnt %d filename %s", ctx->fileCnt[type], ctx->filename[type]);
    memset(ctx->filename[type], 0x0, sizeof(ctx->filename[type]));
    return 0;
}

static void *recorder_GetFrameTmp(CVI_RECORDER_CTX_S *ctx, CVI_MUXER_FRAME_TYPE_E ftype, CVI_RECORDER_TYPE_INDEX_E type)
{
    CVI_MUXER_FRAME_INFO_S frame;
    void *f = NULL;
    int32_t ret = 0;
    ret = CVI_RBUF_Copy_OutTmp(ctx->rbuf[ftype], (void *)&frame, sizeof(CVI_MUXER_FRAME_INFO_S), 0, type);
    if(ret != 0) {
        return f;
    }

    if(frame.hmagic != 0x5a5a5a5a || frame.tmagic != 0x5a5a5a5a) {
        CVI_LOGE("mem of crossing the line %#x %#x %d", frame.hmagic, frame.tmagic, frame.dataLen);
        return f;
    }

    if(frame.dataLen > 0) {
        int32_t totalSize = frame.totalSize;
    #if CVI_MUXER_EXT_DATA_LEN > 0
        if(ftype == CVI_MUXER_FRAME_TYPE_AUDIO){
            totalSize += CVI_MUXER_EXT_DATA_LEN;
        }
    #endif
        if(recorder_PacketRealloc(ctx, ftype, type, totalSize) != 0){
            return f;
        }
        frame.pts = ctx->ptsInfile[type][ftype];

        int32_t off = 0;
        char *buf = (char *)(ctx->packet[type][ftype]);
        memcpy(buf + off, &frame, sizeof(CVI_MUXER_FRAME_INFO_S));
        off += sizeof(CVI_MUXER_FRAME_INFO_S);
    #if CVI_MUXER_EXT_DATA_LEN > 0
        if(ftype == CVI_MUXER_FRAME_TYPE_AUDIO){
            memcpy(buf + off, g_ext_audio_data, CVI_MUXER_EXT_DATA_LEN);
            ret = CVI_RBUF_Copy_OutTmp(ctx->rbuf[ftype], buf + off + CVI_MUXER_EXT_DATA_LEN, frame.dataLen, off, type);
        }else{
            ret = CVI_RBUF_Copy_OutTmp(ctx->rbuf[ftype], buf + off, frame.totalSize - off, off, type);
        }
    #else
        ret = CVI_RBUF_Copy_OutTmp(ctx->rbuf[ftype], buf + off, frame.totalSize - off, off, type);
    #endif
        CVI_RBUF_Refresh_OutTmp(ctx->rbuf[ftype], frame.totalSize, type);
        f = buf;
    }else{
        CVI_RBUF_Refresh_OutTmp(ctx->rbuf[ftype], frame.totalSize, type);
    }
    return f;
}

static int32_t recorder_NextFrameIsKey(CVI_RECORDER_CTX_S *ctx, CVI_RECORDER_TYPE_INDEX_E type)
{
    CVI_MUXER_FRAME_INFO_S *f = NULL;
    f = recorder_GetFrame(ctx, CVI_MUXER_FRAME_TYPE_VIDEO, type, 0);
    if(f && f->hmagic == 0x5a5a5a5a && f->tmagic == 0x5a5a5a5a &&f->isKey){
        return 1;
    }
    return 0;
}

static int32_t recorder_SplitFill(CVI_RECORDER_CTX_S *ctx, CVI_RECORDER_TYPE_INDEX_E type)
{
    ctx->fillFlag = 0;
    int32_t ret = 0;
    recorder_GetNextFilename(ctx, type);
    int32_t iframe = 0;
    int32_t less = 0;
    CVI_MUXER_FRAME_INFO_S *f = NULL;
    CVI_MUXER_FRAME_TYPE_E frameType = CVI_MUXER_FRAME_TYPE_VIDEO;
    for (int32_t i = 0; i < CVI_MUXER_FRAME_TYPE_THUMBNAIL; i++){
        CVI_LOGI("[%d.%d] timeout, but frametype %d not enough [%"PRIu64" < %"PRIu64"]",
            ctx->stRecAttr.id, type, i, ctx->ptsInfile[type][i], ctx->ptsPerfile[type][i]);
        if(ctx->stRecAttr.enRecType == CVI_RECORDER_TYPE_LAPSE){
            break;
        }
    }

AGAIN:
    if(ctx->recStartFlag[type] == 0 || ctx->manualStopFlag == 1 || ctx->fillFlag == 1){
        return 0;
    }

    for(int32_t i = 0;i < CVI_MUXER_FRAME_TYPE_AUDIO;i++){
        if(ctx->dropFrameFlag[i] == 1){
            return 1;
        }
    }

    f = NULL;
    for (int32_t i = 0; i <= CVI_MUXER_FRAME_TYPE_SUBTITLE; i++){
        if(i == CVI_MUXER_FRAME_TYPE_VIDEO && recorder_NextFrameIsKey(ctx, type) == 0){
            less = 1;
            break;
        }
        if(ctx->ptsInfile[type][i] < ctx->ptsPerfile[type][i]){
            less = 1;
            break;
        }
    }

    if(less == 0 && ctx->stRecAttr.enRecType != CVI_RECORDER_TYPE_LAPSE){
        for (int32_t i = 0; i <= CVI_MUXER_FRAME_TYPE_SUBTITLE; i++){
            if(ctx->curFramePts[type][i] < ctx->targetFramePts[type][i]){
                less = 2 + i;
                frameType = i;
                iframe = 0;
                break;
            }
        }
        if(less == 0){
            return 1;
        }
    }

    if(less == 1){
        frameType = recorder_GetFrameType(ctx, type);
        if(frameType == CVI_MUXER_FRAME_TYPE_SUBTITLE){
            frameType = CVI_MUXER_FRAME_TYPE_SUBTITLE;
        }
        if(frameType == CVI_MUXER_FRAME_TYPE_VIDEO && iframe == 0){
            if(recorder_NextFrameIsKey(ctx, type) == 1){
                iframe = 1;
            }
        }
    }

    if(iframe == 1){
        if(frameType == CVI_MUXER_FRAME_TYPE_AUDIO && ctx->curFramePts[type][frameType] - \
            ctx->firstFramePts[frameType] < ctx->curFramePts[type][CVI_MUXER_FRAME_TYPE_VIDEO] - \
            ctx->firstFramePts[CVI_MUXER_FRAME_TYPE_VIDEO]){
            f = recorder_GetFrame(ctx, frameType, type, 1);
        } else if (frameType == CVI_MUXER_FRAME_TYPE_SUBTITLE && ctx->curFramePts[type][frameType] - \
            ctx->firstFramePts[frameType] < ctx->curFramePts[type][CVI_MUXER_FRAME_TYPE_VIDEO] - \
            ctx->firstFramePts[CVI_MUXER_FRAME_TYPE_VIDEO]) {
            f = recorder_GetFrame(ctx, frameType, type, 1);
        } else {
            f = recorder_GetFrameTmp(ctx, frameType, type);
        }
    }else{
        f = recorder_GetFrame(ctx, frameType, type, 1);
    }

    if(f){
        f->pts = ctx->ptsInfile[type][frameType];
        ctx->curFramePts[type][frameType] = f->vpts;
        ret = CVI_MUXER_WritePacket(ctx->muxer[type], frameType, f);
        ctx->ptsInfile[type][frameType]++;
        if(ret < 0){
            return 1;
        }
    }

    if(less == 1){
        less = 0;
        cvi_osal_task_sleep(1000);
        goto AGAIN;
    }

    return 1;
}

static void recorder_EmrTask(void *arg)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)arg;

    CVI_RECORDER_TYPE_INDEX_E rec_type = CVI_RECORDER_TYPE_EVENT_INDEX;

    int32_t ret = 0;
    while(!ctx->task[rec_type].exitflag){
        pthread_mutex_lock(&ctx->mutex[rec_type]);

        if(ctx->recStartFlag[rec_type] == 0){
            for(int32_t i = 0; i < CVI_MUXER_FRAME_TYPE_BUTT; i++){
                // if(i == CVI_MUXER_FRAME_TYPE_SUB_VIDEO && !recorder_SubVideoEn(ctx->stRecAttr))
                //     continue;
                int32_t radio = 0;
                if(CVI_RBUF_Get_Totalsize(ctx->rbuf[i]) > 0){
                    radio = (CVI_RBUF_Get_InSize(ctx->rbuf[i]) % CVI_RBUF_Get_Totalsize(ctx->rbuf[i]))
                        * 100 / CVI_RBUF_Get_Totalsize(ctx->rbuf[i]);
                }

                int32_t fcnt = CVI_RBUF_DataCnt(ctx->rbuf[i], rec_type);
                if((fcnt > 0 && ctx->dropFrameFlag[i] != 0) || radio > 60
                || fcnt * ctx->fduration[i] > ctx->stRecAttr.u32PreRecTimeSec * 1000 * 1000 + RECORDER_PRESTORE_TIME){
                    CVI_MUXER_FRAME_INFO_S frame;
                    ret = CVI_RBUF_Copy_Out(ctx->rbuf[i], (void *)&frame, sizeof(CVI_MUXER_FRAME_INFO_S), 0, rec_type);
                    if(ret == 0){
                        ctx->curFramePts[rec_type][i] = frame.vpts;
                        ctx->targetFramePts[rec_type][i] = frame.vpts
                            + (ctx->stRecAttr.u32PreRecTimeSec + ctx->stRecAttr.u32PostRecTimeSec) * 1000 * 1000;
                        CVI_RBUF_Refresh_Out(ctx->rbuf[i], frame.totalSize, rec_type);
                    }
                }
            }
            pthread_mutex_unlock(&ctx->mutex[rec_type]);
            cvi_osal_task_sleep(10 * 1000);
            continue;
        }

        CVI_MUXER_FRAME_TYPE_E frame_type = recorder_GetFrameType(ctx, rec_type);
        CVI_MUXER_FRAME_INFO_S *f = recorder_GetFrame(ctx, frame_type, rec_type, 1);
        if(f){
            CVI_MUXER_WritePacket(ctx->muxer[rec_type], frame_type, f);
            ctx->ptsInfile[rec_type][frame_type]++;
            ctx->curFramePts[rec_type][frame_type] = f->vpts;
        }

        ret = recorder_is_Split(ctx, rec_type);
        if (ret == 1) {
            recorder_AutoSplit(ctx, rec_type);
            ctx->recStartFlag[rec_type] = 0;
            ctx->splitTimeOut[rec_type] = 0;
        }

        pthread_mutex_unlock(&ctx->mutex[rec_type]);

        cvi_osal_task_sleep(5 * 1000);
    }
}

static void recorder_NormalTask(void *arg)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)arg;

    CVI_RECORDER_TYPE_INDEX_E rec_type = CVI_RECORDER_TYPE_NORMAL_INDEX;
    if(ctx->stRecAttr.enRecType == CVI_RECORDER_TYPE_LAPSE){
        rec_type = CVI_RECORDER_TYPE_LAPSE_INDEX;
    }

    int32_t frameCount = 0;
    int32_t ret = 0;
    uint64_t step0 = 0, step1 = CVI_RECORDER_GetUs();

    while(!ctx->task[rec_type].exitflag){
        pthread_mutex_lock(&ctx->mutex[rec_type]);
        if(ctx->recStartFlag[rec_type] == 0 && ctx->recMemStartFlag == 0) {
            pthread_mutex_unlock(&ctx->mutex[rec_type]);
            cvi_osal_task_sleep(10 * 1000);
            step1 = CVI_RECORDER_GetUs();
            frameCount = 0;
            continue;
        }
        step0 = CVI_RECORDER_GetUs();
        if(step0 - step1 > 150 * 1000){
            CVI_LOGW("task[%d] schedule timeout %"PRIu64"", ctx->stRecAttr.id, step0 - step1);
        }

        CVI_MUXER_FRAME_TYPE_E frame_type = recorder_GetFrameType(ctx, rec_type);
        int32_t radio = (CVI_RBUF_Get_InSize(ctx->rbuf[frame_type]) % CVI_RBUF_Get_Totalsize(ctx->rbuf[frame_type]))
                * 100 / CVI_RBUF_Get_Totalsize(ctx->rbuf[frame_type]);
        int32_t fcnt = CVI_RBUF_DataCnt(ctx->rbuf[frame_type], rec_type);
        if(frame_type == CVI_MUXER_FRAME_TYPE_VIDEO || frame_type == CVI_MUXER_FRAME_TYPE_SUB_VIDEO || frame_type == CVI_MUXER_FRAME_TYPE_AUDIO){
            if(fcnt == 0 || (ctx->stRecAttr.enRecType != CVI_RECORDER_TYPE_LAPSE && radio < 70 && fcnt * ctx->fduration[frame_type] < RECORDER_PRESTORE_TIME && (frame_type < CVI_MUXER_FRAME_TYPE_AUDIO && ctx->dropFrameFlag[frame_type] == 0))){
                pthread_mutex_unlock(&ctx->mutex[rec_type]);
                step1 = CVI_RECORDER_GetUs();
                cvi_osal_task_sleep(5 * 1000);
                continue;
            }
        }
        CVI_MUXER_FRAME_INFO_S *f = recorder_GetFrame(ctx, frame_type, rec_type, 1);
        uint64_t start = CVI_RECORDER_GetUs() / 1000;
        if(f){
            ret = CVI_MUXER_WritePacket(ctx->muxer[rec_type], frame_type, f);
            ctx->ptsInfile[rec_type][frame_type]++;
            ctx->curFramePts[rec_type][frame_type] = f->vpts;
            if(frameCount++ % 3000 == 0 || access("/tmp/rbuf_debug", F_OK) == 0){
                CVI_LOGI("show %d rbuf info:", ctx->stRecAttr.id);
                remove("/tmp/rbuf_debug");
                CVI_RBUF_ShowLog(ctx->rbuf[CVI_MUXER_FRAME_TYPE_VIDEO]);
                if(recorder_SubVideoEn(ctx->stRecAttr))
                    CVI_RBUF_ShowLog(ctx->rbuf[CVI_MUXER_FRAME_TYPE_SUB_VIDEO]);;
                CVI_RBUF_ShowLog(ctx->rbuf[CVI_MUXER_FRAME_TYPE_AUDIO]);
                CVI_RBUF_ShowLog(ctx->rbuf[CVI_MUXER_FRAME_TYPE_SUBTITLE]);
            }
        }

        uint64_t end = CVI_RECORDER_GetUs() / 1000;
        uint64_t t = end - start;
        if(t > 150) {
            CVI_LOGI("[%d %d]: write_file take [%"PRIu64"] ms", ctx->stRecAttr.id, frame_type, t);
            if(t > 200){
                recorder_Event_Callback(CVI_MUXER_SEND_FRAME_TIMEOUT, ctx->filename[rec_type], (void *)&ctx->stRecAttr, (void *)&t);
            }
        }

        ret = recorder_is_Split(ctx, rec_type);
        if(ret == 1){
            if(ctx->fillFlag == 1){
                recorder_SplitFill(ctx, rec_type);
            }else{
                recorder_GetNextFilename(ctx, rec_type);
            }
        }

        if(ret == 1) {
            recorder_AutoSplit(ctx, rec_type);
            if(ctx->stRecAttr.enRecType == CVI_RECORDER_TYPE_NORMAL)
                CVI_RECORDER_Start_NormalRec((void *)ctx);
            else
                CVI_RECORDER_Start_LapseRec((void *)ctx);
        }

        pthread_mutex_unlock(&ctx->mutex[rec_type]);
        step1 = CVI_RECORDER_GetUs();
        cvi_osal_task_sleep(5 * 1000);
    }
}


void CVI_RECORDER_Destroy(void **recorder)
{
    if(!recorder || !*recorder){
        return;
    }

    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)*recorder;
    while(ctx->manualStopFlag) {
        cvi_osal_task_sleep(20 * 1000);
    }

    for(int32_t i = 0; i < CVI_RECORDER_TYPE_BUTT_INDEX; i++) {
        ctx->task[i].exitflag = 1;
        int32_t rc = cvi_osal_task_join(ctx->task[i].task);
        if (rc == CVI_OSAL_SUCCESS) {
            CVI_LOGD("%d task join success, %d", ctx->stRecAttr.id, rc);
            cvi_osal_task_destroy(&(ctx->task[i].task));
        }
    }
    for(int32_t i = 0; i < CVI_RECORDER_TYPE_BUTT_INDEX; i++) {
        pthread_mutex_lock(&ctx->mutex[i]);
        if(ctx->muxer[i] != NULL) {
            if(ctx->recStartFlag[i] == 1) {
                ctx->recStartFlag[i] = 0;
                CVI_MUXER_Stop(ctx->muxer[i]);
            }
            CVI_MUXER_Destroy(ctx->muxer[i]);
            ctx->muxer[i] = NULL;
        }
        pthread_mutex_unlock(&ctx->mutex[i]);
    }

    for(int32_t j = 0; j < CVI_RECORDER_TYPE_BUTT_INDEX; j++){
        pthread_mutex_lock(&ctx->mutex[j]);
        for(int32_t i = 0; i < CVI_MUXER_FRAME_TYPE_BUTT; i++) {
            if(i == CVI_MUXER_FRAME_TYPE_SUB_VIDEO && !recorder_SubVideoEn(ctx->stRecAttr))
                continue;
            if(ctx->rbuf[i] != NULL){
                CVI_RBUF_DeInit(ctx->rbuf[i]);
                ctx->rbuf[i] = NULL;
            }
            if(ctx->packet[j][i]) {
                free(ctx->packet[j][i]);
                ctx->packet[j][i] = NULL;
            }
        }
        pthread_mutex_unlock(&ctx->mutex[j]);
    }

    for(int32_t i = 0; i < CVI_RECORDER_TYPE_BUTT_INDEX; i++){
        pthread_mutex_destroy(&ctx->mutex[i]);
    }

    for(int32_t i = 0; i < CVI_RECORDER_CALLBACK_TYPE_BUTT; i++){
        if(ctx->stRecAttr.fncallback.pfn_get_filename_param[i] != NULL) {
            free(ctx->stRecAttr.fncallback.pfn_get_filename_param[i]);
            ctx->stRecAttr.fncallback.pfn_get_filename_param[i] = NULL;
        }
    }

    CVI_LOGD("%d recorder type %d destroy", ctx->stRecAttr.id, ctx->stRecAttr.enRecType);
    free(ctx);
    *recorder = NULL;
}


int32_t CVI_RECORDER_Create(void **recorder, CVI_RECORDER_ATTR_S *attr)
{
    int32_t rc = -1;
    cvi_osal_task_attr_t ta;
    void *rbuf = NULL;
    uint32_t size = 0;
    static char n_name[16] = {0};
    static char e_name[16] = {0};

    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)malloc(sizeof(CVI_RECORDER_CTX_S));
    if(ctx == NULL){
        goto FAILED;
    }
    memset(ctx, 0x0, sizeof(CVI_RECORDER_CTX_S));

    memcpy(&ctx->stRecAttr, attr, sizeof(CVI_RECORDER_ATTR_S));
    ctx->stMuxerAttr.devmod = attr->device_model;
    ctx->stMuxerAttr.alignflag = attr->enable_file_alignment;
    ctx->stMuxerAttr.presize = attr->prealloc_size;
    CVI_LOGD("prealloc_size=%d", ctx->stMuxerAttr.presize);
    ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].en = 1;
    ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].codec =
        attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.enCodecType;
    ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].framerate =
        attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.fFrameRate;
    ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_VIDEO] =
        (float)attr->stSplitAttr.u64SplitTimeLenMSec * ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].framerate / 1000.0 + CVI_REC_FRAME_COMPENSATION;
    ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_MUXER_FRAME_TYPE_VIDEO] =
        ((float)ctx->stRecAttr.u32PreRecTimeSec + (float)ctx->stRecAttr.u32PostRecTimeSec) * ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].framerate + CVI_REC_FRAME_COMPENSATION;
    ctx->fduration[CVI_MUXER_FRAME_TYPE_VIDEO] = 1000.0 * 1000.0 / ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].framerate;

    if(attr->enable_subvideo == 1){
        ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].en = attr->enable_subvideo;
        ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].codec =
            attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO].unTrackSourceAttr.stVideoInfo.enCodecType;
        ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].framerate =
            attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO].unTrackSourceAttr.stVideoInfo.fFrameRate;
        ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_SUB_VIDEO] =
            (float)attr->stSplitAttr.u64SplitTimeLenMSec * ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].framerate / 1000.0 + CVI_REC_FRAME_COMPENSATION;
        ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_MUXER_FRAME_TYPE_SUB_VIDEO] =
            ((float)ctx->stRecAttr.u32PreRecTimeSec + (float)ctx->stRecAttr.u32PostRecTimeSec) * ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].framerate + CVI_REC_FRAME_COMPENSATION;
        ctx->fduration[CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO] = 1000.0 * 1000.0 / ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].framerate;
    }

    if(ctx->stRecAttr.enRecType == CVI_RECORDER_TYPE_LAPSE){
        if(ctx->stRecAttr.id == 0){
            g_rec0Fps = attr->unRecAttr.stLapseRecAttr.fFramerate;
            ctx->timelapseIntervalNs = attr->unRecAttr.stLapseRecAttr.u32IntervalMs * 1000 * 1000;
        }
        ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].framerate = attr->unRecAttr.stLapseRecAttr.fFramerate;
        ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].framerate = attr->unRecAttr.stLapseRecAttr.fFramerate;
        ctx->stRecAttr.stSplitAttr.u64SplitTimeLenMSec = attr->stSplitAttr.u64SplitTimeLenMSec * ctx->stMuxerAttr.stvideocodec[0].framerate * attr->unRecAttr.stLapseRecAttr.u32IntervalMs / 1000;
        if(ctx->stRecAttr.id != 0){
            if (g_rec0Fps == 0) {
                g_rec0Fps = attr->unRecAttr.stLapseRecAttr.fFramerate;
            }
            float splitTimeLenMSec = (g_rec0Fps * 1000 * 1000 / ctx->stMuxerAttr.stvideocodec[0].framerate);
            ctx->stRecAttr.stSplitAttr.u64SplitTimeLenMSec *= splitTimeLenMSec;
            ctx->stRecAttr.stSplitAttr.u64SplitTimeLenMSec /= (1000 * 1000);
            ctx->ptsPerfile[CVI_RECORDER_TYPE_LAPSE_INDEX][CVI_MUXER_FRAME_TYPE_VIDEO] =
                (float)attr->stSplitAttr.u64SplitTimeLenMSec * ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].framerate / 1000.0 + CVI_REC_FRAME_COMPENSATION;
            // ctx->ptsPerfile[CVI_RECORDER_TYPE_LAPSE_INDEX][CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO] =
            //     (float)attr->stSplitAttr.u64SplitTimeLenMSec * ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].framerate / 1000.0 + CVI_REC_FRAME_COMPENSATION;

            ctx->timelapseIntervalNs = ctx->stRecAttr.stSplitAttr.u64SplitTimeLenMSec * 1000 * 1000 / ctx->ptsPerfile[CVI_RECORDER_TYPE_LAPSE_INDEX][CVI_MUXER_FRAME_TYPE_VIDEO];
        }
    }
    if(ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].en == 0){
        ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_VIDEO] = 0;
        ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_MUXER_FRAME_TYPE_VIDEO] = 0;
    }

    if(ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].en == 0){
        ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO] = 0;
        ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO] = 0;
    }

    ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].w = attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.u32Width;
    ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].h = attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.u32Height;
    ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].w = attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO].unTrackSourceAttr.stVideoInfo.u32Width;
    ctx->stMuxerAttr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].h = attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO].unTrackSourceAttr.stVideoInfo.u32Height;
    CVI_LOGI("video duration per frame %f ptsPerfile %"PRId64"", ctx->fduration[CVI_MUXER_FRAME_TYPE_VIDEO], ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_VIDEO]);
    if(recorder_SubVideoEn(ctx->stRecAttr)){
       CVI_LOGI("sub video duration per frame %f ptsPerfile %"PRId64"", ctx->fduration[CVI_MUXER_FRAME_TYPE_SUB_VIDEO], ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_SUB_VIDEO]);
    }

    ctx->stMuxerAttr.staudiocodec.en = attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_AUDIO].enable;
    if(ctx->stMuxerAttr.staudiocodec.en){
	    ctx->stMuxerAttr.staudiocodec.chns =
	        attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_AUDIO].unTrackSourceAttr.stAudioInfo.u32ChnCnt;
	    ctx->stMuxerAttr.staudiocodec.samplerate =
	        attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_AUDIO].unTrackSourceAttr.stAudioInfo.u32SampleRate;
	    ctx->stMuxerAttr.staudiocodec.framerate =
	        attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_AUDIO].unTrackSourceAttr.stAudioInfo.fFramerate;
	    ctx->stMuxerAttr.staudiocodec.codec =
	        attr->astStreamAttr.aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_AUDIO].unTrackSourceAttr.stAudioInfo.enCodecType;
	    if(ctx->stMuxerAttr.staudiocodec.framerate != 0){
	        ctx->fduration[CVI_MUXER_FRAME_TYPE_AUDIO] = 1000.0 * 1000.0 / ctx->stMuxerAttr.staudiocodec.framerate;
	    }else{
	        ctx->fduration[CVI_MUXER_FRAME_TYPE_AUDIO] = 0;
	    }
	    ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_AUDIO] =
	        (float)attr->stSplitAttr.u64SplitTimeLenMSec * ctx->stMuxerAttr.staudiocodec.framerate / 1000.0 + CVI_REC_FRAME_COMPENSATION;
	    ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_MUXER_FRAME_TYPE_AUDIO] =
	        ((float)ctx->stRecAttr.u32PreRecTimeSec + (float)ctx->stRecAttr.u32PostRecTimeSec) * ctx->stMuxerAttr.staudiocodec.framerate + CVI_REC_FRAME_COMPENSATION;
	    if(ctx->stMuxerAttr.staudiocodec.en == 0){
	        ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_AUDIO] = 0;
	        ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_MUXER_FRAME_TYPE_AUDIO] = 0;
	    }

	    CVI_LOGI("audio duration per frame %f ptsPerfile %"PRId64"", ctx->fduration[CVI_MUXER_FRAME_TYPE_AUDIO], ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_AUDIO]);
    }

    ctx->stMuxerAttr.stthumbnailcodec.en = attr->enable_thumbnail;

    ctx->stMuxerAttr.stsubtitlecodec.en = attr->enable_subtitle;
    if(ctx->stMuxerAttr.stsubtitlecodec.en){
	    ctx->stMuxerAttr.stsubtitlecodec.framerate = attr->subtitle_framerate;
	    ctx->stMuxerAttr.stsubtitlecodec.timebase = 1000000;
	    if(ctx->stMuxerAttr.stsubtitlecodec.framerate != 0){
	        ctx->fduration[CVI_MUXER_FRAME_TYPE_SUBTITLE] = 1000.0 * 1000.0 / ctx->stMuxerAttr.stsubtitlecodec.framerate;
	    }else{
	        ctx->fduration[CVI_MUXER_FRAME_TYPE_SUBTITLE] = 0;
	    }
	    ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_SUBTITLE] =
	        (float)attr->stSplitAttr.u64SplitTimeLenMSec * ctx->stMuxerAttr.stsubtitlecodec.framerate / 1000.0 + CVI_REC_FRAME_COMPENSATION;
	    ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_MUXER_FRAME_TYPE_SUBTITLE] =
	        ((float)ctx->stRecAttr.u32PreRecTimeSec + (float)ctx->stRecAttr.u32PostRecTimeSec) * ctx->stMuxerAttr.stsubtitlecodec.framerate + CVI_REC_FRAME_COMPENSATION;
	    if(ctx->stMuxerAttr.stsubtitlecodec.en == 0){
	        ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_SUBTITLE] = 0;
	        ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_MUXER_FRAME_TYPE_SUBTITLE] = 0;
	    }

	    CVI_LOGI("subtitle duration per frame %f ptsPerfile %"PRId64"", ctx->fduration[CVI_MUXER_FRAME_TYPE_SUBTITLE], ctx->ptsPerfile[CVI_RECORDER_TYPE_NORMAL_INDEX][CVI_MUXER_FRAME_TYPE_SUBTITLE]);
    }
    CVI_LOGD("event ptsPerfile %"PRId64" %"PRId64" %"PRId64"", ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_MUXER_FRAME_TYPE_VIDEO],
        ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_MUXER_FRAME_TYPE_AUDIO], ctx->ptsPerfile[CVI_RECORDER_TYPE_EVENT_INDEX][CVI_MUXER_FRAME_TYPE_SUBTITLE]);

    ctx->stMuxerAttr.u64SplitTimeLenMSec = attr->stSplitAttr.u64SplitTimeLenMSec;
    ctx->stMuxerAttr.pfncallback = recorder_Event_Callback;
    ctx->stMuxerAttr.pfnparam = (void *)&ctx->stRecAttr;

    if(ctx->stRecAttr.enRecType == CVI_RECORDER_TYPE_LAPSE){
        rc = CVI_MUXER_Create(ctx->stMuxerAttr, &ctx->muxer[CVI_RECORDER_TYPE_LAPSE_INDEX]);
    }else {
        rc = CVI_MUXER_Create(ctx->stMuxerAttr, &ctx->muxer[CVI_RECORDER_TYPE_NORMAL_INDEX]);
        ctx->stMuxerAttr.presize = attr->prealloc_size / 2;
        rc |= CVI_MUXER_Create(ctx->stMuxerAttr, &ctx->muxer[CVI_RECORDER_TYPE_EVENT_INDEX]);
        ctx->stMuxerAttr.presize = attr->prealloc_size;
    }

    CVI_LOGD("enRecType = %d, enSplitType %d, u64SplitTimeLenMSec = %"PRIu64"", ctx->stRecAttr.enRecType, ctx->stRecAttr.stSplitAttr.enSplitType, ctx->stRecAttr.stSplitAttr.u64SplitTimeLenMSec);

    int32_t outcnt = 1;
    if(ctx->stRecAttr.enable_emrfile_from_normfile == 0){
        outcnt = 2;
    }

    size = attr->stRbufAttr[CVI_RECORDER_RBUF_VIDEO].size;
    if(CVI_RBUF_Init(&rbuf, size, attr->stRbufAttr[CVI_MUXER_FRAME_TYPE_VIDEO].name, outcnt, attr->fncallback.pfn_rec_malloc_mem , attr->fncallback.pfn_rec_free_mem)){
        goto FAILED;
    }
    CVI_LOGD("video rbuf %p %d", rbuf, size);
    ctx->rbuf[CVI_MUXER_FRAME_TYPE_VIDEO] = rbuf;

    if(attr->enable_subvideo){
        size = attr->stRbufAttr[CVI_RECORDER_RBUF_SUB_VIDEO].size;
        if(CVI_RBUF_Init(&rbuf, size, attr->stRbufAttr[CVI_MUXER_FRAME_TYPE_SUB_VIDEO].name, outcnt, attr->fncallback.pfn_rec_malloc_mem , attr->fncallback.pfn_rec_free_mem)){
            goto FAILED;
        }
        CVI_LOGD("sub video rbuf %p %d", rbuf, size);
        ctx->rbuf[CVI_MUXER_FRAME_TYPE_SUB_VIDEO] = rbuf;
    }

    if(ctx->stMuxerAttr.staudiocodec.en){
        size = attr->stRbufAttr[CVI_RECORDER_RBUF_AUDIO].size;
        rbuf = ctx->rbuf[CVI_MUXER_FRAME_TYPE_AUDIO];
        if(CVI_RBUF_Init(&rbuf, size, attr->stRbufAttr[CVI_MUXER_FRAME_TYPE_AUDIO].name, outcnt, attr->fncallback.pfn_rec_malloc_mem, attr->fncallback.pfn_rec_free_mem)){
            goto FAILED;
        }
        CVI_LOGD("audio rbuf %p %d", rbuf, size);
        ctx->rbuf[CVI_MUXER_FRAME_TYPE_AUDIO] = rbuf;
    }

    if(ctx->stMuxerAttr.stsubtitlecodec.en) {
        size = attr->stRbufAttr[CVI_RECORDER_RBUF_SUBTITLE].size;
        rbuf = ctx->rbuf[CVI_MUXER_FRAME_TYPE_SUBTITLE];
        if(CVI_RBUF_Init(&rbuf, size, attr->stRbufAttr[CVI_MUXER_FRAME_TYPE_SUBTITLE].name, outcnt, attr->fncallback.pfn_rec_malloc_mem,attr->fncallback.pfn_rec_free_mem)){
            goto FAILED;
        }
        CVI_LOGD("subtitle rbuf %p %d", rbuf, size);
        ctx->rbuf[CVI_MUXER_FRAME_TYPE_SUBTITLE] = rbuf;
    }

    for(int32_t i = 0; i < CVI_RECORDER_TYPE_BUTT_INDEX; i++){
        pthread_mutex_init(&ctx->mutex[i], NULL);
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&ctx->mutex[i], &attr);
        pthread_mutexattr_destroy(&attr);
    }

    snprintf(n_name, sizeof(n_name), "rc_normal_%d", attr->id);
    ctx->task[0].exitflag = 0;
    ta.name = n_name;
    ta.entry = recorder_NormalTask;
    ta.param = (void *)ctx;
    ta.priority = CVI_OSAL_PRI_RT_MID;
    ta.detached = false;
    rc = cvi_osal_task_create(&ta, &ctx->task[CVI_RECORDER_TYPE_NORMAL_INDEX].task);
    if (rc != CVI_OSAL_SUCCESS) {
        CVI_LOGE("rc_normal task create failed, %d", rc);
        goto FAILED;
    }

    if(ctx->stRecAttr.enable_emrfile_from_normfile == 0){
        snprintf(e_name, sizeof(e_name), "rc_event_%d", attr->id);
        ta.name = e_name;
        ta.entry = recorder_EmrTask;
        ta.param = (void *)ctx;
        ta.priority = CVI_OSAL_PRI_RT_MID;
        ta.detached = false;
        rc = cvi_osal_task_create(&ta, &ctx->task[CVI_RECORDER_TYPE_EVENT_INDEX].task);
        if (rc != CVI_OSAL_SUCCESS) {
            CVI_LOGE("rc_normal task create failed, %d", rc);
            goto FAILED;
        }
    }

    CVI_FILESYNC_Init();

    *recorder = (void *)ctx;
    CVI_LOGD("success");
    return 0;

FAILED:
    CVI_RECORDER_Destroy((void **)&ctx);
    *recorder = NULL;
    return rc;
}

static void cvi_recorder_init_muxer_frame(CVI_MUXER_FRAME_INFO_S *mframe)
{
    mframe->hmagic = 0x5a5a5a5a;
    mframe->type = CVI_MUXER_FRAME_TYPE_BUTT;
    mframe->isKey = 0;
    mframe->pts = 0;
    mframe->gopInx = 0;
    mframe->vpts = 0;
    mframe->dataLen = 0;
    mframe->extraLen = 0;
    mframe->totalSize = 0;
    mframe->tmagic = 0x5a5a5a5a;
}

static int32_t recorder_wait_videobuf_enough(CVI_RECORDER_CTX_S *ctx, uint32_t target_size, CVI_MUXER_FRAME_TYPE_E type)
{
    int32_t tryCnt = 0;
    do{
        if(CVI_RBUF_Unused(ctx->rbuf[type]) <= target_size){
            if(tryCnt++ < 5){
                ctx->dropFrameFlag[type] = 2;
                cvi_osal_task_sleep(5 * 1000);
                continue;
            }else{
                CVI_LOGD("[%d]: in buf %"PRId64" %"PRId64"", ctx->stRecAttr.id, ctx->ptsInBuf[CVI_MUXER_FRAME_TYPE_VIDEO], ctx->ptsInBuf[CVI_MUXER_FRAME_TYPE_VIDEO]);
                CVI_LOGI("[%d]: drop VIDEO Iframe data %u %d", ctx->stRecAttr.id, CVI_RBUF_Unused(ctx->rbuf[CVI_MUXER_FRAME_TYPE_VIDEO]), target_size);
                CVI_RBUF_ShowLog(ctx->rbuf[CVI_MUXER_FRAME_TYPE_VIDEO]);
                if(recorder_SubVideoEn(ctx->stRecAttr))
                    CVI_RBUF_ShowLog(ctx->rbuf[CVI_MUXER_FRAME_TYPE_SUB_VIDEO]);;
                CVI_RBUF_ShowLog(ctx->rbuf[CVI_MUXER_FRAME_TYPE_AUDIO]);
                CVI_RBUF_ShowLog(ctx->rbuf[CVI_MUXER_FRAME_TYPE_SUBTITLE]);
            }
            ctx->gopInx++;
            ctx->dropFrameFlag[type] = 1;
            return 0;
        }else{
            break;
        }
    }while(1);
    return 1;
}

int32_t CVI_RECORDER_SendFrame(void *recorder, CVI_RECORDER_FRAME_STREAM_S *frame)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);

    if(ctx->rbuf[frame->type] == NULL) {
        return 0;
    }
#if SUPPORT_POST_STOP
    if(ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] == 0 && ctx->recStartFlag[CVI_RECORDER_TYPE_NORMAL_INDEX] == 0
    && ctx->recStartFlag[CVI_RECORDER_TYPE_LAPSE_INDEX] == 0 && ctx->recMemStartFlag == 0){
        return 0;
    }
#endif
    CVI_MUXER_FRAME_INFO_S mframe;
    cvi_recorder_init_muxer_frame(&mframe);

    CVI_RECORDER_TYPE_INDEX_E rec_type = CVI_RECORDER_TYPE_NORMAL_INDEX;
    if(ctx->stRecAttr.enRecType == CVI_RECORDER_TYPE_LAPSE){
        rec_type = CVI_RECORDER_TYPE_LAPSE_INDEX;
    }
    if (frame->type == CVI_MUXER_FRAME_TYPE_VIDEO || frame->type == CVI_MUXER_FRAME_TYPE_SUB_VIDEO) {
        if (frame->type == CVI_MUXER_FRAME_TYPE_VIDEO) {
            mframe.type = CVI_MUXER_FRAME_TYPE_VIDEO;
        } else if(frame->type == CVI_MUXER_FRAME_TYPE_SUB_VIDEO){
            mframe.type = CVI_MUXER_FRAME_TYPE_SUB_VIDEO;
        }
        for (int32_t i = 0; i < frame->num; i++) {
            if(ctx->firstFramePts[mframe.type] == 0){
                if(frame->vftype[i] == 0){
                    CVI_LOGI("start rec %d, first frame must be iframe %"PRIu64" , frame type (%d)", ctx->stRecAttr.id, frame->vi_pts[i],frame->type);
                    return 0;
                }
                if(ctx->stRecAttr.id == 0){
                    g_RecStartTime = CVI_RECORDER_GetUs();
                }
                if(ctx->stRecAttr.enRecType != CVI_RECORDER_TYPE_LAPSE){
                    ctx->splitTimeOut[rec_type] = g_RecStartTime - ctx->recStartTime[rec_type];
                }
                ctx->firstFramePts[mframe.type] = frame->vi_pts[i];
                ctx->reqIdrPts = 2 * ctx->fduration[mframe.type] + frame->vi_pts[i] + ctx->stRecAttr.stSplitAttr.u64SplitTimeLenMSec * 1000;
                CVI_LOGI("%d video firstFramePts %"PRIu64"", ctx->stRecAttr.id, ctx->firstFramePts[mframe.type]);
            }
            if(frame->vftype[i] == 1){
                int32_t icnt = 0;
                mframe.dataLen = 0;
                for(int32_t j = i; j < frame->num && frame->vftype[j] == 1; j++){
                    mframe.dataLen += frame->len[j];
                    icnt++;
                }
                if(icnt == 1){
                    continue;
                }
                if(icnt == 2){
                    i+=1;
                    icnt = 1;
                }

                mframe.vpts = frame->vi_pts[i];
                mframe.gopInx = 0;
                mframe.isKey = 1;
                mframe.pts = ctx->ptsInBuf[mframe.type];
                mframe.extraLen = frame->thumbnail_len;
                mframe.totalSize = mframe.dataLen + sizeof(CVI_MUXER_FRAME_INFO_S) + mframe.extraLen;
                mframe.totalSize = CVI_REC_ALIGN(mframe.totalSize, CVI_REC_ALIGN_LEN);

                if(recorder_wait_videobuf_enough(ctx, mframe.totalSize, frame->type) == 0){
                    return -1;
                }

                if(frame->data[0][4] != 0x67){
                    CVI_LOGD("[%d]: Warning !! I frame not match %#x\n", ctx->stRecAttr.id, frame->data[0][4]);
                }

                int32_t off = 0;
                CVI_RBUF_Copy_In(ctx->rbuf[mframe.type], (void *)&mframe, sizeof(CVI_MUXER_FRAME_INFO_S), 0);
                off += sizeof(CVI_MUXER_FRAME_INFO_S);
                for(int32_t j = i; j < i + icnt; j++){
                    CVI_RBUF_Copy_In(ctx->rbuf[mframe.type], frame->data[j], frame->len[j], off);
                    off += frame->len[j];
                }

                if(frame->thumbnail_len > 0) {
                    CVI_RBUF_Copy_In(ctx->rbuf[mframe.type], frame->thumbnail_data, frame->thumbnail_len, off);
                }

                CVI_RBUF_Refresh_In(ctx->rbuf[mframe.type], mframe.totalSize);

                ctx->gopInx = 0;
                ctx->dropFrameFlag[frame->type] = 0;
                i += (icnt - 1);
            }else if(frame->vftype[i] == 0){
                if(ctx->dropFrameFlag[frame->type] == 1){
                    continue;
                }
                mframe.gopInx = ctx->gopInx;
                mframe.isKey = 0;
                mframe.pts = ctx->ptsInBuf[mframe.type];
                mframe.vpts = frame->vi_pts[i];
                mframe.dataLen = frame->len[i];
                mframe.extraLen = frame->thumbnail_len;
                mframe.totalSize = mframe.dataLen + sizeof(CVI_MUXER_FRAME_INFO_S) + mframe.extraLen;
                mframe.totalSize = CVI_REC_ALIGN(mframe.totalSize, CVI_REC_ALIGN_LEN);

                int32_t off = 0;

                int32_t tryCnt = 0;
                do{
                    if(CVI_RBUF_Unused(ctx->rbuf[mframe.type]) <= (uint32_t)mframe.totalSize){
                        ctx->dropFrameFlag[frame->type] = 2;
                        if(tryCnt++ < 5){
                            cvi_osal_task_sleep(5 * 1000);
                            continue;
                        }
                        break;
                    }else{
                        ctx->dropFrameFlag[frame->type] = 0;
                        break;
                    }
                }while(1);

                if(CVI_RBUF_Unused(ctx->rbuf[mframe.type]) < (uint32_t)mframe.totalSize){
                    CVI_LOGI("[%d]: drop VIDEO pframe data %u %d", ctx->stRecAttr.id, CVI_RBUF_Unused(ctx->rbuf[mframe.type]), mframe.totalSize);
                    ctx->gopInx++;
                    ctx->dropFrameFlag[frame->type] = 2;
                    continue;
                }
                CVI_RBUF_Copy_In(ctx->rbuf[mframe.type], (void *)&mframe, sizeof(CVI_MUXER_FRAME_INFO_S), off);
                off += sizeof(CVI_MUXER_FRAME_INFO_S);
                CVI_RBUF_Copy_In(ctx->rbuf[mframe.type], frame->data[i], frame->len[i], off);
                off += frame->len[i];
                if(frame->thumbnail_len > 0) {
                    CVI_RBUF_Copy_In(ctx->rbuf[mframe.type], frame->thumbnail_data, frame->thumbnail_len, off);
                }
                CVI_RBUF_Refresh_In(ctx->rbuf[mframe.type], mframe.totalSize);
                ctx->gopInx++;
            }
            ctx->ptsInBuf[mframe.type]++;
        }
        if(ctx->stRecAttr.enRecType == CVI_RECORDER_TYPE_LAPSE){
            if((ctx->ptsInBuf[mframe.type] % ctx->ptsPerfile[rec_type][mframe.type]) == 0){
                recorder_ReqIdr(ctx, mframe.type);
            }
        }else{
        	if(frame->vi_pts[frame->num - 1] >= ctx->reqIdrPts){
	            recorder_ReqIdr(ctx, mframe.type);
	            ctx->reqIdrPts += ctx->stRecAttr.stSplitAttr.u64SplitTimeLenMSec * 1000;
            }
        }
    }
    else if (frame->type == CVI_MUXER_FRAME_TYPE_AUDIO){
        mframe.type = CVI_MUXER_FRAME_TYPE_AUDIO;
        if(ctx->firstFramePts[CVI_MUXER_FRAME_TYPE_VIDEO] == 0){
            return 0;
        }

        for(int32_t i = 0;i < CVI_MUXER_FRAME_TYPE_AUDIO;i++){
            if(ctx->dropFrameFlag[i] == 1){
                return 0;
            }
        }

        if(ctx->firstFramePts[mframe.type] == 0){
            ctx->firstFramePts[mframe.type] = frame->vi_pts[0];
            CVI_LOGI("%d audio firstFramePts %"PRIu64"", ctx->stRecAttr.id, ctx->firstFramePts[mframe.type]);
        }

        mframe.vpts = frame->vi_pts[0];
        mframe.isKey = 1;
        mframe.dataLen = frame->len[0];
        mframe.totalSize = mframe.dataLen + sizeof(CVI_MUXER_FRAME_INFO_S);

        mframe.pts = ctx->ptsInBuf[mframe.type];
        if(CVI_RBUF_Unused(ctx->rbuf[mframe.type]) < (uint32_t)mframe.totalSize){
            CVI_LOGI("[%d]: drop audio data %u %d", ctx->stRecAttr.id, CVI_RBUF_Unused(ctx->rbuf[mframe.type]), mframe.totalSize);
            return 0;
        }
        int32_t off = 0;
        CVI_RBUF_Copy_In(ctx->rbuf[mframe.type], (void *)&mframe, sizeof(CVI_MUXER_FRAME_INFO_S), off);
        off = sizeof(CVI_MUXER_FRAME_INFO_S);
        for (int32_t i = 0; i < frame->num; i++) {
            CVI_RBUF_Copy_In(ctx->rbuf[mframe.type], frame->data[i], frame->len[i], off);
            off += frame->len[i];
        }
        CVI_RBUF_Refresh_In(ctx->rbuf[mframe.type], mframe.totalSize);
        ctx->ptsInBuf[mframe.type]++;
    }
    else if(frame->type == CVI_MUXER_FRAME_TYPE_SUBTITLE){
        mframe.type = CVI_MUXER_FRAME_TYPE_SUBTITLE;
        if(ctx->firstFramePts[mframe.type] == 0){
            ctx->firstFramePts[mframe.type] = frame->vi_pts[0];
            CVI_LOGD("%d subtile firstFramePts %"PRIu64"", ctx->stRecAttr.id, ctx->firstFramePts[mframe.type]);
        }
        mframe.vpts = frame->vi_pts[0];
        mframe.isKey = 1;
        mframe.pts = ctx->ptsInBuf[mframe.type];
        mframe.dataLen = frame->len[0];
        mframe.totalSize = mframe.dataLen + sizeof(CVI_MUXER_FRAME_INFO_S);
        ctx->ptsInBuf[mframe.type]++;
        if(CVI_RBUF_Unused(ctx->rbuf[mframe.type]) > (uint32_t)mframe.totalSize) {
            CVI_RBUF_Copy_In(ctx->rbuf[mframe.type], (void *)&mframe, sizeof(CVI_MUXER_FRAME_INFO_S), 0);
            int32_t off = sizeof(CVI_MUXER_FRAME_INFO_S);
            CVI_RBUF_Copy_In(ctx->rbuf[mframe.type], frame->data[0], frame->len[0], off);
            CVI_RBUF_Refresh_In(ctx->rbuf[mframe.type], mframe.totalSize);
        }
    }else {
        CVI_LOGE("[%d: frame type [%d] invailed", ctx->stRecAttr.id, frame->type);
    }
    return 0;
}

static void recorder_ManualStopTask(void *arg)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)arg;

    CVI_RECORDER_TYPE_E type = ctx->stRecAttr.enRecType;
    CVI_RECORDER_TYPE_INDEX_E index = CVI_RECORDER_TYPE_NORMAL_INDEX;
    if(type == CVI_RECORDER_TYPE_LAPSE) {
        index = CVI_RECORDER_TYPE_LAPSE_INDEX;
    }

    for(int32_t i = 0;i < CVI_MUXER_FRAME_TYPE_AUDIO;i++){
        ctx->dropFrameFlag[i] = 2;
    }

    pthread_mutex_lock(&ctx->mutex[index]);
    if(ctx->recStartFlag[index] == 1){
    #if SUPPORT_POST_STOP
        //uint32_t post_time = ctx->stRecAttr.u32PostRecTimeSec;
        // uint64_t start = CVI_RECORDER_GetUs() / 1000;
        // if(ctx->stRecAttr.enRecType != CVI_RECORDER_TYPE_LAPSE){
        //     while(recorder_is_ShortFile((void *)ctx, index)){
        //         pthread_mutex_unlock(&ctx->mutex[index]);
        //         cvi_osal_task_sleep(20 * 1000);
        //         CVI_LOGD("rec %d ptsInfile %"PRId64"", ctx->stRecAttr.id, ctx->ptsInfile[index][CVI_MUXER_FRAME_TYPE_VIDEO]);
        //         pthread_mutex_lock(&ctx->mutex[index]);
        //         uint64_t end = CVI_RECORDER_GetUs() / 1000;
        //         if(end - start >= ctx->stRecAttr.short_file_ms){
        //             break;
        //         }
        //     }
        // }
    #endif
        ctx->recStartFlag[index] = 0;
        recorder_AutoSplit(ctx, index);
    }

    pthread_mutex_unlock(&ctx->mutex[index]);
    CVI_RECORDER_ForceStop_EventRec(arg);

    CVI_LOGI("rec id %d : %d %d %d %d", ctx->stRecAttr.id, ctx->recStartFlag[CVI_RECORDER_TYPE_NORMAL_INDEX], ctx->recStartFlag[CVI_RECORDER_TYPE_LAPSE_INDEX],
        ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX], ctx->recMemStartFlag);
    if(ctx->recStartFlag[CVI_RECORDER_TYPE_NORMAL_INDEX] == 0 && ctx->recStartFlag[CVI_RECORDER_TYPE_LAPSE_INDEX] == 0
    && ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] == 0 && ctx->recMemStartFlag == 0){
        if(ctx->stRecAttr.fncallback.pfn_rec_stop_cb){
            ctx->stRecAttr.fncallback.pfn_rec_stop_cb(ctx->stRecAttr.fncallback.pfn_rec_stop_cb_param);
        }
        ctx->gopInx = 0;
        for(int32_t i = 0;i < CVI_MUXER_FRAME_TYPE_AUDIO;i++){
            ctx->dropFrameFlag[i] = 0;
        }
        ctx->timelapsePts[CVI_MUXER_VIDEO_TRACK_IDX_0] = 0; /*new*/
        ctx->timelapsePts[CVI_MUXER_VIDEO_TRACK_IDX_1] = 0;
        ctx->timelapseLessCnt[CVI_MUXER_VIDEO_TRACK_IDX_0] = 0;
        ctx->timelapseLessCnt[CVI_MUXER_VIDEO_TRACK_IDX_1] = 0;
        ctx->reqIdrFlag = 0;
        CVI_LOGD("rec %d", ctx->stRecAttr.id);
        for(int32_t j = 0; j < CVI_RECORDER_TYPE_BUTT_INDEX; j++){
            pthread_mutex_lock(&ctx->mutex[j]);
            for(int32_t i = 0; i < CVI_MUXER_FRAME_TYPE_BUTT; i++) {
                if(i == CVI_MUXER_FRAME_TYPE_SUB_VIDEO && !recorder_SubVideoEn(ctx->stRecAttr))
                    continue;
                if(ctx->rbuf[i] != NULL){
                    CVI_RBUF_Reset((void *)ctx->rbuf[i]);
                }
                ctx->ptsInLastfile[j][i] = 0;
                ctx->ptsInfile[j][i] = 0;
                ctx->isFirstFrame[j][i] = 1;
                ctx->ptsInBuf[i] = 0;
                ctx->firstFramePts[i] = 0;
                ctx->curFramePts[j][i] = 0;
                ctx->targetFramePts[j][i] = 0;

            }
            ctx->recStartTime[j] = 0;
            ctx->fileCnt[j] = 0;
            ctx->splitTimeOut[j] = 0;
            memset(ctx->filename[j], 0x0, sizeof(ctx->filename[j]));
            memset(ctx->nextFilename[j], 0x0, sizeof(ctx->nextFilename[j]));
            pthread_mutex_unlock(&ctx->mutex[j]);
        }
        CVI_LOGI("rec manual stop %d end", ctx->stRecAttr.id);
    }
    ctx->manualStopFlag = 0;
}

int32_t CVI_RECORDER_Start_MemRec(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    ctx->recMemStartFlag = 1;
    return 0;
}

int32_t CVI_RECORDER_Stop_MemRec(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    ctx->fillFlag = 1;
    if(ctx->recMemStartFlag == 0) {
        return 0;
    }
    ctx->recMemStartFlag = 0;
    return 0;
}

int32_t CVI_RECORDER_Start_NormalRec(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    while(ctx->manualStopFlag) {
        cvi_osal_task_sleep(10 * 1000);
    }

    pthread_mutex_lock(&ctx->mutex[CVI_RECORDER_TYPE_NORMAL_INDEX]);
    if(ctx->recStartFlag[CVI_RECORDER_TYPE_NORMAL_INDEX] == 1 && ctx->recSplit[CVI_RECORDER_TYPE_NORMAL_INDEX] == 0) {
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_NORMAL_INDEX]);
        return 0;
    }

    if(ctx->nextFilename[CVI_RECORDER_TYPE_NORMAL_INDEX][0] == '\0'){
        ctx->stRecAttr.fncallback.pfn_get_filename(ctx->stRecAttr.fncallback.pfn_get_filename_param[CVI_RECORDER_CALLBACK_TYPE_NORMAL],
            ctx->nextFilename[CVI_RECORDER_TYPE_NORMAL_INDEX], sizeof(ctx->nextFilename[CVI_RECORDER_TYPE_NORMAL_INDEX]) - 1);
    }
    if(strlen(ctx->nextFilename[CVI_RECORDER_TYPE_NORMAL_INDEX]) == 0){
        if(ctx->recStartFlag[CVI_RECORDER_TYPE_NORMAL_INDEX] == 1 && ctx->stRecAttr.fncallback.pfn_rec_stop_cb){
            ctx->stRecAttr.fncallback.pfn_rec_stop_cb(ctx->stRecAttr.fncallback.pfn_rec_stop_cb_param);
        }
        ctx->recStartFlag[CVI_RECORDER_TYPE_NORMAL_INDEX] = 0;
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_NORMAL_INDEX]);
        return -1;
    }
    memset(ctx->filename[CVI_RECORDER_TYPE_NORMAL_INDEX], 0x0, sizeof(ctx->filename[CVI_RECORDER_TYPE_NORMAL_INDEX]));
    strncpy(ctx->filename[CVI_RECORDER_TYPE_NORMAL_INDEX], ctx->nextFilename[CVI_RECORDER_TYPE_NORMAL_INDEX], sizeof(ctx->filename[CVI_RECORDER_TYPE_NORMAL_INDEX]) - 1);

    int32_t ret = CVI_MUXER_Start(ctx->muxer[CVI_RECORDER_TYPE_NORMAL_INDEX], (const char *)ctx->filename[CVI_RECORDER_TYPE_NORMAL_INDEX]);
    if(ret < 0) {
        ctx->recStartFlag[CVI_RECORDER_TYPE_NORMAL_INDEX] = 0;
        memset(ctx->filename[CVI_RECORDER_TYPE_NORMAL_INDEX], 0x0, sizeof(ctx->filename[CVI_RECORDER_TYPE_NORMAL_INDEX]));
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_NORMAL_INDEX]);
        return -1;
    }

    CVI_MUXER_FlushPackets(ctx->muxer[CVI_RECORDER_TYPE_EVENT_INDEX], 0);
    ctx->recStartFlag[CVI_RECORDER_TYPE_NORMAL_INDEX] = 1;
    ctx->recSplit[CVI_RECORDER_TYPE_NORMAL_INDEX] = 0;
    pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_NORMAL_INDEX]);

    CVI_RECORDER_EVENT_CALLBACK callback = ctx->stRecAttr.fncallback.pfn_event_cb[CVI_RECORDER_TYPE_NORMAL_INDEX];
    if(ctx->fileCnt[CVI_RECORDER_TYPE_NORMAL_INDEX] == 0) {
        ctx->recStartTime[CVI_RECORDER_TYPE_NORMAL_INDEX] = CVI_RECORDER_GetUs();
        callback(CVI_RECORDER_EVENT_START, ctx->filename[CVI_RECORDER_TYPE_NORMAL_INDEX],
                ctx->stRecAttr.fncallback.pfn_event_cb_param);
    }else{
        callback(CVI_RECORDER_EVENT_SPLIT_START, ctx->filename[CVI_RECORDER_TYPE_NORMAL_INDEX],
                ctx->stRecAttr.fncallback.pfn_event_cb_param);
    }

    CVI_LOGI("start filename %s", ctx->filename[CVI_RECORDER_TYPE_NORMAL_INDEX]);

    return ret;
}

int32_t CVI_RECORDER_Stop_NormalRec(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    CVI_LOGD("stop normal rec %d", ctx->stRecAttr.id);
    ctx->fillFlag = 1;
    pthread_mutex_lock(&ctx->mutex[CVI_RECORDER_TYPE_NORMAL_INDEX]);
    if(ctx->recStartFlag[CVI_RECORDER_TYPE_NORMAL_INDEX] == 0) {
        CVI_RECORDER_EVENT_CALLBACK callback = ctx->stRecAttr.fncallback.pfn_event_cb[CVI_RECORDER_TYPE_NORMAL_INDEX];
        callback(CVI_RECORDER_EVENT_STOP_FAILED, "", ctx->stRecAttr.fncallback.pfn_event_cb_param);
    }
    ctx->manualStopFlag = 1;
    pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_NORMAL_INDEX]);

    static char name[16] = {0};
    snprintf(name, sizeof(name), "manual_stop_%d", ctx->stRecAttr.id);
    cvi_osal_task_attr_t ta;
    cvi_osal_task_handle_t task;
    ta.name = name;
    ta.entry = recorder_ManualStopTask;
    ta.param = (void *)ctx;
    ta.priority = CVI_OSAL_PRI_RT_HIGH;
    ta.detached = true;
    cvi_osal_task_create(&ta, &task);
    CVI_LOGD("First:ctx->manualStopFla:ctx->stRecAttr.id:%d",ctx->stRecAttr.id);
    while (ctx->manualStopFlag != 0 ) {
        usleep(5000);
    }
    CVI_LOGD("End:ctx->manualStopFla:ctx->stRecAttr.id:%d",ctx->stRecAttr.id);
    return 0;
}

int32_t CVI_RECORDER_Start_EventRec(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    pthread_mutex_lock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);
    if(ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] == 1 || ctx->manualStopFlag == 1) {
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);
        return 0;
    }

    pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);

    if(ctx->stRecAttr.enable_emrfile_from_normfile){
        pthread_mutex_lock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);
        ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] = 1;
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);
        // CVI_RECORDER_EVENT_CALLBACK callback = ctx->stRecAttr.fncallback.pfn_event_cb[CVI_RECORDER_TYPE_EVENT_INDEX];
        // callback(CVI_RECORDER_EVENT_START_EMR, ctx->filename[CVI_RECORDER_TYPE_EVENT_INDEX], ctx->stRecAttr.fncallback.pfn_event_cb_param);
        return 0;
    }

    for(int32_t i = 0; i < CVI_MUXER_FRAME_TYPE_BUTT; i++) {
        // if(i == CVI_MUXER_FRAME_TYPE_SUB_VIDEO && !recorder_SubVideoEn(ctx->stRecAttr))
        //     continue;
        ctx->isFirstFrame[CVI_RECORDER_TYPE_EVENT_INDEX][i] = 1;
    }

    pthread_mutex_lock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);
    ctx->stRecAttr.fncallback.pfn_get_filename(ctx->stRecAttr.fncallback.pfn_get_filename_param[CVI_RECORDER_CALLBACK_TYPE_EVENT],
        ctx->nextFilename[CVI_RECORDER_TYPE_EVENT_INDEX], sizeof(ctx->nextFilename[CVI_RECORDER_TYPE_EVENT_INDEX]) - 1);
    if(strlen(ctx->nextFilename[CVI_RECORDER_TYPE_EVENT_INDEX]) == 0){
        ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] = 0;
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);
        return -1;
    }
    strncpy(ctx->filename[CVI_RECORDER_TYPE_EVENT_INDEX], ctx->nextFilename[CVI_RECORDER_TYPE_EVENT_INDEX], sizeof(ctx->filename[CVI_RECORDER_TYPE_EVENT_INDEX]) - 1);
    int32_t ret = CVI_MUXER_Start(ctx->muxer[CVI_RECORDER_TYPE_EVENT_INDEX], (const char *)ctx->filename[CVI_RECORDER_TYPE_EVENT_INDEX]);
    if(ret < 0) {
        ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] = 0;
        memset(ctx->filename[CVI_RECORDER_TYPE_EVENT_INDEX], 0x0, sizeof(ctx->filename[CVI_RECORDER_TYPE_EVENT_INDEX]));
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);
        return -1;
    }

    CVI_MUXER_FlushPackets(ctx->muxer[CVI_RECORDER_TYPE_EVENT_INDEX], 1);
    ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] = 1;
    pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);
    CVI_RECORDER_EVENT_CALLBACK callback = ctx->stRecAttr.fncallback.pfn_event_cb[CVI_RECORDER_TYPE_EVENT_INDEX];
    callback(CVI_RECORDER_EVENT_START_EMR, ctx->filename[CVI_RECORDER_TYPE_EVENT_INDEX], ctx->stRecAttr.fncallback.pfn_event_cb_param);
    ctx->recStartTime[CVI_RECORDER_TYPE_EVENT_INDEX] = CVI_RECORDER_GetUs();
    ctx->reqIdrFlag = 0;
    for(int32_t i = 0 ; i < CVI_MUXER_FRAME_TYPE_AUDIO; i++){
        recorder_ReqIdr(ctx, i);
    }
    CVI_LOGI("start filename %s", ctx->filename[CVI_RECORDER_TYPE_EVENT_INDEX]);
    return ret;
}

int32_t CVI_RECORDER_ForceStop_EventRec(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    ctx->fillFlag = 1;
    if(ctx->stRecAttr.enable_emrfile_from_normfile){
        return 0;
    }

    pthread_mutex_lock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);
    if(ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] == 0) {
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);
        return 0;
    }
    ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] = 0;
    recorder_AutoSplit(ctx, CVI_RECORDER_TYPE_EVENT_INDEX);
    pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_EVENT_INDEX]);

    return 0;
}

int32_t CVI_RECORDER_Stop_EventRec(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    ctx->fillFlag = 1;
    if(ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] == 0) {
        CVI_RECORDER_EVENT_CALLBACK callback = ctx->stRecAttr.fncallback.pfn_event_cb[CVI_RECORDER_TYPE_EVENT_INDEX];
        callback(CVI_RECORDER_EVENT_STOP_FAILED, "", ctx->stRecAttr.fncallback.pfn_event_cb_param);
    }

    if(ctx->stRecAttr.enable_emrfile_from_normfile){
        return 0;
    }

    while(ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] == 1){
        cvi_osal_task_sleep(50 * 1000);
    }

    return 0;
}

int32_t CVI_RECORDER_Stop_EventRecPost(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] = 0;
    cvi_osal_task_sleep(10 * 1000);
    return 0;
}

int32_t CVI_RECORDER_Start_LapseRec(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    while(ctx->manualStopFlag) {
        cvi_osal_task_sleep(10 * 1000);
    }

    pthread_mutex_lock(&ctx->mutex[CVI_RECORDER_TYPE_LAPSE_INDEX]);
    if(ctx->recStartFlag[CVI_RECORDER_TYPE_LAPSE_INDEX] == 1 && ctx->recSplit[CVI_RECORDER_TYPE_LAPSE_INDEX] == 0) {
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_LAPSE_INDEX]);
        return 0;
    }

    if(ctx->nextFilename[CVI_RECORDER_TYPE_LAPSE_INDEX][0] == '\0'){
        ctx->stRecAttr.fncallback.pfn_get_filename(ctx->stRecAttr.fncallback.pfn_get_filename_param[CVI_RECORDER_CALLBACK_TYPE_LAPSE],
            ctx->nextFilename[CVI_RECORDER_TYPE_LAPSE_INDEX], sizeof(ctx->nextFilename[CVI_RECORDER_TYPE_LAPSE_INDEX]) - 1);
    }
    if(strlen(ctx->nextFilename[CVI_RECORDER_TYPE_LAPSE_INDEX]) == 0){
        if(ctx->recStartFlag[CVI_RECORDER_TYPE_EVENT_INDEX] == 1 && ctx->stRecAttr.fncallback.pfn_rec_stop_cb){
            ctx->stRecAttr.fncallback.pfn_rec_stop_cb(ctx->stRecAttr.fncallback.pfn_rec_stop_cb_param);
        }
        ctx->recStartFlag[CVI_RECORDER_TYPE_LAPSE_INDEX] = 0;
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_LAPSE_INDEX]);
        return -1;
    }
    memset(ctx->filename[CVI_RECORDER_TYPE_LAPSE_INDEX], 0x0, sizeof(ctx->filename[CVI_RECORDER_TYPE_LAPSE_INDEX]));
    strncpy(ctx->filename[CVI_RECORDER_TYPE_LAPSE_INDEX], ctx->nextFilename[CVI_RECORDER_TYPE_LAPSE_INDEX], sizeof(ctx->filename[CVI_RECORDER_TYPE_LAPSE_INDEX]) - 1);

    int32_t ret = CVI_MUXER_Start(ctx->muxer[CVI_RECORDER_TYPE_LAPSE_INDEX], (const char *)ctx->filename[CVI_RECORDER_TYPE_LAPSE_INDEX]);
    if(ret < 0) {
        ctx->recStartFlag[CVI_RECORDER_TYPE_LAPSE_INDEX] = 0;
        memset(ctx->filename[CVI_RECORDER_TYPE_LAPSE_INDEX], 0x0, sizeof(ctx->filename[CVI_RECORDER_TYPE_LAPSE_INDEX]));
        pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_LAPSE_INDEX]);
        return -1;
    }

    CVI_MUXER_FlushPackets(ctx->muxer[CVI_RECORDER_TYPE_LAPSE_INDEX], 1);
    ctx->recStartFlag[CVI_RECORDER_TYPE_LAPSE_INDEX] = 1;
    ctx->recSplit[CVI_RECORDER_TYPE_LAPSE_INDEX] = 0;
    pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_LAPSE_INDEX]);

    CVI_RECORDER_EVENT_CALLBACK callback = ctx->stRecAttr.fncallback.pfn_event_cb[CVI_RECORDER_TYPE_LAPSE_INDEX];
    if(ctx->fileCnt[CVI_RECORDER_TYPE_LAPSE_INDEX] == 0) {
        ctx->recStartTime[CVI_RECORDER_TYPE_LAPSE_INDEX] = CVI_RECORDER_GetNs();
        callback(CVI_RECORDER_EVENT_START, ctx->filename[CVI_RECORDER_TYPE_LAPSE_INDEX], ctx->stRecAttr.fncallback.pfn_event_cb_param);
    }else{
        callback(CVI_RECORDER_EVENT_SPLIT_START, ctx->filename[CVI_RECORDER_TYPE_LAPSE_INDEX], ctx->stRecAttr.fncallback.pfn_event_cb_param);
    }

    CVI_LOGI("start filename %s", ctx->filename[CVI_RECORDER_TYPE_LAPSE_INDEX]);
    return ret;
}

int32_t CVI_RECORDER_Stop_LapseRec(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    CVI_LOGD("stop lapse rec %d", ctx->stRecAttr.id);
    ctx->fillFlag = 1;
    pthread_mutex_lock(&ctx->mutex[CVI_RECORDER_TYPE_LAPSE_INDEX]);
    if(ctx->recStartFlag[CVI_RECORDER_TYPE_LAPSE_INDEX] == 0) {
        CVI_RECORDER_EVENT_CALLBACK callback = ctx->stRecAttr.fncallback.pfn_event_cb[CVI_RECORDER_TYPE_LAPSE_INDEX];
        callback(CVI_RECORDER_EVENT_STOP_FAILED, "", ctx->stRecAttr.fncallback.pfn_event_cb_param);
    }
    ctx->manualStopFlag = 1;
    pthread_mutex_unlock(&ctx->mutex[CVI_RECORDER_TYPE_LAPSE_INDEX]);

    static char name[16] = {0};
    snprintf(name, sizeof(name), "manual_stop_%d", ctx->stRecAttr.id);
    cvi_osal_task_attr_t ta;
    cvi_osal_task_handle_t task;
    ta.name = name;
    ta.entry = recorder_ManualStopTask;
    ta.param = (void *)ctx;
    ta.priority = CVI_OSAL_PRI_NORMAL;
    ta.detached = true;
    cvi_osal_task_create(&ta, &task);
    return 0;
}

int32_t CVI_RECORDER_Split(void *recorder)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    if(ctx->recSplit[CVI_RECORDER_TYPE_NORMAL_INDEX] == 1) {
        return 0;
    }
    return 0;
}

int32_t CVI_RECORDER_Timelapse_Is_SendVenc(void *recorder,CVI_MUXER_FRAME_TYPE_E type)
{
    CVI_RECORDER_CTX_S *ctx = (CVI_RECORDER_CTX_S *)recorder;
    CVI_CHECK_CTX_NULL(ctx);
    uint64_t intervalNs = ctx->timelapseIntervalNs;
    uint64_t curTime = CVI_RECORDER_GetNs();
    uint64_t startTime = ctx->recStartTime[CVI_RECORDER_TYPE_LAPSE_INDEX];
    if(ctx->timelapsePts[type] == 0 || ctx->timelapseLessCnt[type] > 0 || curTime >= (startTime + intervalNs * ctx->timelapsePts[type])){
        if(ctx->timelapseLessCnt[type] > 0){
            CVI_LOGD("[%d] s %"PRIu64" c %"PRIu64" %"PRIu64" %d %"PRIu64"", ctx->stRecAttr.id, startTime, curTime, ctx->timelapsePts[type], ctx->timelapseLessCnt[type], intervalNs);
            ctx->timelapsePts[type]++;
            ctx->timelapseLessCnt[type]--;
            return 1;
        }

        if(ctx->timelapseLessCnt[type] < 0){
            ctx->timelapseLessCnt[type]++;

            return 0;
        }
        ctx->timelapseLessCnt[type] = (curTime - startTime) / intervalNs - ctx->timelapsePts[type];
        ctx->timelapsePts[type]++;
        CVI_LOGD("[%d] s %"PRIu64" c %"PRIu64" %"PRIu64" %d %"PRIu64"", ctx->stRecAttr.id, startTime, curTime, ctx->timelapsePts[type], ctx->timelapseLessCnt[type], intervalNs);
        return 1;
    }
    return 0;
}


#ifdef __cplusplus
}
#endif


