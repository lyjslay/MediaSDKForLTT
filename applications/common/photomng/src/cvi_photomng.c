#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "cvi_log.h"
#include "cvi_eventhub.h"
#include "cvi_photomng.h"

int32_t CVI_POHTOMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_EVENTHUB_RegisterTopic(CVI_EVENT_PHOTOMNG_OPEN_FAILED);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_PHOTOMNG_PIV_START);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_PHOTOMNG_PIV_END);

    return s32Ret;
}

int32_t CVI_PHOTOMNG_ContCallBack(CVI_PHOTO_SERVICE_EVENT_E event_type, const char *filename, void *param)
{
    /* Publish Event */
    CVI_EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(CVI_EVENT_S));
    switch (event_type) {
        case CVI_PHOTO_SERVICE_EVENT_OPEN_FILE_FAILED:
            stEvent.topic = CVI_EVENT_PHOTOMNG_OPEN_FAILED;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_PHOTO_SERVICE_EVENT_PIV_START:
            stEvent.topic = CVI_EVENT_PHOTOMNG_PIV_START;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_PHOTO_SERVICE_EVENT_PIV_END:
            stEvent.topic = CVI_EVENT_PHOTOMNG_PIV_END;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case CVI_PHOTO_SERVICE_EVENT_SYNC_DONE:
            CVI_LOGD("CVI_RECORDER_EVENT_SYNC_DONE == %d param = %d %s\n",CVI_PHOTO_SERVICE_EVENT_SYNC_DONE, *(int32_t *)param, filename);
            break;
        default:
            break;
    }

    CVI_EVENTHUB_Publish(&stEvent);

    return 0;
}
