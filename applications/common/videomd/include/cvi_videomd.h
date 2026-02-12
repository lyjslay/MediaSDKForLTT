#ifndef __CVI_VIDEOMD_H__
#define __CVI_VIDEOMD_H__

#include <unistd.h>

#include "cvi_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** macro define */
#define CVI_VIDEOMD_EINVAL            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VIDEODETECT, CVI_EINVAL)               /**<parm error*/
#define CVI_VIDEOMD_EINTER            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VIDEODETECT, CVI_EINTER)               /**<intern error*/
#define CVI_VIDEOMD_ENOINIT           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VIDEODETECT, CVI_ENOINIT)              /**< no initialize*/
#define CVI_VIDEOMD_EINITIALIZED      CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VIDEODETECT, CVI_EINITIALIZED)         /**< already initialized */
#define CVI_VIDEOMD_EREGISTEREVENT    CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VIDEODETECT, CVI_ERRNO_CUSTOM_BOTTOM)  /**<thread creat or join error*/
#define CVI_VIDEOMD_ETHREAD           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_VIDEODETECT, CVI_ERRNO_CUSTOM_BOTTOM+1)/**<thread creat or join error*/

#define CVI_EVENT_VIDEOMD_CHANGE       CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_VIDEODETECT, 1)

typedef struct _CVI_MOTION_DETECT_ATTR_S{
    int32_t        camid;
    int32_t        state;
    int32_t        threshold;
    CVI_MAPI_VPROC_HANDLE_T    vprocHandle;
    uint32_t                   vprocChnId;
    uint32_t                   isExtVproc;
    uint32_t                   w;
    uint32_t                   h;
} CVI_MOTION_DETECT_ATTR_S;

void CVI_MOTION_DETECT_SetState(int32_t id, int32_t en);
int32_t CVI_MOTION_DETECT_Init(CVI_MOTION_DETECT_ATTR_S *attr);
void CVI_MOTION_DETECT_DeInit(int32_t id);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_VIDEOMD_H__ */