#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include "cvi_eventhub.h"
#include "cvi_speechmng.h"
#include "cvi_sysutils.h"

#define TPU_KO_PATH CVI_KOMOD_PATH "/" CHIP_TYPE "_tpu.ko"

static CVI_SPEECH_HANDLE_S *speechHdl;

static int32_t CVI_SPEECHMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_STARTREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_STOPREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_OPENFRONT);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_OPENREAR);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_CLOSESCREEN);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_OPENSCREEN);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_EMRREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_PIV);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_OPENWIFI);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_CLOSEWIFI);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, CVI_SPEECHMNG_EREGISTER_EVENT);
    return s32Ret;
}

static int32_t CVI_SPEECHMNG_Function(int32_t index)
{
    CVI_EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(CVI_EVENT_S));
    switch (index)
    {
    case CVI_SPEECHMNG_TURN_OFF_RECORDING:
        stEvent.topic = CVI_EVENT_SPEECHMNG_STOPREC;
        break;
    case CVI_SPEECHMNG_OPEN_RECORDING:
        stEvent.topic = CVI_EVENT_SPEECHMNG_STARTREC;
        break;
    case CVI_SPEECHMNG_SHOW_FRONT:
        stEvent.topic = CVI_EVENT_SPEECHMNG_OPENFRONT;
        break;
    case CVI_SPEECHMNG_SHOW_REAR:
        stEvent.topic = CVI_EVENT_SPEECHMNG_OPENREAR;
        break;
    case CVI_SPEECHMNG_TURN_OFF_SCREEN:
        stEvent.topic = CVI_EVENT_SPEECHMNG_CLOSESCREEN;
        break;
    case CVI_SPEECHMNG_OPEN_SCREEN:
        stEvent.topic = CVI_EVENT_SPEECHMNG_OPENSCREEN;
        break;
    case CVI_SPEECHMNG_LOCK_VEDIO:
        stEvent.topic = CVI_EVENT_SPEECHMNG_EMRREC;
        break;
    case CVI_SPEECHMNG_TAKE_PICTURE:
        stEvent.topic = CVI_EVENT_SPEECHMNG_PIV;
        break;
    case CVI_SPEECHMNG_TURN_OFF_WIFI:
        stEvent.topic = CVI_EVENT_SPEECHMNG_CLOSEWIFI;
        break;
    case CVI_SPEECHMNG_OPEN_WIFI:
        stEvent.topic = CVI_EVENT_SPEECHMNG_OPENWIFI;
        break;
    default:
        break;
    }
    CVI_EVENTHUB_Publish(&stEvent);
    return 0;
}

int32_t CVI_SPEECHMNG_Init(CVI_SPEECHMNG_PARAM_S* SpeechCfg)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_SPEECHMNG_RegisterEvent();
    cvi_insmod(TPU_KO_PATH, NULL);
    CVI_SPEECH_SERVICE_Register(CVI_SPEECHMNG_Function);
    if(speechHdl == NULL){
        speechHdl = malloc(sizeof(SPEECH_CONTEXT_HANDLE_S));
    }
    CVI_SPEECH_SERVICE_PARAM_S speechParam = {0};
    memcpy(&speechParam, SpeechCfg, sizeof(CVI_SPEECHMNG_PARAM_S));
    s32Ret = CVI_SPEECH_SERVICE_Create(speechHdl, &speechParam);
    return s32Ret;
}

int32_t CVI_SPEECHMNG_DeInit(void)
{
    int32_t s32Ret = 0;
    if (*speechHdl != NULL) {
        s32Ret = CVI_SPEECH_SERVICE_Destroy(*speechHdl);
        if (*speechHdl != NULL) {
            free(speechHdl);
            *speechHdl = NULL;
        }
    }
    cvi_rmmod(TPU_KO_PATH);
    return s32Ret;
}

int32_t CVI_SPEECHMNG_StartSpeech(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_SPEECH_SERVICE_StartSpeech(*speechHdl);
    return s32Ret;
}

int32_t CVI_SPEECHMNG_StopSpeech(void)
{
    int32_t s32Ret = 0;
    CVI_SPEECH_SERVICE_StopSpeech(*speechHdl);
    return s32Ret;
}
