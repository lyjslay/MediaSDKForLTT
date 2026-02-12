#include<stdio.h>
#include<pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>

#include "cvi_mapi_aenc.h"
#include "cvi_audio.h"
#include "cvi_log.h"
#include "cvi_osal.h"
#include "cvi_mapi.h"
#include "cvi_audio_aac_adp.h"
#include "cvi_audio_dl_adp.h"

#define AENC_TASK_TIMEOUT       (2000)//ms

extern void AUDIO_INIT_ONCE(void);
#define AENC_CHN_MAX (32)
static int g_aenc_chn_used[AENC_CHN_MAX] = {0};
static pthread_mutex_t g_aenc_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MAX_AENC_CALLBACK_COUNT (5)
typedef struct CVI_MAPI_AENC_CTX_S {
    CVI_MAPI_AENC_ATTR_S           attr;
    CVI_MAPI_AENC_CALLBACK_S    *p_aenc_cb[MAX_AENC_CALLBACK_COUNT];
    cvi_osal_task_handle_t      cb_task;
    sem_t                        cb_sem;
    cvi_osal_mutex_handle_t    cb_mutex;
    int                           bLock;
    volatile int                   quit;
    int                         enc_chn;
    int                       bBindAcap;
    CVI_MAPI_ACAP_HANDLE_T      AcapHdl;
    int                           bMute;
    int                        bWorking;
    int                           bInit;
    pthread_mutex_t          aenc_mutex;
    int                     AencAac_chn;
    int                    bRegisterAac;
    int                       bAacencode;
    AENC_AAC_INFO_S        stAencAacInfo;
} CVI_MAPI_AENC_CTX_T;

static int getValidAudioEncChanIndex(void)
{
    int i;
    for(i = 0; i < AENC_CHN_MAX; i++) {
        if(g_aenc_chn_used[i] == 0)
            return i;
    }
    return -1;
}

int CVI_MAPI_AENC_Init(CVI_MAPI_AENC_HANDLE_T *AencHdl, const CVI_MAPI_AENC_ATTR_S *pstAencAttr)
{
    int ret = 0;
    CVI_MAPI_AENC_CTX_T *pstAecCtx;
    AENC_CHN_ATTR_S stAencAttr;
    AENC_ATTR_G711_S stAencG711;
    AENC_ATTR_AAC_S  stAencAac;

    pstAecCtx = (CVI_MAPI_AENC_CTX_T *)malloc(sizeof(CVI_MAPI_AENC_CTX_T));
    if (!pstAecCtx) {
        CVI_LOGE("malloc failed\n");
        return CVI_MAPI_ERR_NOMEM;
    }

    memset(pstAecCtx, 0, sizeof(CVI_MAPI_AENC_CTX_T));
    memcpy(&pstAecCtx->attr,pstAencAttr,sizeof(CVI_MAPI_AENC_ATTR_S));

    AUDIO_INIT_ONCE();

    pthread_mutex_lock(&g_aenc_mutex);
    pstAecCtx->enc_chn = getValidAudioEncChanIndex();
    pthread_mutex_unlock(&g_aenc_mutex);
    if(pstAecCtx->enc_chn < 0) {
        CVI_LOGE("get enc chan failed\n");
        ret= CVI_MAPI_ERR_FAILURE;
        goto err;
    }
    g_aenc_chn_used[pstAecCtx->enc_chn] = 1;

    if(pstAencAttr->enAencFormat == CVI_MAPI_AUDIO_CODEC_G711U) {
        stAencAttr.enType = PT_G711U;
        stAencAttr.pValue       = &stAencG711;
    }else if(pstAencAttr->enAencFormat == CVI_MAPI_AUDIO_CODEC_G711A) {
        stAencAttr.enType = PT_G711A;
        stAencAttr.pValue       = &stAencG711;
    }else if(pstAencAttr->enAencFormat == CVI_MAPI_AUDIO_CODEC_AAC) {

        pstAecCtx->bAacencode =1;

        stAencAttr.enType = PT_AAC;


		stAencAac.enBitWidth = AUDIO_BIT_WIDTH_16;
		stAencAac.enSmpRate = pstAencAttr->src_samplerate;
		if(pstAencAttr->src_samplerate >= 48000)
			stAencAac.enBitRate = AAC_BPS_64K;
		else
			stAencAac.enBitRate = AAC_BPS_32K;

		//stAencAac.enSoundMode = bVqe?AUDIO_SOUND_MODE_MONO:pstAencAttr->channels;//--------vbqe
		stAencAac.enSoundMode = pstAencAttr->channels -1;
        stAencAac.enTransType = AAC_TRANS_TYPE_ADTS;
		stAencAac.s16BandWidth = 0;
        stAencAac.enAACType = AAC_TYPE_AACLC;
		stAencAttr.pValue = &stAencAac;
		CVI_MPI_AENC_AacInit();

    }else {
        CVI_LOGE("not support yet.\n");
        ret = CVI_MAPI_ERR_FAILURE;
        goto err;
    }

    stAencAttr.bFileDbgMode = false;
    stAencAttr.u32BufSize = 30;
    stAencAttr.u32PtNumPerFrm = pstAencAttr->u32PtNumPerFrm;
    pstAecCtx->stAencAacInfo.sample_rate = pstAencAttr->src_samplerate;


    ret =  CVI_AENC_CreateChn(pstAecCtx->enc_chn,&stAencAttr);
    if (ret != CVI_SUCCESS) {
         CVI_LOGE("aenc creat chn fail\n");
        ret = CVI_MAPI_ERR_FAILURE;
        goto err;
    }


    pthread_mutex_init(&pstAecCtx->aenc_mutex, NULL);
    pstAecCtx->bInit = true;
    *AencHdl = (CVI_MAPI_AENC_HANDLE_T)pstAecCtx;
    return CVI_MAPI_SUCCESS;
err:
    pstAecCtx->bInit = false;
    g_aenc_chn_used[pstAecCtx->enc_chn] = 0;
    return ret;
}

