#ifndef __CVI_MAPI_AENC_H__
#define __CVI_MAPI_AENC_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "cvi_mapi_define.h"

#include "cvi_comm_aio.h"
#include "cvi_mapi_acap.h"
#include "cvi_comm_adec.h"
#include "cvi_comm_aenc.h"
#include "cvi_osal.h"
#include <semaphore.h>
#define FILE_NAME_LEN 128
//#define AUDIO_BUFFER_MAX  (10*60 * 1024)
#ifdef __cplusplus
extern "C"
{
#endif

typedef CVI_MAPI_HANDLE_T CVI_MAPI_AENC_HANDLE_T;

typedef enum cviMAPI_AUDIO_CODEC_E
{
    CVI_MAPI_AUDIO_CODEC_AAC = 0, /* AAC format */
    CVI_MAPI_AUDIO_CODEC_G711A, /* G711A format */
    CVI_MAPI_AUDIO_CODEC_G711U, /* G711U format */
    CVI_MAPI_AUDIO_CODEC_BUTT
}CVI_MAPI_AUDIO_CODEC_E;

typedef int (*CVI_MAPI_AENC_DATAPROC_CALLBACK_FN_PTR)(CVI_MAPI_AENC_HANDLE_T AencHdl,
    AUDIO_STREAM_S* pAStreamData, void *pPrivateData);


typedef struct cviMAPI_AENC_CALLBACK_S
{
    CVI_MAPI_AENC_DATAPROC_CALLBACK_FN_PTR pfnDataCB;
    void *pPrivateData; /** private data */
} CVI_MAPI_AENC_CALLBACK_S;


typedef struct cviMAPI_AENC_ATTR_S
{
    CVI_MAPI_AUDIO_CODEC_E enAencFormat; /**< audio encode format type*/
    uint32_t u32PtNumPerFrm; /**< sampling point number per frame,the same as acap */
    uint32_t src_samplerate;
    uint32_t channels;
} CVI_MAPI_AENC_ATTR_S;

typedef struct _AENC_AAC_INFO_S {
    CVI_U32 sample_rate;
    CVI_S32 bit_rate;//0-32
    int32_t i32FrameSize;
    char *pOutputBuf;
}AENC_AAC_INFO_S;

int CVI_MAPI_AENC_Init(CVI_MAPI_AENC_HANDLE_T *AencHdl, const CVI_MAPI_AENC_ATTR_S *pstAencAttr);
int CVI_MAPI_AENC_Deinit(CVI_MAPI_AENC_HANDLE_T AencHdl);
int CVI_MAPI_AENC_Start(CVI_MAPI_AENC_HANDLE_T AencHdl);
int CVI_MAPI_AENC_Stop(CVI_MAPI_AENC_HANDLE_T AencHdl);
int CVI_MAPI_AENC_SetMute(CVI_MAPI_AENC_HANDLE_T AencHdl, int bEnable);
int CVI_MAPI_AENC_GetMute(CVI_MAPI_AENC_HANDLE_T AencHdl, int *pbEnable);
int CVI_MAPI_AENC_RegisterCallback(CVI_MAPI_AENC_HANDLE_T AencHdl, const CVI_MAPI_AENC_CALLBACK_S* pstAencCB);
int CVI_MAPI_AENC_UnregisterCallback(CVI_MAPI_AENC_HANDLE_T AencHdl, const CVI_MAPI_AENC_CALLBACK_S* pstAencCB);
int CVI_MAPI_AENC_BindACap(AUDIO_DEV AiDev, AI_CHN AiChn,AUDIO_DEV AeDev, AENC_CHN AeChn);
int CVI_MAPI_AENC_UnbindACap(AUDIO_DEV AiDev, AI_CHN AiChn,AUDIO_DEV AeDev, AENC_CHN AeChn);
int CVI_MAPI_AENC_SendFrame(CVI_MAPI_AENC_HANDLE_T AencHdl, const AUDIO_FRAME_S* pstFrm,AEC_FRAME_S* pstAecFrm);
int CVI_MAPI_AENC_GetStream(CVI_MAPI_AENC_HANDLE_T AencHdl, AUDIO_STREAM_S *pstStream,CVI_S32 s32MilliSec);

#ifdef __cplusplus
}
#endif

#endif //__CVI_MAPI_AENC_H__


