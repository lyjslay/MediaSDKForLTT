#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_modeinner.h"

//PLAYBACK MODE
static int32_t CVI_MODEMNG_PlayBackModeScanFile(void)
{
    int32_t s32Ret = 0;
    uint32_t u32Index = 0;
    uint32_t u32DirCount = 0;
    CVI_PARAM_FILEMNG_S stCfg = {0};
    CVI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT];

    s32Ret = CVI_PARAM_GetFileMngParam(&stCfg);
    if(s32Ret != 0) {
        CVI_LOGW("get file system param fialed\n");
        return s32Ret;
    }

    for(u32Index = 0; u32Index < DTCF_DIR_BUTT; u32Index++) {
        if ( 0 < strnlen(stCfg.FileMngDtcf.aszDirNames[u32Index], CVI_DIR_LEN_MAX)) {
            aenDirs[u32DirCount++] = u32Index;
        }
    }

    uint32_t u32Temp = 0;
    s32Ret = CVI_FILEMNG_SetSearchScope(aenDirs, u32DirCount, &u32Temp);
    if (s32Ret != 0) {
        CVI_LOGW("search scope all dir fialed\n");
    }

    return s32Ret;
}

static bool check_jpg_file_complete(const char*filename)
{
    FILE *fp = fopen(filename, "rb");
    if (NULL == fp) {
        CVI_LOGF("fail to open file %s\n", filename);
        return false;
    }
    unsigned char read_temp_buf[8] = {0};
    uint32_t seek_loc = 0;
    fseek(fp, 0, SEEK_SET);
    fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
    seek_loc = (((read_temp_buf[4] << 8) | read_temp_buf[5]) + 4);
    fseek(fp, seek_loc, SEEK_SET);
    fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
    seek_loc = (read_temp_buf[4]<<24 | read_temp_buf[5]<<16 | read_temp_buf[6]<<8 | read_temp_buf[7]<<0) + seek_loc + 4;
    fseek(fp, seek_loc, SEEK_SET);
    fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
    if ((read_temp_buf[0] == 0xFF) && (read_temp_buf[1] == 0xD9) && (read_temp_buf[2] == 0xFF) && (read_temp_buf[3] == 0xD9)) {
        fclose(fp);
        return true;
    } else {
        fclose(fp);
        return false;
    }

    return true;
}

int32_t CVI_MODEMNG_PlayFile(char* filename, int type) {
    int32_t  s32Ret = 0;
    //char filename[MAX_PATH];
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
    CVI_PLAYER_MEDIA_INFO_S info = {};

    CVI_LOGE("filename = %s\n", filename);

    s32Ret = CVI_PLAYER_SERVICE_SetInput(ps_handle, filename);
    if (s32Ret != 0) {
        CVI_LOGE("Player set input %s failed", filename);
        return s32Ret;
    }

    s32Ret = CVI_PLAYER_SERVICE_GetMediaInfo(ps_handle, &info);
    if (s32Ret != 0) {
        CVI_LOGE("Player Get MediaInfo %s failed", filename);
        return s32Ret;
    }
    printf("### Playback file info: w(%d) h(%d) video_codec=%s, audio_codec=%s ###\n", info.width, info.height, info.video_codec, info.audio_codec);

    if (type == 1) {
        bool jpg_flag = check_jpg_file_complete(filename);
        if (false == jpg_flag) {
            CVI_LOGE("error: jpg file %s is not complete, play failed!!!\n", filename);
            return -1;
        }
    }

    #ifdef SERVICES_Player_Subvideo_ON
    CVI_PLAYER_SERVICE_SetPlaySubStreamFlag(ps_handle, true);
    #endif

    if (strcmp(info.video_codec, "h264") == 0 || strcmp(info.video_codec, "mjpeg") == 0) {
        CVI_PLAYER_SERVICE_Play(ps_handle);
    } else {
        CVI_LOGE("video can't play because codec %s is not supported\n", info.video_codec);
        return -2;
    }

    return s32Ret;

}

