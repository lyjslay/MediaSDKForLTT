#ifndef __CVI_LIVEVIEW_H__
#define __CVI_LIVEVIEW_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "cvi_mapi.h"
#include "cvi_mq.h"

#ifndef CHECK_RET
#define CHECK_RET(express)                                                       \
    do {                                                                         \
        int32_t rc = express;                                                        \
        if (rc != 0) {                                                           \
            printf("\nFailed at %s: %d  (rc:0x%#x!)\n", __FILE__, __LINE__, rc); \
        }                                                                        \
    } while (0)
#endif

#define CVI_CMD_CLIENT_ID_LIVEVIEW (CVI_MQ_CLIENT_ID_SVC_1)

#define CVI_CMD_CHANNEL_ID_LIVEVIEW(liveview_id) (0x00 + (liveview_id))

typedef enum cvi_cmd_liveview_e {
    CVI_CMD_LIVEVIEW_INVALID = 0,
    CVI_CMD_LIVEVIEW_SHUTDOWN,
    CVI_CMD_LIVEVIEW_SWITCH,
    CVI_CMD_LIVEVIEW_MOVEUP,
    CVI_CMD_LIVEVIEW_MOVEDOWN,
    CVI_CMD_LIVEVIEW_MIRROR,
    CVI_CMD_LIVEVIEW_FILP,
    CVI_CMD_LIVEVIEW_ADJUSTFOCUS,
    CVI_CMD_LIVEVIEW_MAX
} cvi_cmd_liveview_t;

typedef struct CVI_LIVEVIEW_SERVICE_WNDATTR_S {
    bool        WndEnable;
    bool        UsedCrop;
    bool        SmallWndEnable;
    uint32_t    BindVprocId;
    uint32_t    BindVprocChnId;
    uint32_t    WndX;
    uint32_t    WndY;
    uint32_t    WndWidth;
    uint32_t    WndHeight;
    uint32_t    WndsWidth;
    uint32_t    WndsHeight;
    uint32_t    WndsX;
    uint32_t    WndsY;
    uint32_t    OneStep;
    uint32_t    WndMirror;
    uint32_t    WndFilp;
    int32_t     yStep;
    /* new */
    float ratio;
    uint32_t Yoffset;
    uint32_t Xoffset;
} CVI_LIVEVIEW_SERVICE_WNDATTR_S;

typedef struct _CVI_LIVEVIEW_SERVICE_ATTR_S {
    CVI_LIVEVIEW_SERVICE_WNDATTR_S  wnd_attr;
    CVI_MAPI_VPROC_HANDLE_T         vproc_hdl;
} CVI_LIVEVIEW_SERVICE_ATTR_S;

typedef struct _CVI_LIVEVIEW_SERVICE_PARAM_S {
    uint32_t    WndCnt;
    VPSS_GRP vproc_id;
    VB_POOL hVbPool;
    CVI_LIVEVIEW_SERVICE_ATTR_S LiveviewService[CVI_DISP_MAX_WND_NUM];
} CVI_LIVEVIEW_SERVICE_PARAM_S;

typedef void *CVI_LIVEVIEW_SERVICE_HANDLE_T;
typedef CVI_LIVEVIEW_SERVICE_PARAM_S lv_param_t, *lv_param_handle_t;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t CVI_LIVEVIEW_SERVICE_Create(CVI_LIVEVIEW_SERVICE_HANDLE_T *hdl, CVI_LIVEVIEW_SERVICE_PARAM_S *params);
int32_t CVI_LIVEVIEW_SERVICE_Destroy(CVI_LIVEVIEW_SERVICE_HANDLE_T hdl);
int32_t CVI_LIVEVIEW_SERVICE_GetParam(CVI_LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId, CVI_LIVEVIEW_SERVICE_WNDATTR_S *WndParam);
int32_t CVI_LIVEVIEW_SERVICE_SetStepY(CVI_LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId,int32_t step,int32_t* lastStep);
int32_t CVI_LIVEVIEW_SERVICE_GetStepY(CVI_LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId,int32_t* lastStep);
int32_t CVI_LIVEVIEW_SERVICE_AddStepY(CVI_LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId, int32_t step,int32_t* lastStep);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif