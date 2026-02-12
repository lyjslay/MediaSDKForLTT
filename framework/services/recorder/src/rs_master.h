#ifndef __CVI_REC_MASTER_H__
#define __CVI_REC_MASTER_H__

#include "cvi_osal.h"

#include "rs_define.h"
#include "cvi_recorder.h"
#include "cvi_record_service.h"
#include "filesync.h"


/* rec callback set */
typedef struct cviREC_CALLBACK_S {
    void *pfnRequestFileNames; /* callback  request file names */
    void *pfnGetDirType;
    void *pfnNormalRecCb;
    void *pfnLapseRecCb;
    void *pfnEventRecCb;
    void *pfnGetSubtitleCb;
    void *pfnGetGPSInfoCb;
} CVI_REC_CALLBACK_S;




typedef struct cviMAPI_HANDLE_S{
    CVI_MAPI_VPROC_HANDLE_T rec_vproc;
    CVI_MAPI_VPROC_HANDLE_T sub_rec_vproc;
    CVI_MAPI_VPROC_HANDLE_T thumbnail_vproc;
    CVI_MAPI_VENC_HANDLE_T rec_venc_hdl;
    CVI_MAPI_VENC_HANDLE_T sub_rec_venc_hdl;
    CVI_MAPI_VENC_HANDLE_T thumbnail_venc_hdl;
    CVI_U32 thumbnail_bufsize;
    CVI_MAPI_VENC_HANDLE_T piv_venc_hdl;
    CVI_U32 piv_bufsize;
    CVI_S32 vproc_chn_id_venc;
    CVI_S32 sub_vproc_chn_id_venc;
    CVI_S32 vproc_chn_id_thumbnail;
    CVI_RECORD_SERVICE_VENC_BIND_MODE_E venc_bind_mode;
    volatile bool venc_rec_start;
}CVI_PHOTO_SERVICE_MAPI_HANDLES_S;

/* record attribute param */
typedef struct cviREC_ATTR_S {
    CVI_RECORD_SERVICE_HANDLE_T rs;
    CVI_RECORDER_TYPE_E enRecType; /* record type */
    union {
        CVI_RECORDER_NORMAL_ATTR_S stNormalRecAttr; /* normal record attribute */
        CVI_RECORDER_LAPSE_ATTR_S stLapseRecAttr;   /* lapse record attribute */
    } unRecAttr;

    CVI_RECORDER_SPLIT_ATTR_S stSplitAttr; /* record split attribute */
    CVI_U32 u32StreamCnt;                                       /*  stream cnt */
    CVI_RECORDER_STREAM_ATTR_S astStreamAttr[CVI_REC_STREAM_MAX_CNT]; /*  array of stream attr */
    CVI_U32 u32PreRecTimeSec;                                   /*  pre record time */
    CVI_U32 u32PostRecTimeSec;                                   /*  post record time */
    CVI_REC_CALLBACK_S stCallback;
    CVI_S32 s32RecPresize;
    CVI_S32 s32SnapPresize;
    CVI_S32 s32MemRecPreSec;
    CVI_BOOL enable_record_on_start;
    CVI_BOOL enable_perf_on_start;
    CVI_BOOL enable_debug_on_start;
    CVI_BOOL enable_subtitle;
    CVI_BOOL enable_thumbnail;
    CVI_BOOL enable_subvideo;
    CVI_S32 recorder_file_type;
#define CS_PARAM_MAX_FILENAME_LEN (128)
    CVI_CHAR recorder_save_dir_base[CS_PARAM_MAX_FILENAME_LEN];
    CVI_PHOTO_SERVICE_MAPI_HANDLES_S handles;
    CVI_FLOAT short_file_ms;
    char devmodel[32];
    char mntpath[32];
} CVI_REC_ATTR_T;

void *cvi_master_create(int32_t id, CVI_REC_ATTR_T *attr);
int32_t cvi_master_destroy(int32_t id);
int32_t cvi_master_update_attr(int32_t id, CVI_REC_ATTR_T *attr);
int32_t cvi_master_start_normal_rec(int32_t id);
int32_t cvi_master_stop_normal_rec(int32_t id);
int32_t cvi_master_start_lapse_rec(int32_t id);
int32_t cvi_master_stop_lapse_rec(int32_t id);
int32_t cvi_master_start_event_rec(int32_t id);
int32_t cvi_master_stop_event_rec(int32_t id);
int32_t cvi_master_start_mem_rec(int32_t id);
int32_t cvi_master_stop_mem_rec(int32_t id);
int32_t cvi_master_set_mute(int32_t id);
int32_t cvi_master_cancle_mute(int32_t id);
int32_t cvi_master_snap(int32_t id, char *file_name);
void cvi_master_waitsnap_finish(int32_t id);

#endif // __CS_VIDEO_H__
