#ifndef __PS_CONTEXT_H__
#define __PS_CONTEXT_H__


#include "cvi_log.h"
#include "cvi_osal.h"
#include "cvi_mq.h"
#include "cvi_mapi.h"
#include "cvi_mapi_ao.h"
#include "cvi_player_service.h"
#include "cvi_player/cvi_player.h"
#include "cvi_file_recover/cvi_file_recover.h"

typedef struct {
    CVI_PLAYER_SERVICE_PARAM_S param;
    volatile bool playing;
    volatile bool shutdown;
    CVI_MAPI_VCODEC_E codec_type;
    cvi_osal_mutex_handle_t play_mutex;
    uint32_t screen_width;
    uint32_t screen_height;
    uint32_t vdec_max_buffer_size;
    bool send_vo_again;
    // handle
    CVI_MAPI_DISP_HANDLE_T disp;
    CVI_MAPI_VPROC_HANDLE_T vproc;
    CVI_MAPI_AO_HANDLE_T ao;
    CVI_S32 ao_channel;
    CVI_MAPI_VDEC_HANDLE_T vdec;
    CVI_PLAYER_HANDLE_T player;
    CVI_FILE_RECOVER_HANDLE_T file_recover;
    // event task
    cvi_osal_task_handle_t event_task;
    CVI_MQ_ENDPOINT_HANDLE_t mq_ep;
    // handler
    CVI_PLAYER_SERVICE_EVENT_HANDLER event_handler;
    // signal and slot
    CVI_PLAYER_SERVICE_SIGNALS_S signals;
    CVI_PLAYER_SERVICE_SLOTS_S slots;
} PS_CONTEXT_T;

typedef PS_CONTEXT_T* PS_CONTEXT_HANDLE;

#endif // __PS_CONTEXT_H__
