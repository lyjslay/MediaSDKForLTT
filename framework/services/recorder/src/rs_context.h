#ifndef __RS_CONTEXT_H__
#define __RS_CONTEXT_H__

#include <stdio.h>
#include <stdlib.h>

#include "cvi_log.h"
#include "cvi_mapi.h"
#include "cvi_mq.h"
#include "cvi_osal.h"
#include "cvi_perf.h"
#include "cvi_recorder.h"
#include "cvi_audio.h"

#include "rs_define.h"
#include "rs_param.h"

#ifndef CHECK_RET
#define CHECK_RET(express)                                                       \
    do {                                                                         \
        int32_t rc = express;                                                        \
        if (rc != 0) {                                                           \
            printf("\nFailed at %s: %d  (rc:0x%#x!)\n", __FILE__, __LINE__, rc); \
            exit(-1);                                                            \
        }                                                                        \
    } while (0)
#endif

#ifndef LOG_RET
#define LOG_RET(express)                                                         \
    do {                                                                         \
        int32_t rc = express;                                                        \
        if (rc != 0) {                                                           \
            printf("\nFailed at %s: %d  (rc:0x%#x!)\n", __FILE__, __LINE__, rc); \
        }                                                                        \
    } while (0)
#endif

enum _rs_state_bit_e {
    RS_STATE_REC_CREATE_BIT = 0,
    RS_STATE_MEM_RECORD_BIT,
    RS_STATE_RECORD_BIT,
    RS_STATE_PIV_BIT,
    RS_STATE_PERF_BIT,
    RS_STATE_DEBUG_BIT,
    RS_STATE_EVENT_RECORD_BIT,
    RS_STATE_STOP_EVENT_RECORD_POST_REC_BIT, // single trigger state, not continuous state
    RS_STATE_TIMELAPSE_RECORD_BIT,
    RS_STATE_MUTE_BIT,
    RS_STATE_BIT_MAX
};

#define RS_STATE_IDLE (0)
#define RS_STATE_REC_CREATE_EN (1 << RS_STATE_REC_CREATE_BIT)
#define RS_STATE_MEM_RECORD_EN (1 << RS_STATE_MEM_RECORD_BIT)
#define RS_STATE_RECORD_EN (1 << RS_STATE_RECORD_BIT)
#define RS_STATE_PIV_EN (1 << RS_STATE_PIV_BIT)
#define RS_STATE_PERF_EN (1 << RS_STATE_PERF_BIT)
#define RS_STATE_DEBUG_EN (1 << RS_STATE_DEBUG_BIT)
#define RS_STATE_EVENT_RECORD_EN (1 << RS_STATE_EVENT_RECORD_BIT)
#define RS_STATE_STOP_EVENT_RECORD_POST_REC_EN (1 << RS_STATE_STOP_EVENT_RECORD_POST_REC_BIT)
#define RS_STATE_TIMELAPSE_RECORD_EN (1 << RS_STATE_TIMELAPSE_RECORD_BIT)
#define RS_STATE_MUTE_EN (1 << RS_STATE_MUTE_BIT)

typedef void (CVI_AUDIO_SERVICR_ACAP_GET_FRAME_CALLBACK) (const AUDIO_FRAME_S *frame, const AEC_FRAME_S *aec_frame, void *arg);

typedef struct __rs_context {
    int32_t id;
    void *attr;

    // state
    volatile uint32_t cur_state;
    volatile uint32_t new_state;
    pthread_mutex_t state_mutex;
    volatile int32_t shutdown;
    pthread_mutex_t param_mutex;
    volatile int32_t param_change;

    // video task
    cvi_osal_task_handle_t video_task;
    cvi_osal_task_handle_t sub_video_task;
    cvi_osal_task_handle_t subtitle_task;
    cvi_osal_task_handle_t state_task;
    cvi_osal_task_handle_t piv_task;
    cvi_osal_task_handle_t thumb_task;
    CVI_RECORDER_HANDLE_T recorder[2];
    FILE *outfp;

    // piv
    volatile int32_t need_thumbnail;
    uint32_t piv_prealloclen;

    char piv_filename[128];
    int32_t piv_finish;
    int32_t piv_shared;
    pthread_mutex_t piv_mutex;
    pthread_cond_t  piv_cond;
    pthread_mutex_t thumbnail_mutex;
    pthread_mutex_t rec_mutex;

    // audio
    CVI_AUDIO_SERVICR_ACAP_GET_FRAME_CALLBACK *acap_cb;

    // event task
    cvi_osal_task_handle_t event_task;
    CVI_MQ_ENDPOINT_HANDLE_t mq_ep;

    // perf stat & mark
    CVI_PERF_MARK_HANDLE_T perf_mark_frame;
    CVI_PERF_STAT_HANDLE_T perf_stat_vcap;
    CVI_PERF_STAT_HANDLE_T perf_stat_venc_rec;
    CVI_PERF_STAT_HANDLE_T perf_stat_rec_save;
    CVI_PERF_STAT_HANDLE_T perf_stat_vproc;
    CVI_PERF_STAT_HANDLE_T perf_stat_disp;

    // misc perf count
    uint64_t venc_rec_stream_total_len;
    uint64_t venc_rec_stream_count;

    int32_t stop_flag;
} rs_context_t, *rs_context_handle_t;

#define RS_STATE_UP(rs, name) \
    ((((rs)->cur_state ^ (rs)->new_state) & RS_STATE_##name##_EN) && ((rs)->new_state & RS_STATE_##name##_EN))
#define RS_STATE_DOWN(rs, name) \
    ((((rs)->cur_state ^ (rs)->new_state) & RS_STATE_##name##_EN) && ((rs)->cur_state & RS_STATE_##name##_EN))
#define RS_STATE_ENABLED(rs, name) ((rs)->cur_state & RS_STATE_##name##_EN)

static inline void rs_wait_state_change(rs_context_handle_t rs) {
    // pooling at 1ms interval
    // TODO: use sem
    do {
        // cvi_osal_task_resched();
        cvi_osal_task_sleep(10 * 1000);
    } while (rs->cur_state != rs->new_state);
}

static inline void rs_change_state(rs_context_handle_t rs, uint32_t new_state) {
    // set new_state
    pthread_mutex_lock(&rs->state_mutex);
    rs->new_state = new_state;
    pthread_mutex_unlock(&rs->state_mutex);
    // wait new_state
    rs_wait_state_change(rs);
}

static inline void rs_enable_state(rs_context_handle_t rs, uint32_t enable_bits) {
    // set new_state
    pthread_mutex_lock(&rs->state_mutex);
    rs->new_state = rs->cur_state | enable_bits;
    pthread_mutex_unlock(&rs->state_mutex);
    // wait new_state
    rs_wait_state_change(rs);
}

static inline void rs_disable_state(rs_context_handle_t rs, uint32_t disable_bits) {
    // set new_state
    pthread_mutex_lock(&rs->state_mutex);
    rs->new_state = rs->cur_state & (~disable_bits);
    pthread_mutex_unlock(&rs->state_mutex);
    // wait new_state
    rs_wait_state_change(rs);
}

#endif  // __RS_CONTEXT_H__