int CVI_MAPI_AENC_Deinit(CVI_MAPI_AENC_HANDLE_T AencHdl)
{
    int ret = CVI_MAPI_SUCCESS;
    //uint32_t timeout_cnt = 0;
    CVI_MAPI_AENC_CTX_T *pstAencCtx = (CVI_MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        return CVI_MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    pstAencCtx->quit = 1;
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);

    CVI_AENC_DestroyChn(pstAencCtx->enc_chn);

    if (pstAencCtx->bAacencode == 1)
        CVI_MPI_AENC_AacDeInit();

    g_aenc_chn_used[pstAencCtx->enc_chn] = 0;
    free(pstAencCtx);
    return ret;
}

int CVI_MAPI_AENC_Start(CVI_MAPI_AENC_HANDLE_T AencHdl)
{
#if 0
    CVI_MAPI_AENC_CTX_T *pstAencCtx = (CVI_MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }
    pthread_mutex_lock(&pstAencCtx->aenc_mutex);

    pstAencCtx->bWorking = 1;
    printf("pstAencCtx->AcapHdl  = %p,pstAencCtx->bBindAcap = %d\n",pstAencCtx->AcapHdl,pstAencCtx->bBindAcap);
    if(pstAencCtx->bBindAcap && pstAencCtx->AcapHdl) {
        printf("start aaaaaaaaaaaaaaaaaaaaaaa\n");
        CVI_MAPI_ACAP_Start(pstAencCtx->AcapHdl);
    }
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);
#endif
    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AENC_Stop(CVI_MAPI_AENC_HANDLE_T AencHdl)
{
#if 0
    CVI_MAPI_AENC_CTX_T *pstAencCtx = (CVI_MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    pstAencCtx->bWorking = 0;
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);
#endif
    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AENC_SetMute(CVI_MAPI_AENC_HANDLE_T AencHdl, int bEnable)
{
    CVI_MAPI_AENC_CTX_T *pstAencCtx = (CVI_MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    pstAencCtx->bMute = bEnable;
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AENC_GetMute(CVI_MAPI_AENC_HANDLE_T AencHdl, int *pbEnable)
{
    CVI_MAPI_AENC_CTX_T *pstAencCtx = (CVI_MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }
    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    *pbEnable = pstAencCtx->bMute;
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);
    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AENC_RegisterCallback(CVI_MAPI_AENC_HANDLE_T AencHdl, const CVI_MAPI_AENC_CALLBACK_S* pstAencCB)
{
    int ret = CVI_MAPI_SUCCESS;
    int i;
    CVI_MAPI_AENC_CTX_T *pstAencCtx = (CVI_MAPI_AENC_CTX_T *)AencHdl;

    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    for(i = 0; i < MAX_AENC_CALLBACK_COUNT; i++) {
        if(!pstAencCtx->p_aenc_cb[i]) {
            pstAencCtx->p_aenc_cb[i] = (CVI_MAPI_AENC_CALLBACK_S* )pstAencCB;
            break;
        }
    }
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);

    return ret;
}

int CVI_MAPI_AENC_UnregisterCallback(CVI_MAPI_AENC_HANDLE_T AencHdl, const CVI_MAPI_AENC_CALLBACK_S* pstAencCB)
{

    int ret = CVI_MAPI_SUCCESS;
    int i;
    CVI_MAPI_AENC_CTX_T *pstAencCtx = (CVI_MAPI_AENC_CTX_T *)AencHdl;

    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }
    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    for(i = 0; i < MAX_AENC_CALLBACK_COUNT; i++) {
        if(pstAencCtx->p_aenc_cb[i] && pstAencCtx->p_aenc_cb[i] == pstAencCB) {
            pstAencCtx->p_aenc_cb[i] = NULL;
            CVI_LOGI("unregister suc.\n");
            break;
        }
    }
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);

    return ret;

}

