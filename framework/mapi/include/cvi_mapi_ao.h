#ifndef __CVI_MAPI_AO_H__
#define __CVI_MAPI_AO_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdio.h"
#include "cvi_mapi_define.h"
#include "cvi_comm_aio.h"
#include "cvi_comm_adec.h"
#include "cvi_osal.h"
#include "cvi_mapi_adec.h"
#define CVI_MAPI_TRUE  1
#define CVI_MAPI_FALSE 0

#ifdef __cplusplus
extern "C"
{
#endif
typedef CVI_MAPI_HANDLE_T CVI_MAPI_AO_HANDLE_T;
#define MAX_AO_CALLBACK_COUNT (1)

typedef struct cviMAPI_AO_ATTR_S
{
    AUDIO_SAMPLE_RATE_E enSampleRate;
    uint32_t channels;
    uint32_t u32PtNumPerFrm;//frames unit
    uint32_t u32PowerPinId;
    uint32_t AudioChannel;
    int32_t  volume;
} CVI_MAPI_AO_ATTR_S;

typedef int (*CVI_MAPI_AO_DATAPROC_CALLBACK_FN_PTR)(CVI_MAPI_AO_HANDLE_T AoHdl,
    AUDIO_STREAM_S* pAStreamData, void *pPrivateData);

typedef struct cviMAPI_AO_CALLBACK_S
{
    CVI_MAPI_AO_DATAPROC_CALLBACK_FN_PTR pfnDataCB;
    void *pPrivateData; /** private data */
} CVI_MAPI_AO_CALLBACK_S;

typedef struct CVI_AO_CB_INFO_T{
    CVI_MAPI_AUDIO_CODEC_E enCodecType;
    uint32_t u32PtNumPerFrm;
    char *pBuffer;
    char * out_filename;
    FILE *fp;
}CVI_AO_CB_INFO;

typedef struct CVI_MAPI_AO_CTX_S {
    CVI_MAPI_AO_ATTR_S      attr;
    int                     AoDevid;
    int                     AoChn;
    int                     volume;
    int                     bMute;
    int                     bInit;
    int                     bBindAdec;
    volatile int            quit;
    pthread_mutex_t         ao_mutex;
    CVI_MAPI_ADEC_HANDLE_T  AdecHdl;
    cvi_osal_task_handle_t  cb_task;
    CVI_MAPI_AO_CALLBACK_S    *p_ao_cb[MAX_AO_CALLBACK_COUNT];
} CVI_MAPI_AO_CTX_T;

int CVI_MAPI_AO_GetHandle(CVI_MAPI_AO_HANDLE_T *AoHdl);
int CVI_MAPI_AO_Init(CVI_MAPI_AO_HANDLE_T *AoHdl,const CVI_MAPI_AO_ATTR_S *pstAoAttr);
int CVI_MAPI_AO_Deinit(CVI_MAPI_AO_HANDLE_T AoHdl);
int CVI_MAPI_AO_Start(CVI_MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn);
int CVI_MAPI_AO_Stop(CVI_MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn);
int CVI_MAPI_AO_EnableReSmp(CVI_MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn, AUDIO_SAMPLE_RATE_E enChnSamRate);
int CVI_MAPI_AO_DisableReSmp(CVI_MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn);
int CVI_MAPI_AO_SetVolume(CVI_MAPI_AO_HANDLE_T AoHdl,int volume);
int CVI_MAPI_AO_GetVolume(CVI_MAPI_AO_HANDLE_T AoHdl,int *volume);
int CVI_MAPI_AO_Mute(CVI_MAPI_AO_HANDLE_T AoHdl);
int CVI_MAPI_AO_Unmute(CVI_MAPI_AO_HANDLE_T AoHdl);
int CVI_MAPI_AO_SendFrame(CVI_MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn,const AUDIO_FRAME_S *pstAudioFrame, uint32_t u32Timeout);
int CVI_MAPI_AO_BindAdec(AUDIO_DEV AoDev, AO_CHN AoChn,ADEC_CHN AdChn);
int CVI_MAPI_AO_UnbindAdec(AUDIO_DEV AoDev, AO_CHN AoChn,ADEC_CHN AdChn);
int CVI_MAPI_AO_SendSysFrame(CVI_MAPI_AO_HANDLE_T AoHdl,const AUDIO_FRAME_S *pstAudioFrame, uint32_t u32Timeout);
int CVI_MAPI_AO_RegisterCallback(CVI_MAPI_AO_HANDLE_T AoHdl, const CVI_MAPI_AO_CALLBACK_S* pstAoCB);
int CVI_MAPI_AO_UnregisterCallback(CVI_MAPI_AO_HANDLE_T AoHdl, const CVI_MAPI_AO_CALLBACK_S* pstAoCB);
int CVI_MAPI_AO_SetAmplifier(CVI_MAPI_AO_HANDLE_T AoHdl, CVI_BOOL En);
int CVI_MAPI_AO_SetChnVolume(CVI_MAPI_AO_HANDLE_T AoHdl, CVI_S32 AoChn, int volume);
int CVI_MAPI_AO_GetChnVolume(CVI_MAPI_AO_HANDLE_T AoHdl, CVI_S32 AoChn, int *volume);
#ifdef __cplusplus
}
#endif

#endif //__CVI_MAPI_AO_H__




