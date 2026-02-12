#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>

#include <cvi_type.h>
#include <cvi_comm_aio.h>
#include "cvi_mapi_ao.h"
#include "cvi_audio.h"
#include "cvi_log.h"
//#include "acodec.h"
#include "cvi_mapi_internal.h"
#include "cvi_mapi_define.h"
#include "cvi_osal.h"
#include "cvi_mapi_adec.h"
#include "cvi_hal_gpio.h"

typedef struct CVI_MAPI_ADEC_CTX_S {
    CVI_MAPI_ADEC_ATTR_S           attr;
    int                         dec_chn;
    int                           bMute;
    int                        bWorking;
    int                           bInit;
    AUDIO_FRAME_S               *pstFrame;
    pthread_mutex_t          adec_mutex;
} CVI_MAPI_ADEC_CTX_T;

static CVI_MAPI_AO_CTX_T *g_ao_hdl;
extern void AUDIO_INIT_ONCE(void);

int CVI_MAPI_AO_GetHandle(CVI_MAPI_AO_HANDLE_T *AoHdl)
{
    if (!g_ao_hdl) {
        CVI_LOGE("Please CVI_MAPI_AO_Init first");
        return CVI_MAPI_ERR_FAILURE;
    }
    *AoHdl = (CVI_MAPI_AO_HANDLE_T)g_ao_hdl;

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AO_Init(CVI_MAPI_AO_HANDLE_T *AoHdl,const CVI_MAPI_AO_ATTR_S *pstAoAttr)
{
    int ret=CVI_MAPI_SUCCESS;

    AIO_ATTR_S AudoutAttr;
    AudoutAttr.enSamplerate = pstAoAttr->enSampleRate;
    AudoutAttr.u32ChnCnt = pstAoAttr->channels;
    AudoutAttr.u32PtNumPerFrm = pstAoAttr->u32PtNumPerFrm;
    AudoutAttr.enSoundmode    = (pstAoAttr->AudioChannel == 2)?AUDIO_SOUND_MODE_STEREO:AUDIO_SOUND_MODE_MONO;
    AudoutAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    AudoutAttr.u32EXFlag      = 0;
    AudoutAttr.u32FrmNum      = 10; /* only use in bind mode */
    AudoutAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    AudoutAttr.u32ClkSel      = 0;
    AudoutAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

    AUDIO_INIT_ONCE();

    if(g_ao_hdl) {
        CVI_LOGE("CVI_MAPI_AO_Init alread init.\n");
        return CVI_MAPI_ERR_FAILURE;
    }
    g_ao_hdl = (CVI_MAPI_AO_CTX_T *)malloc(sizeof(CVI_MAPI_AO_CTX_T));
    if (!g_ao_hdl) {
        CVI_LOGE("malloc failed\n");
        return CVI_MAPI_ERR_NOMEM;
    }

    memset(g_ao_hdl, 0, sizeof(CVI_MAPI_AO_CTX_T));
    memcpy(&g_ao_hdl->attr,pstAoAttr,sizeof(CVI_MAPI_AO_ATTR_S));

    ret=CVI_AO_SetPubAttr(g_ao_hdl->AoDevid, &AudoutAttr);
    if(ret){
        CVI_LOGE("%s: CVI_AO_SetPubAttr(%d) failed with %d\n", __func__,g_ao_hdl->AoDevid,ret);
        free(g_ao_hdl);
        return ret;
    }
    g_ao_hdl->bInit = true;
    ret = CVI_AO_Enable(g_ao_hdl->AoDevid);
    if (ret) {
        CVI_LOGE("%s: CVI_AO_Enable(%d) failed with %d\n", __func__,g_ao_hdl->AoDevid,ret);
        return ret;
    }
    if(pstAoAttr->volume > 0){
        CVI_AO_SetVolume(g_ao_hdl->AoDevid, pstAoAttr->volume);
    }
    CVI_AO_GetVolume(g_ao_hdl->AoDevid,&(g_ao_hdl->volume));
    CVI_LOGW("aplay_demo volume %d\n",g_ao_hdl->volume);

    *AoHdl = (CVI_MAPI_AO_HANDLE_T)g_ao_hdl;
    return ret;
}

int CVI_MAPI_AO_Start(CVI_MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn)
{
    int ret=CVI_MAPI_SUCCESS;
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }
    if(et->bInit!=true){
        CVI_LOGE("Please CVI_MAPI_AO_Init first\n");
        return CVI_MAPI_ERR_FAILURE;
    }

    ret=CVI_AO_EnableChn(et->AoDevid,AoChn);
    if(ret){
        CVI_LOGE("%s: CVI_AI_Enable(%d) failed with %d\n",__func__,et->AoDevid,ret);
        return ret;
    }

    return ret;
}