int CVI_MAPI_AENC_BindACap(AUDIO_DEV AiDev, AI_CHN AiChn,AUDIO_DEV AeDev, AENC_CHN AeChn)
{

	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_AI;
	stSrcChn.s32DevId = AiDev;
	stSrcChn.s32ChnId = AiChn;
	stDestChn.enModId = CVI_ID_AENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = AeChn;

	return CVI_AUD_SYS_Bind(&stSrcChn, &stDestChn);
}

int CVI_MAPI_AENC_UnbindACap(AUDIO_DEV AiDev, AI_CHN AiChn,AUDIO_DEV AeDev, AENC_CHN AeChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_AI;
	stSrcChn.s32ChnId = AiChn;
	stSrcChn.s32DevId = AiDev;
	stDestChn.enModId = CVI_ID_AENC;
	stDestChn.s32DevId = AeDev;
	stDestChn.s32ChnId = AeChn;


	return CVI_AUD_SYS_UnBind(&stSrcChn, &stDestChn);
}

int CVI_MAPI_AENC_SendFrame(CVI_MAPI_AENC_HANDLE_T AencHdl, const AUDIO_FRAME_S* pstFrm,AEC_FRAME_S* pstAecFrm)
{
    int ret = CVI_MAPI_SUCCESS;

    CVI_MAPI_AENC_CTX_T *pstAencCtx = (CVI_MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    ret = CVI_AENC_SendFrame(pstAencCtx->enc_chn, pstFrm,pstAecFrm);
    if(ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_AENC_SendFrame failed.ret:%#x\n",ret);
        pthread_mutex_unlock(&pstAencCtx->aenc_mutex);
        return ret;
    }

    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);
    return ret;
}

int CVI_MAPI_AENC_GetStream(CVI_MAPI_AENC_HANDLE_T AencHdl, AUDIO_STREAM_S *pstStream,CVI_S32 s32MilliSec)
{
    CVI_MAPI_AENC_CTX_T *pstAencCtx = (CVI_MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    return CVI_AENC_GetStream(pstAencCtx->enc_chn,pstStream,s32MilliSec);
}