int32_t CVI_MODEMNG_PlayPause(void) {
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = CVI_MEDIA_GetCtx()->SysServices.PsHdl;
    CVI_PLAYER_SERVICE_Pause(ps_handle);
    return 0;
}

int32_t CVI_MODEMNG_PlayContinue(int switch_file_flag) {
    int32_t  s32Ret = 0;
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = CVI_MEDIA_GetCtx()->SysServices.PsHdl;
    switch (switch_file_flag) {
        case 0:
            CVI_PLAYER_SERVICE_Play(ps_handle);
            break;
        case 1: //play next
            CVI_PLAYER_SERVICE_Stop(ps_handle);
            break;
        case 2: //play previous file
            CVI_PLAYER_SERVICE_Stop(ps_handle);
            break;
        default:
            CVI_LOGI("ERROR PARM\n");
            break;
    }

    return s32Ret;
}

int32_t CVI_MODEMNG_PlayStop(void) {
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = CVI_MEDIA_GetCtx()->SysServices.PsHdl;
    CVI_PLAYER_SERVICE_Stop(ps_handle);
    return 0;
}

int32_t CVI_MODEMNG_PlayForward(int speed) {
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = CVI_MEDIA_GetCtx()->SysServices.PsHdl;
    CVI_PLAYER_SERVICE_PlayerSeep(ps_handle, speed);
    return 0;
}

int32_t CVI_MODEMNG_PlayBackward(int speed) {
    CVI_PLAYER_SERVICE_HANDLE_T ps_handle = CVI_MEDIA_GetCtx()->SysServices.PsHdl;
    CVI_PLAYER_SERVICE_PlayerSeepBack(ps_handle, speed);
    return 0;
}

int32_t CVI_MODEMNG_OpenPlayBackMode(void)
{
#ifdef SERVICES_PLAYER_ON
    int32_t s32Ret = 0;
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    s32Ret = CVI_FILEMNG_FileCoverStatus(false);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_FILEMNG_FileCoverStatus fail");

    s32Ret = CVI_MEDIA_VbInitPlayBack();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_VbInitPlayBack fail");

    s32Ret = CVI_MEDIA_DispInit(false);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_DispInit fail");

    s32Ret = CVI_MEDIA_PlayBackSerInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_PlayBackSerInit fail");

    CVI_MEDIA_PARAM_INIT_S *SysMediaParams = CVI_MEDIA_GetCtx();
    CVI_PLAYER_SERVICE_HANDLE_T PlaySerhdl = SysMediaParams->SysServices.PsHdl;

    s32Ret = CVI_PLAYER_SERVICE_SetEventHandler(PlaySerhdl, CVI_PLAYBACKMNG_EventCallBack);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_PLAYER_SERVICE_SetEventHandler fail");

    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = CVI_WORK_MODE_PLAYBACK;
    CVI_EVENTHUB_Publish(&stEvent);

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_PLAYBACK;

    //set open amplifiler
    CVI_VOICEPLAY_SetAmplifier(false);
#endif
    return 0;
}

int32_t CVI_MODEMNG_ClosePlayBackMode(void)
{
#ifdef SERVICES_LIVEVIEW_ON
    int32_t s32Ret = 0;

    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_BUTT;
    pstModeMngCtx->u32ModeState = CVI_MEDIA_MOVIE_STATE_BUTT;

    s32Ret = CVI_MEDIA_PlayBackSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_PlayBackSerDeInit fail");

    s32Ret = CVI_MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_DispDeInit fail");

    s32Ret = CVI_MEDIA_VbDeInit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MEDIA_VbDeInit fail");

    if (CVI_CARD_STATE_AVAILABLE == CVI_MODEMNG_GetCardState()) {
        s32Ret = CVI_MODEMNG_PlayBackModeScanFile();
        MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_MODEMNG_PlayBackModeScanFile fail");
    }

    CVI_EVENT_S stEvent;
    stEvent.topic = CVI_EVENT_MODEMNG_MODECLOSE;
    stEvent.arg1 = CVI_WORK_MODE_PLAYBACK;
    CVI_EVENTHUB_Publish(&stEvent);

    s32Ret = CVI_FILEMNG_FileCoverStatus(true);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_FILEMNG_FileCoverStatus fail");

    //set close amplifiler
    //CVI_VOICEPLAY_SetAmplifier(true);
    CVI_VOICEPLAY_SetAmplifierFlage(true);
#endif

    return 0;
}

