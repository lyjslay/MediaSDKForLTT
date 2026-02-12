#ifndef __CVI_ADAS_SERVICE_H__
#define __CVI_ADAS_SERVICE_H__

#include <stdint.h>
#include "cvi_osal.h"
#include "cvi_mapi.h"
#include "cvi_tdl_app.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define GOTO_IF_FAILED(func, result, label)                              \
  do {                                                                   \
    result = (func);                                                     \
    if (result != CVI_SUCCESS) {                                         \
      printf("failed! ret=%#x, at %s:%d\n", result, __FILE__, __LINE__); \
      goto label;                                                        \
    }                                                                    \
  } while (0)

typedef int32_t (*CVI_ADAS_SERVICE_VOICE_CALLBACK)(int32_t index);
typedef int32_t (*CVI_ADAS_SERVICE_LABEL_CALLBACK)(int32_t camid, int32_t index, uint32_t count, char* coordinates);
typedef int32_t (*CVI_ADAS_SERVICE_LABEL_OSDC_CALLBACK)(int32_t camid, uint32_t car_count, char* car_coordinates, uint32_t lane_count, char* lane_coordinates);
typedef void *CVI_ADAS_SERVICE_HANDLE_T;

typedef struct _CVI_ADAS_CALLBACK_S {
    void *pfnVoiceCb;
    void *pfnLabelCb;
    void *pfnLabelOsdcCb;
} CVI_ADAS_SERVICE_CALLBACK_S;

typedef struct _CVI_ADAS_SERVICE_VPROC_S{
    CVI_MAPI_VPROC_HANDLE_T    vprocHandle;
    int32_t                    vprocId;
    uint32_t                   vprocChnId;
    // uint32_t                   isExtVproc;
} CVI_ADAS_SERVICE_VPROC_ATTR_S;

typedef struct _CVI_ADAS_SERVICE_MODEL_ATTR_S{
    float   fps;
    int32_t width;
    int32_t height;
    char CarModelPath[128];
    char LaneModelPath[128];
} CVI_ADAS_SERVICE_MODEL_ATTR_S;

typedef struct _CVI_ADAS_SERVICE_ATTR_S {
    CVI_ADAS_SERVICE_HANDLE_T       ADASHdl;
    CVI_ADAS_SERVICE_CALLBACK_S     stADASCallback;
    CVI_ADAS_SERVICE_VPROC_ATTR_S   stVprocAttr;
    CVI_ADAS_SERVICE_MODEL_ATTR_S   stADASModelAttr;
} CVI_ADAS_SERVICE_ATTR_S;

typedef struct _CVI_ADAS_SERVICE_PARAM_S{
    int32_t                         camid;
    CVI_ADAS_SERVICE_MODEL_ATTR_S   stADASModelParam;
    CVI_ADAS_SERVICE_VPROC_ATTR_S   stVPSSParam;
    void                            *adas_voice_event_cb;
    void                            *adas_label_event_cb;
    void                            *adas_label_osdc_event_cb;
} CVI_ADAS_SERVICE_PARAM_S;

typedef struct _CVI_ADAS_SERVICE_CTX_S{
    int32_t                         id;
    int32_t                         state;
    void                            *attr;
    cvi_osal_task_handle_t          adas_task;
    pthread_mutex_t                 adas_mutex;
    cvitdl_app_handle_t             app_handle;
    cvitdl_handle_t                 tdl_handle;
}CVI_ADAS_SERVICE_CTX_S, *ADAS_CONTEXT_HANDLE_S;

typedef enum _CVI_ADAS_SERVICE_CMD_E
{
    CVI_ADAS_SERVICE_NORMAL = 0,
    CVI_ADAS_SERVICE_CAR_MOVING,
    CVI_ADAS_SERVICE_CAR_CLOSING,
    CVI_ADAS_SERVICE_CAR_COLLISION,
    CVI_ADAS_SERVICE_CAR_LANE,
    CVI_ADAS_SERVICE_LABEL_CAR,
    CVI_ADAS_SERVICE_LABEL_LANE,
    CVI_ADAS_SERVICE_BUTT
} CVI_ADAS_SERVICE_CMD_E;

int32_t CVI_ADAS_SERVICE_Destroy(CVI_ADAS_SERVICE_HANDLE_T hdl);
int32_t CVI_ADAS_SERVICE_Create(CVI_ADAS_SERVICE_HANDLE_T *hdl, CVI_ADAS_SERVICE_PARAM_S *ADASParam);
void CVI_ADAS_SERVICE_SetState(int32_t id, int32_t en);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_ADAS_AERVICE_H__ */