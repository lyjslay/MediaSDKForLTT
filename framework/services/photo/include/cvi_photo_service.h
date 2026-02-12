#ifndef __CVI_RS_PHOTO_H__
#define __CVI_RS_PHOTO_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "cvi_mapi.h"
#include "cvi_osal.h"

#include "cvi_recorder.h"
#include "filesync.h"

#include "cvi_log.h"
#include "cvi_mq.h"
#include "cvi_perf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define PHS_SUCCESS ((int32_t)(0))
#define PHS_ERR_FAILURE ((int32_t)(-1001))
#define PHS_ERR_NOMEM ((int32_t)(-1002))
#define PHS_ERR_TIMEOUT ((int32_t)(-1003))
#define PHS_ERR_INVALID ((int32_t)(-1004))

typedef enum _CVI_PHOTO_EVENT_E
{
    CVI_PHOTO_SERVICE_EVENT_OPEN_FILE_FAILED,
    CVI_PHOTO_SERVICE_EVENT_PIV_START,
    CVI_PHOTO_SERVICE_EVENT_PIV_END,
    CVI_PHOTO_SERVICE_EVENT_SYNC_DONE,
    CVI_PHOTO_SERVICE_EVENT_BUTT
} CVI_PHOTO_SERVICE_EVENT_E;

typedef int32_t (*CVI_PHOTO_SERVICE_EVENT_CALLBACK)(CVI_PHOTO_SERVICE_EVENT_E event_type, const char *filename, void *p);

typedef void *CVI_PHOTO_SERVICE_HANDLE_T;

/* photo callback set */
typedef struct _CVI_PHOTO_CALLBACK_S {
    void *pfnNormalPhsCb;
} CVI_PHOTO_SERVICE_CALLBACK_S;

typedef enum CVI_PHOTO_SERVICE_VENC_BIND_MODE_E {
    CVI_PHOTO_SERVICE_VENC_BIND_MODE_NONE,
    CVI_PHOTO_SERVICE_VENC_BIND_MODE_VPSS,
    CVI_PHOTO_SERVICE_VENC_BIND_MODE_VI, // not support yet
    CVI_PHOTO_SERVICE_VENC_BIND_MODE_BUTT
} CVI_PHOTO_SERVICE_VENC_BIND_MODE_E;

typedef struct CVI_MAPI_HANDLES_S{
    CVI_MAPI_VENC_HANDLE_T photo_venc_hdl;
    uint32_t photo_bufsize;

    CVI_MAPI_VPROC_HANDLE_T thumbnail_vproc;
    int32_t vproc_chn_id_thumbnail;
    CVI_MAPI_VENC_HANDLE_T thumbnail_venc_hdl;
    uint32_t thumbnail_bufsize;

    CVI_MAPI_VPROC_HANDLE_T src_vproc;
    int32_t src_vproc_chn_id;

    CVI_MAPI_VPROC_HANDLE_T scale_vproc;
    int32_t scale_vproc_chn_id;
} CVI_PHOTO_SERVICE_MAPI_HANDLES_S;

/* photo attribute param */
typedef struct CVI_PHOTO_ATTR_S {
    CVI_PHOTO_SERVICE_HANDLE_T phs;
    CVI_PHOTO_SERVICE_CALLBACK_S stCallback;
    CVI_S32 s32SnapPresize;
    CVI_PHOTO_SERVICE_MAPI_HANDLES_S handles;
} CVI_PHOTO_SERVICE_ATTR_S;

#ifndef LOG_PHET
#define LOG_PHET(express)                                                         \
    do {                                                                         \
        int32_t ph = express;                                                        \
        if (ph != 0) {                                                           \
            printf("\nFailed at %s: %d  (photo:0x%#x!)\n", __FILE__, __LINE__, ph); \
        }                                                                        \
    } while (0)
#endif

enum _phs_state_bit_e {
    PHS_STATE_PHOTO_CREATE_BIT = 0,
    PHS_STATE_PHOTO_BIT,
    PHS_STATE_PIV_BIT,
    PHS_STATE_PERF_BIT,
    PHS_STATE_DEBUG_BIT,
    PHS_STATE_MUTE_BIT,
    RS_STATE_BIT_MAX
};

