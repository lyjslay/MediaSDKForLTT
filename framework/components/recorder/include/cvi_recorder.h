#ifndef _CVI_RECORDER_H_
#define _CVI_RECORDER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "cvi_muxer.h"

typedef void *CVI_RECORDER_HANDLE_T;

typedef enum CVI_RECORDER_EVENT_E
{
    CVI_RECORDER_EVENT_START,
    CVI_RECORDER_EVENT_STOP,
    CVI_RECORDER_EVENT_STOP_FAILED,
    CVI_RECORDER_EVENT_SPLIT,
    CVI_RECORDER_EVENT_WRITE_FRAME_DROP,
    CVI_RECORDER_EVENT_WRITE_FRAME_TIMEOUT,
    CVI_RECORDER_EVENT_WRITE_FRAME_FAILED,
    CVI_RECORDER_EVENT_OPEN_FILE_FAILED,
    CVI_RECORDER_EVENT_CLOSE_FILE_FAILED,
    CVI_RECORDER_EVENT_SHORT_FILE,
    CVI_RECORDER_EVENT_PIV_START,
    CVI_RECORDER_EVENT_PIV_END,
    CVI_RECORDER_EVENT_SYNC_DONE,
    CVI_RECORDER_EVENT_SPLIT_START,
    CVI_RECORDER_EVENT_START_EMR,
    CVI_RECORDER_EVENT_END_EMR,
    CVI_RECORDER_EVENT_FRAME_DROP,
    CVI_RECORDER_EVENT_BUTT
} CVI_RECORDER_EVENT_E;

typedef enum _cvi_RECORDER_PTS_STATE_E {
    CVI_RECORDER_PTS_STATE_INIT,
    CVI_RECORDER_PTS_STATE_SETTED,
    CVI_RECORDER_PTS_STATE_CHECKED,
    CVI_RECORDER_PTS_STATE_BUTT
} CVI_RECORDER_PTS_STATE_E;

typedef struct _cvi_RECORDER_EVENT_WRITE_FRAME_TIMEOUT_S{
    int32_t timeout_ms;
    void *param;
} CVI_RECORDER_EVENT_WRITE_FRAME_TIMEOUT_S;

typedef int32_t (*CVI_RECORDER_GET_FILENAME_CALLBACK)(void *p, char *filename, int32_t filename_len);
typedef int32_t (*CVI_RECORDER_EVENT_CALLBACK)(CVI_RECORDER_EVENT_E event_type, const char *filename, void *p);
typedef int32_t (*CVI_RECORDER_GET_SUBTITLE_CALLBACK)(void *p, int32_t viPipe, char *str, int32_t str_len);
typedef int32_t (*CVI_RECORDER_GET_MEM_BUFFER_STOP_CALLBACK)(void *p);
typedef int32_t (*CVI_RECORDER_REQUEST_IDR_CALLBACK)(void *p, CVI_MUXER_FRAME_TYPE_E type);
typedef int32_t (*CVI_RECORDER_STOP_CALLBACK)(void *p);
typedef int32_t (*CVI_RECORDER_GET_DIR_TYPE_CALLBACK)(int32_t id, int32_t base);

typedef enum CVI_CALLBACK_TYPE{
    CVI_RECORDER_CALLBACK_TYPE_NORMAL = 0,
    CVI_RECORDER_CALLBACK_TYPE_LAPSE,
    CVI_RECORDER_CALLBACK_TYPE_EVENT,
    CVI_RECORDER_CALLBACK_TYPE_BUTT
}CVI_RECORDER_CALLBACK_TYPE_E;

typedef enum CVI_RECORDER_TYPE_INDEX_E{
    CVI_RECORDER_TYPE_NORMAL_INDEX = 0,
    CVI_RECORDER_TYPE_LAPSE_INDEX = 0,
    CVI_RECORDER_TYPE_EVENT_INDEX = 1,
    CVI_RECORDER_TYPE_BUTT_INDEX = 2
}CVI_RECORDER_TYPE_INDEX_E;


