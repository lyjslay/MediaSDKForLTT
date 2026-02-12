#ifndef __CVI_MAPI_ACAP_H__
#define __CVI_MAPI_ACAP_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "cvi_mapi_define.h"
#include "cvi_comm_aio.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef CVI_MAPI_HANDLE_T CVI_MAPI_ACAP_HANDLE_T;

#define CVI_MAPI_ACAP_DEV_MAX_NUM (1)

typedef enum cviAUDIO_CAP_ERROR_E
{
    AUDIO_ACAP_OK=0,
    AUDIO_ACAP_ERROR_FAILURE=0xFFFF0001,
    AUDIO_ACAP_ERROR_INVALID_PARAMS,
    AUDIO_ACAP_ERROR_NULL_POINTER,
    AUDIO_ACAP_ERROR_BUTT,
} AUDIO_CAP_ERROR_E;

typedef struct cviMAPI_ACAP_ATTR_S
{
    AUDIO_SAMPLE_RATE_E enSampleRate;
    uint32_t channel;
    uint32_t u32PtNumPerFrm;//frames unit
    int bVqeOn;
    uint32_t AudioChannel;
    int32_t volume;
} CVI_MAPI_ACAP_ATTR_S;


int CVI_MAPI_ACAP_Init(CVI_MAPI_ACAP_HANDLE_T *AcapHdl,const CVI_MAPI_ACAP_ATTR_S *pstACapAttr);
int CVI_MAPI_ACAP_Deinit(CVI_MAPI_ACAP_HANDLE_T AcapHdl);
int CVI_MAPI_ACAP_EnableRecordVqe (CVI_MAPI_ACAP_HANDLE_T AcapHdl);
int CVI_MAPI_ACAP_EnableVqe(CVI_MAPI_ACAP_HANDLE_T AcapHdl);
int CVI_MAPI_ACAP_DisableVqe(CVI_MAPI_ACAP_HANDLE_T AcapHdl);
int CVI_MAPI_ACAP_Start(CVI_MAPI_ACAP_HANDLE_T AcapHdl);
int CVI_MAPI_ACAP_Stop(CVI_MAPI_ACAP_HANDLE_T AcapHdl);
int CVI_MAPI_ACAP_EnableReSmp(CVI_MAPI_ACAP_HANDLE_T AcapHdl, AUDIO_SAMPLE_RATE_E enOutSampleRate);
int CVI_MAPI_ACAP_DisableReSmp(CVI_MAPI_ACAP_HANDLE_T AcapHdl);
int CVI_MAPI_ACAP_SetVolume(CVI_MAPI_ACAP_HANDLE_T AcapHdl,int s32AudioGain);
int CVI_MAPI_ACAP_GetVolume(CVI_MAPI_ACAP_HANDLE_T AcapHdl,int *ps32AudioGain);
int CVI_MAPI_ACAP_Mute(CVI_MAPI_ACAP_HANDLE_T AcapHdl);
int CVI_MAPI_ACAP_Unmute(CVI_MAPI_ACAP_HANDLE_T AcapHdl);
int CVI_MAPI_ACAP_GetFrame(CVI_MAPI_ACAP_HANDLE_T AcapHdl,AUDIO_FRAME_S *pstFrm, AEC_FRAME_S *pstAecFrm);
int CVI_MAPI_ACAP_ReleaseFrame(CVI_MAPI_ACAP_HANDLE_T AcapHdl,const AUDIO_FRAME_S *pstFrm, const AEC_FRAME_S *pstAecFrm);


#ifdef __cplusplus
}
#endif

#endif //__CVI_MAPI_ACAP_H__

