#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "cvi_log.h"
#include "cvi_eventhub.h"
#include "cvi_recordmng.h"

int32_t CVI_RECORDMNG_ContCallBack(CVI_RECORDER_EVENT_E event_type, const char *filename, void *param)
{
    /* Publish Event */
    CVI_EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(CVI_EVENT_S));
    switch (event_type) {
        case CVI_RECORDER_EVENT_START:
            CVI_LOGD("CVI_RECORDER_EVENT_START == %d param = %d\n",CVI_RECORDER_EVENT_START, *(int32_t *)param);
            stEvent.topic = CVI_EVENT_RECMNG_STARTREC;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_STOP:
            CVI_LOGD("CVI_RECORDER_EVENT_STOP == %d param = %d, filename = %s\n",CVI_RECORDER_EVENT_STOP, *(int32_t *)param, filename);
            stEvent.topic = CVI_EVENT_RECMNG_STOPREC;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_SPLIT_START:
            CVI_LOGD("CVI_RECORDER_EVENT_SPLIT_START == %d param = %d, filename = %s\n",CVI_RECORDER_EVENT_SPLIT_START, *(int32_t *)param, filename);
            stEvent.topic = CVI_EVENT_RECMNG_SPLITSTART;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_SPLIT:
            CVI_LOGD("CVI_RECORDER_EVENT_SPLIT == %d param = %d, filename = %s\n",CVI_RECORDER_EVENT_SPLIT, *(int32_t *)param, filename);
            stEvent.topic = CVI_EVENT_RECMNG_SPLITREC;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_WRITE_FRAME_TIMEOUT: {
            CVI_RECORDER_EVENT_WRITE_FRAME_TIMEOUT_S *timeout = (CVI_RECORDER_EVENT_WRITE_FRAME_TIMEOUT_S *)param;
            CVI_LOGD("CVI_RECORDER_EVENT_WRITE_FRAME_TIMEOUT == %d param = %d, timeout_ms = %d, filename = %s\n", CVI_RECORDER_EVENT_WRITE_FRAME_TIMEOUT, *(int32_t *)timeout->param, timeout->timeout_ms, filename);
            break;
        }
        case CVI_RECORDER_EVENT_WRITE_FRAME_FAILED:
            stEvent.topic = CVI_EVENT_RECMNG_WRITE_ERROR;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_OPEN_FILE_FAILED:
            stEvent.topic = CVI_EVENT_RECMNG_OPEN_FAILED;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_PIV_START:
            stEvent.topic = CVI_EVENT_RECMNG_PIV_START;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_PIV_END:
            stEvent.topic = CVI_EVENT_RECMNG_PIV_END;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_START_EMR:
            CVI_LOGD("CVI_RECORDER_EVENT_START_EMR == %d param = %d\n",CVI_RECORDER_EVENT_START_EMR, *(int32_t *)param);
            stEvent.topic = CVI_EVENT_RECMNG_STARTEMRREC;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_END_EMR:
            CVI_LOGD("CVI_RECORDER_EVENT_END_EMR == %d param = %d, filename = %s\n",CVI_RECORDER_EVENT_END_EMR, *(int32_t *)param, filename);
            stEvent.topic = CVI_EVENT_RECMNG_EMRREC_END;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_SYNC_DONE:
            CVI_LOGD("CVI_RECORDER_EVENT_SYNC_DONE == %d param = %d %s\n",CVI_RECORDER_EVENT_SYNC_DONE, *(int32_t *)param, filename);
            break;
        default:
            break;
    }

    CVI_EVENTHUB_Publish(&stEvent);

    return 0;
}

int32_t CVI_RECORDMNG_EventCallBack(CVI_RECORDER_EVENT_E event_type, const char *filename, void *param)
{
    /* Publish Event */
    CVI_EVENT_S stEvent;
    switch (event_type) {
        case CVI_RECORDER_EVENT_START:
            CVI_LOGD("CVI_RECORDER_EVENT_START == %d param = %d\n",CVI_RECORDER_EVENT_START, *(int32_t *)param);
            stEvent.topic = CVI_EVENT_RECMNG_STARTEVENTREC;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_STOP:
            CVI_LOGD("CVI_RECORDER_EVENT_STOP == %d param = %d, filename = %s\n",CVI_RECORDER_EVENT_STOP, *(int32_t *)param, filename);
            stEvent.topic = CVI_EVENT_RECMNG_EVENTREC_END;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_RECORDER_EVENT_START_EMR:
            CVI_LOGD("CVI_RECORDER_EVENT_START_EMR == %d param = %d\n",CVI_RECORDER_EVENT_START_EMR, *(int32_t *)param);
            stEvent.topic = CVI_EVENT_RECMNG_STARTEMRREC;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        default:
            break;
    }
    CVI_EVENTHUB_Publish(&stEvent);

    return 0;
}

int32_t CVI_RECORDMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_STARTREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_STOPREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_SPLITSTART);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_SPLITREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_STARTEVENTREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_EVENTREC_END);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_STARTEMRREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_EMRREC_END);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_PIV_START);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_PIV_END);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_WRITE_ERROR);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_RECMNG_OPEN_FAILED);
    CVI_APPCOMM_CHECK_RETURN(s32Ret, CVI_RECMNG_EREGISTER_EVENT);

    return 0;
}