typedef struct CVI_CALLBACK_HANDLES_S{
    CVI_RECORDER_GET_SUBTITLE_CALLBACK pfn_get_subtitle_cb;
    void *pfn_get_subtitle_cb_param;
    CVI_RECORDER_GET_FILENAME_CALLBACK pfn_get_filename;
    void *pfn_get_filename_param[CVI_RECORDER_CALLBACK_TYPE_BUTT];
    CVI_RECORDER_REQUEST_IDR_CALLBACK pfn_request_idr;
    void *pfn_request_idr_param;
    CVI_RECORDER_EVENT_CALLBACK pfn_event_cb[CVI_RECORDER_TYPE_BUTT_INDEX]; /*normal && lapse share*/
    void *pfn_event_cb_param;
    CVI_RECORDER_GET_MEM_BUFFER_STOP_CALLBACK pfn_mem_buffer_stop_cb;
    void *pfn_mem_buffer_stop_cb_param;
    CVI_RECORDER_STOP_CALLBACK pfn_rec_stop_cb;
    void *pfn_rec_malloc_mem;
    void *pfn_rec_free_mem;
    void *pfn_rec_stop_cb_param;
}CVI_RECORDER_CB_HANDLES_S;

typedef enum CVI_RECORDER_RBUF_TYPE_E{
    CVI_RECORDER_RBUF_VIDEO = 0,
    CVI_RECORDER_RBUF_SUB_VIDEO,
    CVI_RECORDER_RBUF_AUDIO,
    CVI_RECORDER_RBUF_SUBTITLE,
    CVI_RECORDER_RBUF_BUTT
}CVI_RECORDER_RBUF_TYPE_E;


typedef struct CVI_RECORDER_RBUF_ATTR_S{
    uint32_t size;
    const char *name;
}CVI_RECORDER_RBUF_ATTR_S;


#define CVI_RECORDER_TRACK_MAX_CNT (CVI_RECORDER_TRACK_SOURCE_TYPE_BUTT)

typedef enum cviTrack_SourceType_E {
    CVI_RECORDER_TRACK_SOURCE_TYPE_VIDEO = 0,
    CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO,
    CVI_RECORDER_TRACK_SOURCE_TYPE_AUDIO,
    CVI_RECORDER_TRACK_SOURCE_TYPE_PRIV,
    CVI_RECORDER_TRACK_SOURCE_TYPE_BUTT
} CVI_RECORDER_TRACK_SOURCE_TYPE_E;


/* record type enum */
typedef enum cviREC_TYPE_E {
    CVI_RECORDER_TYPE_NORMAL = 0, /* normal record */
    CVI_RECORDER_TYPE_LAPSE,      /* time lapse record, record a frame by an fixed time interval */
    CVI_RECORDER_TYPE_BUTT
} CVI_RECORDER_TYPE_E;

#define CVI_REC_STREAM_MAX_CNT (4)

/* splite define */
/* record split type enum */
typedef enum cviREC_SPLIT_TYPE_E {
    CVI_RECORDER_SPLIT_TYPE_NONE = 0, /* means split is disabled */
    CVI_RECORDER_SPLIT_TYPE_TIME,     /* record split when time reaches */
    CVI_RECORDER_SPLIT_TYPE_BUTT
} CVI_RECORDER_SPLIT_TYPE_E;

/* record split attribute param */
typedef struct cviREC_SPLIT_ATTR_S {
    CVI_RECORDER_SPLIT_TYPE_E enSplitType; /* split type */
    uint64_t u64SplitTimeLenMSec;       /* split time, unit in msecond(ms) */
} CVI_RECORDER_SPLIT_ATTR_S;


/* normal record attribute param */
typedef struct cviREC_NORMAL_ATTR_S {
    uint32_t u32Rsv; /* reserve */
} CVI_RECORDER_NORMAL_ATTR_S;

/* lapse record attribute param */
typedef struct cviREC_LAPSE_ATTR_S {
    uint32_t u32IntervalMs; /* lapse record time interval, unit in millisecord(ms) */
    float fFramerate;
} CVI_RECORDER_LAPSE_ATTR_S;


typedef struct cviTrack_VideoSourceInfo_S {
    CVI_MUXER_TRACK_VIDEO_CODEC_E enCodecType;
    uint32_t u32Width;
    uint32_t u32Height;
    uint32_t u32BitRate;
    float fFrameRate;
    uint32_t u32Gop;
    float fSpeed;
} CVI_RECORDER_TRACK_VideoSourceInfo_S;

typedef struct cviTrack_AudioSourceInfo_S {
    CVI_MUXER_TRACK_AUDIO_CODEC_E enCodecType;
    uint32_t u32ChnCnt;
    uint32_t u32SampleRate;
    uint32_t u32AvgBytesPerSec;
    uint32_t u32SamplesPerFrame;
    unsigned short u16SampleBitWidth;
    float fFramerate;
} CVI_RECORDER_TRACK_AudioSourceInfo_S;

typedef struct cviTrack_PrivateSourceInfo_S
{
    uint32_t u32PrivateData;
    uint32_t u32FrameRate;
    uint32_t u32BytesPerSec;
    int32_t bStrictSync;
} CVI_RECORDER_TRACK_PrivateSourceInfo_S;