int CVI_MAPI_AO_SendFrame(CVI_MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn,const AUDIO_FRAME_S *pstAudioFrame, uint32_t u32Timeout)
{
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }
    if(et->bBindAdec) {
        CVI_LOGE("audio has bind Adec and not use CVI_MAPI_AO_SendFrame()\n");
        return CVI_MAPI_ERR_INVALID;
    }
    return CVI_AO_SendFrame(et->AoDevid,AoChn,pstAudioFrame, u32Timeout);
}


int CVI_MAPI_AO_Deinit(CVI_MAPI_AO_HANDLE_T AoHdl)
{
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }
    CVI_AO_Disable(et->AoDevid);
    if (g_ao_hdl) {
        free(g_ao_hdl);
        g_ao_hdl=NULL;
    }
    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AO_Stop(CVI_MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn)
{
    int ret;
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        return CVI_MAPI_ERR_INVALID;
    }

    ret = CVI_AO_DisableChn(et->AoDevid,AoChn);
    if (ret) {
        CVI_LOGE("%s: CVI_AO_DisableChn failed with %#x!\n", __func__, ret);
        return ret;
    }

    return ret;
}
int CVI_MAPI_AO_EnableReSmp(CVI_MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn, AUDIO_SAMPLE_RATE_E enChnSamRate)
{
    int ret = CVI_MAPI_SUCCESS;
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    ret = CVI_AO_EnableReSmp(et->AoDevid,AoChn,enChnSamRate);
    if(ret){
        CVI_LOGE("CVI_AO_EnableReSmp(%d,%d,%d) err\n",et->AoDevid,AoChn,enChnSamRate);
        return CVI_MAPI_ERR_FAILURE;
    }

    return ret;
}

