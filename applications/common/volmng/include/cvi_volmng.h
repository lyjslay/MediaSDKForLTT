#ifndef __CVI_VOLMNG_H__
#define __CVI_VOLMNG_H__

#include "cvi_appcomm.h"
#include "cvi_mapi_ao.h"
//#include "cvi_player_service.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** macro define */
#define CVI_VOLMNG_EINVAL            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VOLMNG, CVI_EINVAL)               /**<parm error*/
#define CVI_VOLMNG_EINTER            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VOLMNG, CVI_EINTER)               /**<intern error*/
#define CVI_VOLMNG_ENOINIT           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VOLMNG, CVI_ENOINIT)              /**< no initialize*/
#define CVI_VOLMNG_EINITIALIZED      CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VOLMNG, CVI_EINITIALIZED)         /**< already initialized */
#define CVI_VOLMNG_EREGISTEREVENT    CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VOLMNG, CVI_ERRNO_CUSTOM_BOTTOM)  /**<thread creat or join error*/
#define CVI_VOLMNG_ETHREAD           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VOLMNG, CVI_ERRNO_CUSTOM_BOTTOM+1)/**<thread creat or join error*/


/** Path Maximum Length */
#define CVI_VOICE_MAX_SEGMENT_CNT (5)
#define VOLUME_DEFAULT 6

typedef struct _CVI_VOICEPLAYER_AOUT_OPT_S {
    void * hAudDevHdl;    // device id
    void * hAudTrackHdl;  // chn id
} CVI_VOICEPLAY_AOUT_OPT_S;

typedef struct _CVI_VOICEPLAY_VOICETABLE_S
{
    uint32_t u32VoiceIdx;
    char aszFilePath[CVI_APPCOMM_MAX_PATH_LEN];
} CVI_VOICEPLAY_VOICETABLE_S;

/** voiceplay configuration */
typedef struct __CVI_VOICEPLAY_CFG_S
{
    uint32_t u32MaxVoiceCnt;
    CVI_VOICEPLAY_VOICETABLE_S* pstVoiceTab;
    CVI_VOICEPLAY_AOUT_OPT_S stAoutOpt;
} CVI_VOICEPLAY_CFG_S;

typedef struct _CVI_VOICEPLAY_VOICE_S
{
    uint32_t volume;
    uint32_t u32VoiceCnt;
    uint32_t au32VoiceIdx[CVI_VOICE_MAX_SEGMENT_CNT];
    bool bDroppable;
} CVI_VOICEPLAY_VOICE_S;


//int32_t CVI_Start_VOICE_PLAY_File(const char* pathname, int32_t volume);                        /*When the machine starting up call palying the voice*/
int32_t CVI_VOICEPLAY_Init(const CVI_VOICEPLAY_CFG_S* pstCfg);                              /*the api will call in the media arrangment*/
int32_t CVI_VOICEPLAY_DeInit(void);
int32_t CVI_VOICEPLAY_Push(const CVI_VOICEPLAY_VOICE_S* pstVoice, int32_t u32Timeout_ms);   /*the api will push in queue */
int32_t CVI_VOICEPLAY_SetAmplifier(bool en);
int32_t CVI_VOICEPLAY_SetAmplifierFlage(bool en);
int32_t CVI_VOICEPLAY_SetVolume(int32_t volume);
int32_t CVI_VOICEPLAY_GetVolume(int32_t *volume);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_PLAYBACKMNG_H__ */