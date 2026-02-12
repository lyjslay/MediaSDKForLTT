#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "cvi_log.h"
#include "cvi_eventhub.h"
#ifdef SERVICES_PLAYER_ON
#include "cvi_player_service_command.h"
#endif
#include "cvi_playbackmng.h"

void CVI_PLAYBACKMNG_EventCallBack(CVI_PLAYER_SERVICE_HANDLE_T hdl, CVI_PLAYER_SERVICE_EVENT_S *event_t)
{
    UNUSED(hdl);

    if (event_t == NULL) {
        CVI_LOGE("event is null");
        return;
    }
    /* Publish Event */
    CVI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(stEvent));

    switch(event_t->type) {
    case CVI_PLAYER_SERVICE_EVENT_PLAY_FINISHED:
        stEvent.topic = CVI_EVENT_PLAYBACKMNG_FINISHED;
        CVI_LOGI("Player play finish");
        break;
    case CVI_PLAYER_SERVICE_EVENT_PLAY_PROGRESS:
        stEvent.topic = CVI_EVENT_PLAYBACKMNG_PROGRESS;
        break;
    case CVI_PLAYER_SERVICE_EVENT_PAUSE:
        stEvent.topic = CVI_EVENT_PLAYBACKMNG_PAUSE;
        break;
    case CVI_PLAYER_SERVICE_EVENT_RESUME:
        stEvent.topic = CVI_EVENT_PLAYBACKMNG_RESUME;
        break;
    case CVI_PLAYER_SERVICE_EVENT_OPEN_FAILED:
    case CVI_PLAYER_SERVICE_EVENT_RECOVER_FAILED:
        stEvent.topic = CVI_EVENT_PLAYBACKMNG_FILE_ABNORMAL;
        break;
    case CVI_PLAYER_SERVICE_EVENT_PLAY:
        CVI_LOGI("Player playing...");
        stEvent.topic = CVI_EVENT_PLAYBACKMNG_PLAY;
        break;
    default:
        break;
    }
    CVI_EVENTHUB_Publish(&stEvent);
}

int32_t CVI_PLAYBACKMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_EVENTHUB_RegisterTopic(CVI_EVENT_PLAYBACKMNG_FINISHED);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_PLAYBACKMNG_PLAY);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_PLAYBACKMNG_PROGRESS);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_PLAYBACKMNG_PAUSE);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_PLAYBACKMNG_RESUME);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_PLAYBACKMNG_FILE_ABNORMAL);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, CVI_PLAYBACKMNG_EREGISTER_EVENT);

    return 0;
}