#define PHS_STATE_IDLE (0)
#define PHS_STATE_PHOTO_CREATE_EN (1 << PHS_STATE_PHOTO_CREATE_BIT)
#define PHS_STATE_PHOTO_EN (1 << PHS_STATE_PHOTO_BIT)
#define PHS_STATE_PIV_EN (1 << PHS_STATE_PIV_BIT)
#define PHS_STATE_PERF_EN (1 << PHS_STATE_PERF_BIT)
#define PHS_STATE_DEBUG_EN (1 << PHS_STATE_DEBUG_BIT)
#define PHS_STATE_MUTE_EN (1 << PHS_STATE_MUTE_BIT)

typedef struct __phs_context {
    int32_t id;
    void *attr;

    // state
    volatile uint32_t cur_state;
    volatile uint32_t new_state;
    pthread_mutex_t state_mutex;
    volatile int32_t shutdown;

    // video task
    cvi_osal_task_handle_t state_task;
    cvi_osal_task_handle_t piv_task;
    cvi_osal_task_handle_t thumb_task;

    // piv
    volatile int32_t need_thumbnail;
    uint32_t piv_prealloclen;

    char piv_filename[128];
    int32_t piv_finish;
    pthread_mutex_t piv_mutex;
    pthread_cond_t  piv_cond;
    pthread_mutex_t thumbnail_mutex;

} phs_context_t, *phs_context_handle_t;

static inline void phs_wait_state_change(phs_context_handle_t phs) {
    // pooling at 1ms interval
    // TODO: use sem
    do {
        // cvi_osal_task_resched();
        cvi_osal_task_sleep(10 * 1000);
    } while (phs->cur_state != phs->new_state);
}

static inline void phs_change_state(phs_context_handle_t phs, uint32_t new_state) {
    // set new_state
    pthread_mutex_lock(&phs->state_mutex);
    phs->new_state = new_state;
    pthread_mutex_unlock(&phs->state_mutex);
    // wait new_state
    phs_wait_state_change(phs);
}

static inline void phs_enable_state(phs_context_handle_t phs, uint32_t enable_bits) {
    // set new_state
    pthread_mutex_lock(&phs->state_mutex);
    phs->new_state = phs->cur_state | enable_bits;
    pthread_mutex_unlock(&phs->state_mutex);
    // wait new_state
    phs_wait_state_change(phs);
}

static inline void phs_disable_state(phs_context_handle_t phs, uint32_t disable_bits) {
    // set new_state
    pthread_mutex_lock(&phs->state_mutex);
    phs->new_state = phs->cur_state & (~disable_bits);
    pthread_mutex_unlock(&phs->state_mutex);
    // wait new_state
    phs_wait_state_change(phs);
}

typedef struct _cvi_PHOTO_SERVICE_PARAM_S {
    int32_t photo_id;
    uint32_t prealloclen;

    CVI_MAPI_VENC_HANDLE_T photo_venc_hdl;
    uint32_t photo_bufsize;

    CVI_MAPI_VPROC_HANDLE_T thumbnail_vproc;
    int32_t vproc_chn_id_thumbnail;
    CVI_MAPI_VENC_HANDLE_T thumbnail_venc_hdl;
    uint32_t thumbnail_bufsize;

    CVI_MAPI_VPROC_HANDLE_T src_vproc;
    int32_t src_vproc_chn_id;

    CVI_MAPI_VPROC_HANDLE_T scale_vproc;
    int32_t scale_vproc_chn_id;

    void *cont_photo_event_cb;
} CVI_PHOTO_SERVICE_PARAM_S;

#define MAX_CONTEXT_CNT 4

int32_t CVI_PHOTO_SERVICE_Create(CVI_PHOTO_SERVICE_HANDLE_T *hdl, CVI_PHOTO_SERVICE_PARAM_S *param);
int32_t CVI_PHOTO_SERVICE_Destroy(CVI_PHOTO_SERVICE_HANDLE_T hdl);
void CVI_PHOTO_SERVICE_WaitPivFinish(CVI_PHOTO_SERVICE_HANDLE_T hdl);
int32_t CVI_PHOTO_SERVICE_PivCapture(CVI_PHOTO_SERVICE_HANDLE_T hdl, char *file_name);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  // __CVI_RS_PHOTP_H__










