#ifndef __CVI_RECORDMNG_H__
#define __CVI_RECORDMNG_H__

#include "cvi_appcomm.h"
#include "cvi_recorder.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** Error Code */
#define CVI_RECMNG_EINVAL          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_RECMNG, CVI_EINVAL)                  /**<Invalid argument */
#define CVI_RECMNG_ENOTINIT        CVI_APPCOMM_ERR_ID(CVI_APP_MOD_RECMNG, CVI_ENOINIT)                 /**<Not inited */
#define CVI_RECMNG_EINITIALIZED    CVI_APPCOMM_ERR_ID(CVI_APP_MOD_RECMNG, CVI_EINITIALIZED)            /**<Already Initialized */
#define CVI_RECMNG_EINTER          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_RECMNG, CVI_EINTER)                  /**<Already Initialized */
#define CVI_RECMNG_EREGISTER_EVENT CVI_APPCOMM_ERR_ID(CVI_APP_MOD_RECMNG, CVI_ERRNO_CUSTOM_BOTTOM)     /**<register event failed */
#define CVI_RECMNG_EMAXINSTANCE    CVI_APPCOMM_ERR_ID(CVI_APP_MOD_RECMNG, CVI_ERRNO_CUSTOM_BOTTOM + 1) /**<beyond maximum instance */
#define CVI_RECMNG_ESTORAGE        CVI_APPCOMM_ERR_ID(CVI_APP_MOD_RECMNG, CVI_ERRNO_CUSTOM_BOTTOM + 2) /**<storage interface error */

/** event ID define */
typedef enum cviEVENT_RECMNG_E {
    CVI_EVENT_RECMNG_STARTREC = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_RECMNG, 0),
    CVI_EVENT_RECMNG_STOPREC,
    CVI_EVENT_RECMNG_SPLITSTART,
    CVI_EVENT_RECMNG_SPLITREC,
    CVI_EVENT_RECMNG_STARTEVENTREC,
    CVI_EVENT_RECMNG_EVENTREC_END,
    CVI_EVENT_RECMNG_STARTEMRREC,
    CVI_EVENT_RECMNG_EMRREC_END,
    CVI_EVENT_RECMNG_OPEN_FAILED,
    CVI_EVENT_RECMNG_WRITE_ERROR,
    CVI_EVENT_RECMNG_PIV_START,
    CVI_EVENT_RECMNG_PIV_END,
    CVI_EVENT_RECMNG_BUTT
} CVI_EVENT_RECMNG_E;

int32_t CVI_RECORDMNG_EventCallBack(CVI_RECORDER_EVENT_E event_type, const char *filename, void *param);
int32_t CVI_RECORDMNG_ContCallBack(CVI_RECORDER_EVENT_E event_type, const char *filename, void *param);
int32_t CVI_RECORDMNG_GetSubtitleCallBack(void *p, char *str, int32_t str_len);
int32_t CVI_RECORDMNG_RegisterEvent(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_RECORDMNG_H__ */