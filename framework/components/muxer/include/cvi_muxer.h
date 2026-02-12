#ifndef _CVI_MUXER_H_
#define _CVI_MUXER_H_


#include <stdint.h>
#include <stddef.h>

typedef enum cviTrack_VideoCodec_E
{
    CVI_MUXER_TRACK_VIDEO_CODEC_H264 = 96,
    CVI_MUXER_TRACK_VIDEO_CODEC_H265 = 98,
    CVI_MUXER_TRACK_VIDEO_CODEC_MJPEG = 102,
    CVI_MUXER_TRACK_VIDEO_CODEC_BUTT
} CVI_MUXER_TRACK_VIDEO_CODEC_E;


typedef enum cviTrack_AudioCodec_E
{
    CVI_MUXER_TRACK_AUDIO_CODEC_G711Mu  = 0,   /**< G.711 Mu           */
    CVI_MUXER_TRACK_AUDIO_CODEC_G711A   = 8,   /**< G.711 A            */
    CVI_MUXER_TRACK_AUDIO_CODEC_G726    = 97,   /**< G.726              */
    CVI_MUXER_TRACK_AUDIO_CODEC_AMR     = 101,   /**< AMR encoder format */
    CVI_MUXER_TRACK_AUDIO_CODEC_ADPCM  = 104,   /**< ADPCM              */
    CVI_MUXER_TRACK_AUDIO_CODEC_AAC = 105,
    CVI_MUXER_TRACK_AUDIO_CODEC_WAV  = 108,   /**< WAV encoder        */
    CVI_MUXER_TRACK_AUDIO_CODEC_MP3 = 109,
    CVI_MUXER_TRACK_AUDIO_CODEC_BUTT
} CVI_MUXER_TRACK_AUDIO_CODEC_E;


typedef struct cviCODEC_VIDEO_T{
    int32_t en;
    float framerate;
    CVI_MUXER_TRACK_VIDEO_CODEC_E codec;
    uint32_t w;
    uint32_t h;
}CVI_MUXER_CODEC_VIDEO_S;

typedef struct cviCODEC_AUDIO_T{
    int32_t en;
    CVI_MUXER_TRACK_AUDIO_CODEC_E codec;
    uint32_t samplerate;
    uint32_t chns;
    float framerate;
}CVI_MUXER_CODEC_AUDIO_S;

typedef struct cviCODEC_SUBTITLE_T{
    int32_t en;
    float framerate;
    uint32_t timebase;
}CVI_MUXER_CODEC_SUBTITLE_S;

typedef struct cviCODEC_THUMBNAIL_T{
    int32_t en;
    int32_t res;
}CVI_MUXER_CODEC_THUMBNAIL_S;

typedef enum CVI_MUXER_EVENT_E {
    CVI_MUXER_OPEN_FILE_FAILED,
    CVI_MUXER_SEND_FRAME_FAILED,
    CVI_MUXER_SEND_FRAME_TIMEOUT,
    CVI_MUXER_PTS_JUMP,
    CVI_MUXER_STOP,
    CVI_MUXER_EVENT_BUTT
} CVI_MUXER_EVENT_E;

typedef enum CVI_MUXER_FRAME_TYPE_E {
    CVI_MUXER_FRAME_TYPE_VIDEO,
    CVI_MUXER_FRAME_TYPE_SUB_VIDEO,
    CVI_MUXER_FRAME_TYPE_AUDIO,
    CVI_MUXER_FRAME_TYPE_SUBTITLE,
    CVI_MUXER_FRAME_TYPE_THUMBNAIL,
    CVI_MUXER_FRAME_TYPE_BUTT
} CVI_MUXER_FRAME_TYPE_E;

typedef struct cviFRAME_INFO_T{
    uint32_t hmagic;
    CVI_MUXER_FRAME_TYPE_E type;
    int32_t isKey;
    int32_t gopInx;
    int64_t pts;
    uint64_t vpts;
    int32_t dataLen;
    int32_t extraLen;
    int32_t totalSize;
    uint32_t tmagic;
}CVI_MUXER_FRAME_INFO_S;

typedef enum CVI_MUXER_VIDEO_TRACKIDX_E {
    CVI_MUXER_VIDEO_TRACK_IDX_0 = 0,
    CVI_MUXER_VIDEO_TRACK_IDX_1,
    CVI_MUXER_VIDEO_TRACK_IDX_BUTT
} CVI_MUXER_VIDEO_TRACKIDX_E;

typedef int32_t (*CVI_MUXER_EVENT_CALLBACK)(CVI_MUXER_EVENT_E event_type, const char *filename, void *p, void *extend);

typedef struct cviMUXER_ATTR_T{
    CVI_MUXER_CODEC_VIDEO_S stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_BUTT];
    CVI_MUXER_CODEC_AUDIO_S staudiocodec;
    CVI_MUXER_CODEC_SUBTITLE_S stsubtitlecodec;
    CVI_MUXER_CODEC_THUMBNAIL_S stthumbnailcodec;
    char *devmod;
    int32_t alignflag;
    int32_t presize;
    uint64_t u64SplitTimeLenMSec;
    CVI_MUXER_EVENT_CALLBACK pfncallback;
    void *pfnparam;
}CVI_MUXER_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t CVI_MUXER_Create(CVI_MUXER_ATTR_S attr, void **muxer);
int32_t CVI_MUXER_Start(void *muxer, const char *fname);
int32_t CVI_MUXER_WritePacket(void *muxer, CVI_MUXER_FRAME_TYPE_E type, CVI_MUXER_FRAME_INFO_S *packet);
void CVI_MUXER_Stop(void *muxer);
void CVI_MUXER_Destroy(void *muxer);
void CVI_MUXER_FlushPackets(void *muxer, int32_t flag);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#define CVI_MUXER_EXT_DATA_LEN 4
extern const unsigned char g_ext_audio_data[CVI_MUXER_EXT_DATA_LEN];


#endif
