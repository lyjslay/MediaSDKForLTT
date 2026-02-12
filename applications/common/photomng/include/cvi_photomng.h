#ifndef __CVI_PHOTOMNG_H__
#define __CVI_PHOTOMNG_H__

#include "cvi_appcomm.h"
#include "cvi_photo_service.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum cviEVENT_PHOTOMNG_E {
    CVI_EVENT_PHOTOMNG_OPEN_FAILED = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_PHOTOMNG, 0),
    CVI_EVENT_PHOTOMNG_PIV_START,
    CVI_EVENT_PHOTOMNG_PIV_END,
    CVI_EVENT_PHOTOMNG_BUTT
} CVI_EVENT_PHOTOMNG_E;

int32_t CVI_PHOTOMNG_ContCallBack(CVI_PHOTO_SERVICE_EVENT_E event_type, const char *filename, void *param);
int32_t CVI_POHTOMNG_RegisterEvent(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_RECORDMNG_H__ */