#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "cvi_record_service.h"
#include "rs_master.h"
#include "cvi_log.h"

static CVI_REC_ATTR_T gst_cvi_rec_attr[MAX_CONTEXT_CNT];


static int32_t cvi_rs_get_rec_id(CVI_RECORD_SERVICE_HANDLE_T rs)
{
    for (int32_t i = 0; i < MAX_CONTEXT_CNT; i++) {
        if (gst_cvi_rec_attr[i].rs == rs) {
            return i;
        }
    }
    return MAX_CONTEXT_CNT;
}

static void CVI_APPATTR_2_RECORDATTR(CVI_RECORD_SERVICE_PARAM_S *aattr, CVI_REC_ATTR_T *rattr)
{
    CVI_LOGD("rec_mode = %d", aattr->rec_mode);

    if (aattr->timelapse_recorder_gop_interval == 0) {
        rattr->enRecType = CVI_RECORDER_TYPE_NORMAL;
    } else {
        rattr->enRecType = CVI_RECORDER_TYPE_LAPSE;
        rattr->unRecAttr.stLapseRecAttr.u32IntervalMs = aattr->timelapse_recorder_gop_interval * 1000;
        rattr->unRecAttr.stLapseRecAttr.fFramerate = aattr->timelapse_recorder_framerate;
    }

    if (aattr->recorder_split_interval_ms == 0) {
        rattr->stSplitAttr.enSplitType = CVI_RECORDER_SPLIT_TYPE_NONE;
    } else {
        rattr->stSplitAttr.enSplitType = CVI_RECORDER_SPLIT_TYPE_TIME;
        rattr->stSplitAttr.u64SplitTimeLenMSec = aattr->recorder_split_interval_ms;
    }
    CVI_LOGD("enRecType = %d, enSplitType %d, u64SplitTimeLenMSec = %"PRIu64"", rattr->enRecType, rattr->stSplitAttr.enSplitType, rattr->stSplitAttr.u64SplitTimeLenMSec);

    rattr->u32StreamCnt = 1;
    CVI_U32 u32StreamIdx = 0;
    CVI_U32 u32TrackCnt = 0;

    for (u32StreamIdx = 0; u32StreamIdx < rattr->u32StreamCnt; u32StreamIdx++) {
        CVI_RECORDER_TRACK_SOURCE_S *handle = &rattr->astStreamAttr[u32StreamIdx].aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_VIDEO];
        handle->enTrackType = CVI_RECORDER_TRACK_SOURCE_TYPE_VIDEO;
        handle->enable = 1;
        if (aattr->recorder_video_codec == CVI_RECORD_SERVICE_VIDEO_CODEC_H264) {
            handle->unTrackSourceAttr.stVideoInfo.enCodecType = CVI_MUXER_TRACK_VIDEO_CODEC_H264;
        } else if (aattr->recorder_video_codec == CVI_RECORD_SERVICE_VIDEO_CODEC_H265) {
            handle->unTrackSourceAttr.stVideoInfo.enCodecType = CVI_MUXER_TRACK_VIDEO_CODEC_H265;
        } else {
            handle->unTrackSourceAttr.stVideoInfo.enCodecType = CVI_MUXER_TRACK_VIDEO_CODEC_MJPEG;
        }
        handle->unTrackSourceAttr.stVideoInfo.u32Height = aattr->rec_height;
        handle->unTrackSourceAttr.stVideoInfo.u32Width = aattr->rec_width;
        handle->unTrackSourceAttr.stVideoInfo.u32BitRate = (CVI_U32)aattr->bitrate_kbps;
        handle->unTrackSourceAttr.stVideoInfo.fFrameRate = aattr->framerate;
        handle->unTrackSourceAttr.stVideoInfo.u32Gop = aattr->gop;
        handle->unTrackSourceAttr.stVideoInfo.fSpeed = aattr->framerate;
        u32TrackCnt++;

        CVI_RECORDER_TRACK_SOURCE_S *sub_handle = &rattr->astStreamAttr[u32StreamIdx].aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO];
        sub_handle->enTrackType = CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO;
        sub_handle->enable = aattr->enable_subvideo;
        if(sub_handle->enable == 1){
            if (aattr->sub_recorder_video_codec == CVI_RECORD_SERVICE_VIDEO_CODEC_H264) {
                sub_handle->unTrackSourceAttr.stVideoInfo.enCodecType = CVI_MUXER_TRACK_VIDEO_CODEC_H264;
            } else if (aattr->sub_recorder_video_codec == CVI_RECORD_SERVICE_VIDEO_CODEC_H265) {
                sub_handle->unTrackSourceAttr.stVideoInfo.enCodecType = CVI_MUXER_TRACK_VIDEO_CODEC_H265;
            } else {
                sub_handle->unTrackSourceAttr.stVideoInfo.enCodecType = CVI_MUXER_TRACK_VIDEO_CODEC_MJPEG;
            }
            sub_handle->unTrackSourceAttr.stVideoInfo.u32Height = aattr->sub_rec_height;
            sub_handle->unTrackSourceAttr.stVideoInfo.u32Width = aattr->sub_rec_width;
            sub_handle->unTrackSourceAttr.stVideoInfo.u32BitRate = aattr->sub_bitrate_kbps;
            sub_handle->unTrackSourceAttr.stVideoInfo.fFrameRate = aattr->sub_framerate;
            sub_handle->unTrackSourceAttr.stVideoInfo.u32Gop = aattr->sub_gop;
            sub_handle->unTrackSourceAttr.stVideoInfo.fSpeed = aattr->sub_framerate;
            u32TrackCnt++;
        }

        handle = &rattr->astStreamAttr[u32StreamIdx].aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_AUDIO];
        handle->enTrackType = CVI_RECORDER_TRACK_SOURCE_TYPE_AUDIO;
        handle->enable = aattr->audio_recorder_enable;
        if (rattr->enRecType == CVI_RECORDER_TYPE_LAPSE) {
            handle->enable = 0;
        }
        // }else{
        //     handle->enable = 1;
        // }
        CVI_LOGD("enable %d, enRecType %d", handle->enable, rattr->enRecType);
        if (aattr->recorder_audio_codec == CVI_RECORD_SERVICE_AUDIO_CODEC_PCM) {
            handle->unTrackSourceAttr.stAudioInfo.enCodecType = CVI_MUXER_TRACK_AUDIO_CODEC_ADPCM;
        } else if (aattr->recorder_audio_codec == CVI_RECORD_SERVICE_AUDIO_CODEC_AAC) {
            handle->unTrackSourceAttr.stAudioInfo.enCodecType = CVI_MUXER_TRACK_AUDIO_CODEC_AAC;
        } else {
            /*do*/
        }
        handle->unTrackSourceAttr.stAudioInfo.u32SampleRate = aattr->audio_sample_rate;
        handle->unTrackSourceAttr.stAudioInfo.u32ChnCnt = aattr->audio_channels;
        handle->unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame = aattr->audio_num_per_frame;
        u32TrackCnt++;

        handle = &rattr->astStreamAttr[u32StreamIdx].aHTrackSrcHandle[CVI_RECORDER_TRACK_SOURCE_TYPE_PRIV];
        handle->enTrackType = CVI_RECORDER_TRACK_SOURCE_TYPE_PRIV;
        handle->enable = 1;
        u32TrackCnt++;
        rattr->astStreamAttr[u32StreamIdx].u32TrackCnt = u32TrackCnt;
    }

    rattr->handles.piv_venc_hdl = aattr->piv_venc_hdl;
    rattr->handles.piv_bufsize = aattr->piv_bufsize;

    rattr->handles.rec_vproc = aattr->rec_vproc;
    rattr->handles.thumbnail_vproc = aattr->thumbnail_vproc;
    rattr->handles.thumbnail_venc_hdl = aattr->thumbnail_venc_hdl;
    rattr->handles.thumbnail_bufsize = aattr->thumbnail_bufsize;

    rattr->handles.rec_venc_hdl = aattr->rec_venc_hdl;

    rattr->enable_subvideo = aattr->enable_subvideo;
    rattr->handles.sub_rec_venc_hdl = aattr->sub_rec_venc_hdl;
    rattr->handles.sub_rec_vproc = aattr->sub_rec_vproc;
    rattr->handles.sub_vproc_chn_id_venc = aattr->sub_vproc_chn_id_venc;

    rattr->handles.vproc_chn_id_thumbnail = aattr->vproc_chn_id_thumbnail;
    rattr->handles.vproc_chn_id_venc = aattr->vproc_chn_id_venc;
    rattr->handles.venc_bind_mode = aattr->venc_bind_mode;
    rattr->handles.venc_rec_start = 0;

    rattr->stCallback.pfnRequestFileNames = aattr->generate_filename_cb;
    rattr->stCallback.pfnNormalRecCb = aattr->cont_recorder_event_cb;
    rattr->stCallback.pfnEventRecCb = aattr->event_recorder_event_cb;
    rattr->stCallback.pfnLapseRecCb = aattr->timelapse_recorder_event_cb;
    rattr->stCallback.pfnGetSubtitleCb = aattr->get_subtitle_cb;
    rattr->stCallback.pfnGetDirType = aattr->get_rec_dir_type_cb;
    rattr->stCallback.pfnGetGPSInfoCb = aattr->get_gps_info_cb;
    rattr->s32RecPresize = aattr->pre_alloc_unit;
    rattr->s32SnapPresize = aattr->prealloclen;
    rattr->s32MemRecPreSec = aattr->memory_buffer_sec;
    rattr->enable_debug_on_start = aattr->enable_debug_on_start;
    rattr->enable_record_on_start = aattr->enable_record_on_start;
    rattr->enable_perf_on_start = aattr->enable_perf_on_start;
    rattr->enable_subtitle = aattr->enable_subtitle;
    if (rattr->enRecType == CVI_RECORDER_TYPE_LAPSE) {
        rattr->enable_subtitle = 0;
    }
    rattr->enable_thumbnail = aattr->enable_thumbnail;
    rattr->recorder_file_type = aattr->recorder_file_type;
    strncpy(rattr->recorder_save_dir_base, aattr->recorder_save_dir_base, CS_PARAM_MAX_FILENAME_LEN - 1);
    rattr->u32PreRecTimeSec = aattr->event_recorder_pre_recording_sec;
    rattr->u32PostRecTimeSec = aattr->event_recorder_post_recording_sec;
    rattr->short_file_ms = aattr->short_file_ms;
    strncpy(rattr->devmodel, aattr->devmodel, sizeof(rattr->devmodel) - 1);
    strncpy(rattr->mntpath, aattr->mntpath, strlen(aattr->mntpath));
}