/** Playback Mode message process */
int32_t CVI_MODEMNG_PlaybackModeMsgProc(CVI_MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
#ifdef SERVICES_PLAYER_ON
    // int32_t s32Ret = 0;
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return CVI_PROCESS_MSG_RESULTE_OK;
    }

    /** check parameters */
    MODEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstModeMngCtx->bInProgress);

    CVI_STATE_S* pstStateAttr = (CVI_STATE_S*)pvArg;
    CVI_LOGD("current mode(%s)\n\n", pstStateAttr->name);
    CVI_LOGD(" will process message topic(%x) \n\n", pstMsg->topic);
    switch (pstMsg->topic)
    {
        // transplant from ui
        case CVI_EVENT_MODEMNG_PLAYBACK_PLAY:
        {
            int ret = 0;
            if(pstMsg->aszPayload[0]){
                printf("### playmode play file %s, file type:%d ###\n", pstMsg->aszPayload, pstMsg->arg1);
                ret = CVI_MODEMNG_PlayFile((char*)pstMsg->aszPayload, pstMsg->arg1);
                if (ret != 0) {
                    printf("play fail, ret:%d, write to msg result\n", ret);
                    pstMsg->s32Result = ret;
                }
                return CVI_PROCESS_MSG_RESULTE_OK;
            }
            return CVI_PROCESS_MSG_UNHANDLER;
        }
        case CVI_EVENT_MODEMNG_PLAYBACK_PAUSE:
        {
            printf("### playmode pause ###\n");
            CVI_MODEMNG_PlayPause();
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_MODEMNG_PLAYBACK_RESUME:
        {
            printf("### playmode resume, flag:%d ###\n", pstMsg->arg1);
            CVI_MODEMNG_PlayContinue(pstMsg->arg1);
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_MODEMNG_PLAYBACK_FINISHED:
        {
            printf("### playmode stop ###\n");
            CVI_MODEMNG_PlayStop();
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_MODEMNG_PLAYBACK_FORWARD:
        {
            printf("### playmode forward speed: %d ###\n", pstMsg->arg1);
            CVI_MODEMNG_PlayForward(pstMsg->arg1);
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_MODEMNG_PLAYBACK_BACKWARD:
        {
            printf("### playmode backward speed: %d ###\n", pstMsg->arg1);
            CVI_MODEMNG_PlayBackward(pstMsg->arg1);
            return CVI_PROCESS_MSG_RESULTE_OK;
        }

        case CVI_EVENT_PLAYBACKMNG_PLAY:
        {
            CVI_MODEMNG_MonitorStatusNotify(pstMsg);
            pstModeMngCtx->u32ModeState = CVI_MEDIA_PLAYBACK_STATE_PLAY;
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_PLAYBACKMNG_FINISHED:
        {
            if(CVI_CARD_STATE_REMOVE == CVI_MODEMNG_GetCardState()) {
                CVI_LOGD("Card Removed, Not process event:%x\n",
                    CVI_EVENT_PLAYBACKMNG_FINISHED);
                return CVI_PROCESS_MSG_UNHANDLER;
            }
            pstModeMngCtx->u32ModeState = CVI_MEDIA_PLAYBACK_STATE_VIEW;
            CVI_MODEMNG_MonitorStatusNotify(pstMsg);
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_PLAYBACKMNG_PROGRESS:
        {
            CVI_MODEMNG_MonitorStatusNotify(pstMsg);
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_PLAYBACKMNG_PAUSE:
        {
            CVI_MODEMNG_MonitorStatusNotify(pstMsg);
            pstModeMngCtx->u32ModeState = CVI_MEDIA_PLAYBACK_STATE_PAUSE;
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_PLAYBACKMNG_RESUME:
        {
            CVI_MODEMNG_MonitorStatusNotify(pstMsg);
            pstModeMngCtx->u32ModeState = CVI_MEDIA_PLAYBACK_STATE_PLAY;
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_PLAYBACKMNG_FILE_ABNORMAL:
        {
            CVI_MODEMNG_MonitorStatusNotify(pstMsg);
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        case CVI_EVENT_SENSOR_PLUG_STATUS:
        {
            CVI_PARAM_CFG_S Param;
            CVI_PARAM_GetParam(&Param);
            int32_t snsid = pstMsg->aszPayload[1];
            int32_t mode = pstMsg->aszPayload[0];
            if (CVI_SENSOR_PLUG_IN == pstMsg->arg1) {
                CVI_LOGD("sensor %d plug in\n", snsid);
                CVI_LOGD("sensor %d resolution=%d\n", snsid, mode);
                if(MAX_CAMERA_INSTANCES > 1 && MAX_DEV_INSTANCES == 1){
                    for(int i = 0;i < MAX_CAMERA_INSTANCES;i++){
                        Param.CamCfg[i].CamMediaInfo.CurMediaMode = CVI_MEDIA_Res2RecordMediaMode(mode);
                        Param.WorkModeCfg.RecordMode.CamMediaInfo[i].CurMediaMode = CVI_MEDIA_Res2RecordMediaMode(mode);
                        Param.WorkModeCfg.PhotoMode.CamMediaInfo[i].CurMediaMode = CVI_MEDIA_Res2PhotoMediaMode(mode);
                    }
                } else {
                    Param.CamCfg[snsid].CamMediaInfo.CurMediaMode = CVI_MEDIA_Res2RecordMediaMode(mode);
                    Param.WorkModeCfg.RecordMode.CamMediaInfo[snsid].CurMediaMode = CVI_MEDIA_Res2RecordMediaMode(mode);
                    Param.WorkModeCfg.PhotoMode.CamMediaInfo[snsid].CurMediaMode = CVI_MEDIA_Res2PhotoMediaMode(mode);
                }
                Param.CamCfg[snsid].CamEnable = true;
                Param.MediaComm.Window.Wnds[snsid].WndEnable = true;
                CVI_MAPI_VCAP_SetAhdMode(snsid, mode);
            } else if (CVI_SENSOR_PLUG_OUT == pstMsg->arg1) {
                CVI_LOGD("sensor %d plug out\n", snsid);
                Param.CamCfg[snsid].CamEnable = false;
                Param.MediaComm.Window.Wnds[snsid].WndEnable = false;
            }
            CVI_PARAM_SetParam(&Param);
            return CVI_PROCESS_MSG_RESULTE_OK;
        }
        default:
            return CVI_PROCESS_MSG_UNHANDLER;
    }
#endif
    return CVI_PROCESS_MSG_RESULTE_OK;
}


int32_t CVI_MODEMNG_PlaybackStatesInit(const CVI_STATE_S* pstBase)
{
    int32_t s32Ret = 0;
#ifdef SERVICES_PLAYER_ON
    static CVI_STATE_S stPlaybackState =
    {
        CVI_WORK_MODE_PLAYBACK,
        MODEEMNG_STATE_PLAYBACK,
        CVI_MODEMNG_OpenPlayBackMode,
        CVI_MODEMNG_ClosePlayBackMode,
        CVI_MODEMNG_PlaybackModeMsgProc,
        NULL
    };
    stPlaybackState.argv = &stPlaybackState;
    s32Ret = CVI_HFSM_AddState(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stPlaybackState,
                              (CVI_STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "HFSM add NormalRec state");
#endif

    return s32Ret;
}

/** deinit playback mode */
int32_t CVI_MODEMNG_PlaybackStatesDeinit(void)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_ClosePlayBackMode();
    return s32Ret;
}
