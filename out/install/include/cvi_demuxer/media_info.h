#pragma once

#include <stdint.h>

#define CVI_DEMUXER_STREAM_MAX_NUM 5

typedef struct CviDemuxerStreamResolution {
    int32_t stream_index;
    uint32_t width;
    uint32_t height;
    char codec[16];
} CVI_DEMUXER_STREAM_RESOLUTION_S;

typedef struct CviDemuxerMediaInfo {
    char file_name[64];
    char format[16];
    int32_t width;
    int32_t height;
    uint64_t file_size;
    double start_time_sec;
    double duration_sec;
    double audio_duration_sec;
    double video_duration_sec;
    CVI_DEMUXER_STREAM_RESOLUTION_S stream_resolution[CVI_DEMUXER_STREAM_MAX_NUM];
    int32_t used_video_stream_index;
    float frame_rate;
    uint64_t bit_rate;
    uint32_t audio_channel_layout;
    uint32_t sample_rate;
    int32_t used_audio_stream_index;
    char video_codec[16];
    char audio_codec[16];
} CVI_DEMUXER_MEDIA_INFO_S;

typedef struct CviDemuxerstreaminfo {
    int32_t videonum;
    int32_t videoden;
    int32_t audionum;
    int32_t audioden;
    int32_t videowidth;
    int32_t videoheight;
    float frame_rate;
    int64_t duration;
} CVI_DEMUXER_STREAM_INFO_S;