int CVI_MAPI_AO_DisableReSmp(CVI_MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn)
{
    int ret = CVI_MAPI_SUCCESS;
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    ret = CVI_AO_DisableReSmp(et->AoDevid, AoChn);
    if(ret){
        CVI_LOGE("CVI_AO_DisableReSmp(%d,%d) err\n", et->AoDevid, AoChn);
        return CVI_MAPI_ERR_FAILURE;
    }

    return ret;
}
int CVI_MAPI_AO_SetVolume(CVI_MAPI_AO_HANDLE_T AoHdl,int volume)
{
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }
    CVI_LOGW("AoDevid :%d and set volume: %d\n",et->AoDevid,volume);

    if(CVI_AO_SetVolume(et->AoDevid,volume)){
        CVI_LOGE("CVI_AO_SetVolume() ERR\n");
        return CVI_MAPI_ERR_FAILURE;
    }
    et->volume=volume;
    return  CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AO_GetVolume(CVI_MAPI_AO_HANDLE_T AoHdl,int *volume)
{

    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    if(CVI_AO_GetVolume(et->AoDevid,volume)){
        CVI_LOGE("CVI_GetVolime() ERR\n");
        return CVI_MAPI_ERR_FAILURE;
    }
    CVI_LOGW("AoDevid :%d and get volume: %d\n",et->AoDevid,*volume);
    CVI_LOGW("g_ao_hdl->volume=%d\n",et->volume);
    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AO_Mute(CVI_MAPI_AO_HANDLE_T AoHdl)
{
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    CVI_AO_SetMute(et->AoDevid,CVI_MAPI_TRUE,NULL);
    et->bMute = CVI_MAPI_TRUE;
    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AO_Unmute(CVI_MAPI_AO_HANDLE_T AoHdl)
{
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    CVI_AO_SetMute(et->AoDevid,CVI_MAPI_FALSE,NULL);
    et->bMute = CVI_MAPI_FALSE;
    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AO_BindAdec(AUDIO_DEV AoDev, AO_CHN AoChn,ADEC_CHN AdChn)
{
    MMF_CHN_S stSrcChn, stDestChn;

    stSrcChn.enModId = CVI_ID_ADEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = AdChn;
    stDestChn.enModId = CVI_ID_AO;
    stDestChn.s32DevId = AoDev;
    stDestChn.s32ChnId = AoChn;

    return CVI_AUD_SYS_Bind(&stSrcChn,&stDestChn);
}
int CVI_MAPI_AO_UnbindAdec(AUDIO_DEV AoDev, AO_CHN AoChn,ADEC_CHN AdChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_ADEC;
	stSrcChn.s32ChnId = AdChn;
	stSrcChn.s32DevId = 0;
	stDestChn.enModId = CVI_ID_AO;
	stDestChn.s32DevId = AoDev;
	stDestChn.s32ChnId = AoChn;
	return CVI_AUD_SYS_UnBind(&stSrcChn, &stDestChn);

}
int CVI_MAPI_AO_SendSysFrame(CVI_MAPI_AO_HANDLE_T AoHdl,const AUDIO_FRAME_S *pstAudioFrame, uint32_t u32Timeout)
{
    CVI_LOGW("not support.\n");
    return CVI_MAPI_SUCCESS;
}


int CVI_MAPI_AO_RegisterCallback(CVI_MAPI_AO_HANDLE_T AoHdl, const CVI_MAPI_AO_CALLBACK_S* pstAoCB)
{
    int ret = CVI_MAPI_SUCCESS;
    int i;
     CVI_MAPI_AO_CTX_T *pstAoCtx=(CVI_MAPI_AO_CTX_T *)AoHdl;

    if(!pstAoCtx) {
        CVI_LOGE("params NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAoCtx->ao_mutex);
    for(i = 0; i < MAX_AO_CALLBACK_COUNT; i++) {
        if(!pstAoCtx->p_ao_cb[i]) {
            pstAoCtx->p_ao_cb[i] = (CVI_MAPI_AO_CALLBACK_S* )pstAoCB;
            break;
        }
    }
    pthread_mutex_unlock(&pstAoCtx->ao_mutex);

    return ret;
}

int CVI_MAPI_AO_UnregisterCallback(CVI_MAPI_AO_HANDLE_T AoHdl, const CVI_MAPI_AO_CALLBACK_S* pstAoCB)
{
    int ret = CVI_MAPI_SUCCESS;
    int i;
     CVI_MAPI_AO_CTX_T *pstAoCtx=(CVI_MAPI_AO_CTX_T *)AoHdl;

    if(!pstAoCtx) {
        CVI_LOGE("params NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAoCtx->ao_mutex);
    for(i = 0; i < MAX_AO_CALLBACK_COUNT; i++) {
        if(pstAoCtx->p_ao_cb[i] && pstAoCtx->p_ao_cb[i] == pstAoCB) {
            pstAoCtx->p_ao_cb[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&pstAoCtx->ao_mutex);
    return ret;

}

int CVI_MAPI_AO_SetAmplifier(CVI_MAPI_AO_HANDLE_T AoHdl, CVI_BOOL En)
{
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    CVI_GPIO_Set_Value(et->attr.u32PowerPinId, En);

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AO_SetChnVolume(CVI_MAPI_AO_HANDLE_T AoHdl, CVI_S32 AoChn, int volume)
{
    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }
    CVI_LOGW("AoDevid :%d and set %d volume: %d\n",et->AoDevid,AoChn,volume);

    // if(CVI_AO_SetChnVolume(et->AoDevid, AoChn, volume)){
    //     CVI_LOGE("CVI_AO_SetVolume() ERR\n");
    //     return CVI_MAPI_ERR_FAILURE;
    // }
    et->volume=volume;
    return  CVI_MAPI_SUCCESS;
}

int CVI_MAPI_AO_GetChnVolume(CVI_MAPI_AO_HANDLE_T AoHdl, CVI_S32 AoChn, int *volume)
{

    CVI_MAPI_AO_CTX_T *et=(CVI_MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return CVI_MAPI_ERR_INVALID;
    }

    // if(CVI_AO_GetChnVolume(et->AoDevid, AoChn, volume)){
    //     CVI_LOGE("CVI_GetVolime() ERR\n");
    //     return CVI_MAPI_ERR_FAILURE;
    // }
    CVI_LOGW("AoDevid :%d and get %d volume: %d\n",et->AoDevid, AoChn, *volume);
    CVI_LOGW("g_ao_hdl->volume=%d\n",et->volume);
    return CVI_MAPI_SUCCESS;
}