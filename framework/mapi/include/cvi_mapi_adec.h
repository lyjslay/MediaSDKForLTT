#ifndef __CVI_MAPI_ADEC_H__
#define __CVI_MAPI_ADEC_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "cvi_mapi_define.h"

#include "cvi_comm_aio.h"
#include "cvi_comm_adec.h"

#include "cvi_mapi_aenc.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef CVI_MAPI_HANDLE_T CVI_MAPI_ADEC_HANDLE_T;

typedef struct cviMAPI_ADEC_ATTR_S
{
    CVI_MAPI_AUDIO_CODEC_E enAdecFormat;
    AUDIO_SOUND_MODE_E  enSoundmode;
    AUDIO_SAMPLE_RATE_E enSamplerate;
    ADEC_MODE_E enMode;
    int frame_size;
} CVI_MAPI_ADEC_ATTR_S;


typedef struct cviMAPI_AUDIO_FRAME_INFO_S
{
    AUDIO_FRAME_INFO_S stAudFrameInfo;
    void *pstream;
    uint32_t u32Len;
    uint64_t u64Timestamp;
} CVI_MAPI_AUDIO_FRAME_INFO_S;


int CVI_MAPI_ADEC_Init(CVI_MAPI_ADEC_HANDLE_T *AdecHdl, const CVI_MAPI_ADEC_ATTR_S* pstAdecAttr);
int CVI_MAPI_ADEC_Deinit(CVI_MAPI_ADEC_HANDLE_T AdecHdl);
int CVI_MAPI_ADEC_SendStream(CVI_MAPI_ADEC_HANDLE_T AdecHdl, const AUDIO_STREAM_S* pstAdecStream, int bBlock);
int CVI_MAPI_ADEC_SendEndOfStream(CVI_MAPI_ADEC_HANDLE_T AdecHdl);
int CVI_MAPI_ADEC_GetFrame(CVI_MAPI_ADEC_HANDLE_T AdecHdl, CVI_MAPI_AUDIO_FRAME_INFO_S* pstAudioFrameInfo, int bBlock);
int CVI_MAPI_ADEC_ReleaseFrame(CVI_MAPI_ADEC_HANDLE_T AdecHdl, const CVI_MAPI_AUDIO_FRAME_INFO_S* pstAudioFrameInfo);


#ifdef __cplusplus
}
#endif

#endif //__CVI_MAPI_ADEC_H__