typedef struct cviTrack_Source_S
{
    CVI_RECORDER_TRACK_SOURCE_TYPE_E enTrackType;
    int32_t enable;
    union
    {
        CVI_RECORDER_TRACK_VideoSourceInfo_S stVideoInfo;
        CVI_RECORDER_TRACK_AudioSourceInfo_S stAudioInfo;
        CVI_RECORDER_TRACK_PrivateSourceInfo_S stPrivInfo;
    } unTrackSourceAttr;
}CVI_RECORDER_TRACK_SOURCE_S;


/* record stream attribute */
typedef struct cviREC_STREAM_ATTR_S {
    uint32_t u32TrackCnt;                                            /* track cnt */
    CVI_RECORDER_TRACK_SOURCE_S aHTrackSrcHandle[CVI_RECORDER_TRACK_MAX_CNT]; /* array of track source cnt */
} CVI_RECORDER_STREAM_ATTR_S;


typedef struct CVI_RECORDER_ATTR_S {
    CVI_RECORDER_STREAM_ATTR_S astStreamAttr;
    CVI_RECORDER_CB_HANDLES_S fncallback;
    CVI_RECORDER_TYPE_E enRecType; /* record type */
    union {
        CVI_RECORDER_NORMAL_ATTR_S stNormalRecAttr; /* normal record attribute */
        CVI_RECORDER_LAPSE_ATTR_S stLapseRecAttr;   /* lapse record attribute */
    } unRecAttr;
    CVI_RECORDER_RBUF_ATTR_S stRbufAttr[CVI_RECORDER_RBUF_BUTT];
    CVI_RECORDER_SPLIT_ATTR_S stSplitAttr; /* record split attribute */
    int32_t enable_subtitle;
    int32_t enable_thumbnail;
    int32_t enable_subvideo;
    int32_t enable_file_alignment;
    int32_t enable_emrfile_from_normfile;
    float subtitle_framerate;
    uint32_t u32PreRecTimeSec;                                   /*  pre record time */
    uint32_t u32PostRecTimeSec;                                   /*  post record time */
    int32_t s32MemRecPreSec;
    char *device_model;
    int32_t prealloc_size;
    float short_file_ms;
    int32_t id;
} CVI_RECORDER_ATTR_S;

#define CVI_FRAME_STREAM_SEGMENT_MAX_NUM (8)
#define CVI_SUBTITLE_MAX_LEN (200)
#define CVI_SEND_FRAME_TIMEOUT_MS (1000)

typedef struct CVI_FRAME_STREAM_S {
    CVI_MUXER_FRAME_TYPE_E type;
    bool vftype[CVI_FRAME_STREAM_SEGMENT_MAX_NUM];
    int32_t num;
    uint64_t vi_pts[CVI_FRAME_STREAM_SEGMENT_MAX_NUM];
    unsigned char *data[CVI_FRAME_STREAM_SEGMENT_MAX_NUM];
    size_t len[CVI_FRAME_STREAM_SEGMENT_MAX_NUM];
    unsigned char *thumbnail_data;
    size_t thumbnail_len;
} CVI_RECORDER_FRAME_STREAM_S;


int32_t CVI_RECORDER_SendFrame(void *recorder, CVI_RECORDER_FRAME_STREAM_S *frame);
int32_t CVI_RECORDER_Start_MemRec(void *recorder);
int32_t CVI_RECORDER_Stop_MemRec(void *recorder);
int32_t CVI_RECORDER_Start_NormalRec(void *recorder);
int32_t CVI_RECORDER_Stop_NormalRec(void *recorder);
int32_t CVI_RECORDER_Start_EventRec(void *recorder);
int32_t CVI_RECORDER_Stop_EventRec(void *recorder);
int32_t CVI_RECORDER_ForceStop_EventRec(void *recorder);
int32_t CVI_RECORDER_Stop_EventRecPost(void *recorder);
int32_t CVI_RECORDER_Start_LapseRec(void *recorder);
int32_t CVI_RECORDER_Stop_LapseRec(void *recorder);
void CVI_RECORDER_Destroy(void **recorder);
int32_t CVI_RECORDER_Create(void **recorder, CVI_RECORDER_ATTR_S *attr);
int32_t CVI_RECORDER_Split(void *recorder);
int32_t CVI_RECORDER_Timelapse_Is_SendVenc(void *recorder,CVI_MUXER_FRAME_TYPE_E type); /*new*/

uint64_t CVI_RECORDER_GetUs(void);


#ifdef __cplusplus
}
#endif
#endif
