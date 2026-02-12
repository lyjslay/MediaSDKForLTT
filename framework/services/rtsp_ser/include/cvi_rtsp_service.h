
#ifndef _CVI_RTSP_SER_API_H_
#define _CVI_RTSP_SER_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cvi_mapi.h"
#include "cvi_mapi_aenc.h"

typedef enum {
    CVI_RTSP_SERVICE_VIDEO_CODEC_H264 = 0,
    CVI_RTSP_SERVICE_VIDEO_CODEC_H265,
    CVI_RTSP_SERVICE_VIDEO_CODEC_JPEG
} CVI_RTSP_SERVICE_VIDEO_CODEC_E;

typedef enum {
    CVI_RTSP_SERVICE_AUDIO_CODEC_NONE,
    CVI_RTSP_SERVICE_AUDIO_CODEC_PCM,
    CVI_RTSP_SERVICE_AUDIO_CODEC_AAC
} CVI_RTSP_SERVICE_AUDIO_CODEC_E;

#define MAX_RTSP_STREAM_NAME_LEN (32)
typedef void(CVI_RTSP_SERVICE_CALLBACK) (int32_t references, void *arg);

typedef struct {
    int32_t recorder_id;
    char rtsp_name[MAX_RTSP_STREAM_NAME_LEN];
    int32_t max_conn;
    int32_t timeout;
    int32_t port;
    CVI_RTSP_SERVICE_VIDEO_CODEC_E video_codec;
    CVI_RTSP_SERVICE_AUDIO_CODEC_E audio_codec;
    CVI_RTSP_SERVICE_CALLBACK *rtsp_play;
    void *rtsp_play_arg;
    CVI_RTSP_SERVICE_CALLBACK *rtsp_teardown;
    void *rtsp_teardown_arg;

    uint32_t width;
    uint32_t height;
    float framerate;
    int32_t bitrate_kbps;
    int32_t audio_sample_rate;
    int32_t audio_channels;
    int32_t audio_pernum;

    int32_t chn_id;
    CVI_MAPI_VPROC_HANDLE_T vproc;
    CVI_MAPI_VENC_HANDLE_T venc_hdl;
    CVI_MAPI_ACAP_HANDLE_T acap_hdl;
    CVI_MAPI_AENC_HANDLE_T aenc_hdl;
} CVI_RTSP_SERVICE_PARAM_S;



typedef void *CVI_RTSP_SERVICE_HANDLE_T;

int32_t CVI_RTSP_SERVICE_Create(CVI_RTSP_SERVICE_HANDLE_T *hdl, CVI_RTSP_SERVICE_PARAM_S *param);
int32_t CVI_RTSP_SERVICE_Destroy(CVI_RTSP_SERVICE_HANDLE_T hdl);
int32_t CVI_RTSP_SERVICE_UpdateParam(CVI_RTSP_SERVICE_HANDLE_T hdl, CVI_RTSP_SERVICE_PARAM_S *param);
int32_t CVI_RTSP_SERVICE_StartMute(CVI_RTSP_SERVICE_HANDLE_T hdl);
int32_t CVI_RTSP_SERVICE_StopMute(CVI_RTSP_SERVICE_HANDLE_T hdl);
int32_t CVI_RTSP_SERVICE_StartStop(uint32_t value, char *name);


#ifdef __cplusplus
}
#endif

#endif