int32_t CVI_RECORD_SERVICE_Create(CVI_RECORD_SERVICE_HANDLE_T *hdl, CVI_RECORD_SERVICE_PARAM_S *param) {
    CVI_REC_ATTR_T *attr = &gst_cvi_rec_attr[param->recorder_id];
    CVI_APPATTR_2_RECORDATTR(param, attr);
    attr->rs = cvi_master_create(param->recorder_id, attr);
    *hdl = attr->rs;
    return (*hdl != NULL) ? 0 : -1;
}

int32_t CVI_RECORD_SERVICE_Destroy(CVI_RECORD_SERVICE_HANDLE_T hdl) {
    return cvi_master_destroy(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_UpdateParam(CVI_RECORD_SERVICE_HANDLE_T hdl, CVI_RECORD_SERVICE_PARAM_S *param) {
    if (!param) {
        CVI_LOGE("Input recorder parameter is NULL");
        return -1;
    }

    CVI_REC_ATTR_T *attr = &gst_cvi_rec_attr[param->recorder_id];
    CVI_APPATTR_2_RECORDATTR(param, attr);
    return cvi_master_update_attr(cvi_rs_get_rec_id(hdl), attr);
}

int32_t CVI_RECORD_SERVICE_StartRecord(CVI_RECORD_SERVICE_HANDLE_T hdl) {
    if (!hdl) {
        return -1;
    }
    return cvi_master_start_normal_rec(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_StopRecord(CVI_RECORD_SERVICE_HANDLE_T hdl)
{
    if (!hdl) {
        return -1;
    }
    return cvi_master_stop_normal_rec(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_StartTimelapseRecord(CVI_RECORD_SERVICE_HANDLE_T hdl)
{
    if (!hdl) {
        return -1;
    }
    return cvi_master_start_lapse_rec(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_StopTimelapseRecord(CVI_RECORD_SERVICE_HANDLE_T hdl)
{
    if (!hdl) {
        return -1;
    }
    return cvi_master_stop_lapse_rec(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_EventRecord(CVI_RECORD_SERVICE_HANDLE_T hdl)
{
    if (!hdl) {
        return -1;
    }
    return cvi_master_start_event_rec(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_StopEventRecord(CVI_RECORD_SERVICE_HANDLE_T hdl)
{
    if (!hdl) {
        return -1;
    }
    return cvi_master_stop_event_rec(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_StartMute(CVI_RECORD_SERVICE_HANDLE_T hdl)
{
    if (!hdl) {
        return -1;
    }
    return cvi_master_set_mute(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_StopMute(CVI_RECORD_SERVICE_HANDLE_T hdl)
{
    if (!hdl) {
        return -1;
    }
    return cvi_master_cancle_mute(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_PivCapture(CVI_RECORD_SERVICE_HANDLE_T hdl, char *file_name)
{
    if (!hdl) {
        return -1;
    }
    return cvi_master_snap(cvi_rs_get_rec_id(hdl), file_name);
}

void CVI_RECORD_SERVICE_WaitPivFinish(CVI_RECORD_SERVICE_HANDLE_T hdl)
{
    if (!hdl) {
        CVI_LOGE("record service handle is null !\n");
        return;
    }
    cvi_master_waitsnap_finish(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_StartMemoryBuffer(CVI_RECORD_SERVICE_HANDLE_T hdl) {
    if (!hdl) {
        return -1;
    }
    return cvi_master_start_mem_rec(cvi_rs_get_rec_id(hdl));
}

int32_t CVI_RECORD_SERVICE_StopMemoryBuffer(CVI_RECORD_SERVICE_HANDLE_T hdl) {
    if (!hdl) {
        return -1;
    }
    return cvi_master_stop_mem_rec(cvi_rs_get_rec_id(hdl));
}





