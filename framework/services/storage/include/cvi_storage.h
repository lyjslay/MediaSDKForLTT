#ifndef __CVI_SDCARD_H__
#define __CVI_SDCARD_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "cvi_mq.h"
#include "cvi_stg.h"

#define STORAGE_LABEL_LEN   (64)
#ifndef CHECK_RET
#define CHECK_RET(express)                                                       \
    do {                                                                         \
        int32_t rc = express;                                                        \
        if (rc != 0) {                                                           \
            printf("\nFailed at %s: %d  (rc:0x%#x!)\n", __FILE__, __LINE__, rc); \
        }                                                                        \
    } while (0)
#endif
typedef int32_t (*CVI_STORAGE_SERVICE_EVENT_CALLBACK)(CVI_STG_STATE_E state);

typedef struct _CVI_STORAGE_SERVICE_PARAM_S {
    STG_DEVINFO_S   devinfo;
    char            labelname[STORAGE_LABEL_LEN];
    CVI_STORAGE_SERVICE_EVENT_CALLBACK            storage_event_callback;

} CVI_STORAGE_SERVICE_PARAM_S;

typedef void *CVI_STORAGE_SERVICE_HANDLE_T;
typedef CVI_STORAGE_SERVICE_PARAM_S stg_param_t, *stg_param_handle_t;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t CVI_STORAGE_SERVICE_Create(CVI_STORAGE_SERVICE_HANDLE_T *hdl, CVI_STORAGE_SERVICE_PARAM_S *params);
int32_t CVI_STORAGE_SERVICE_Destroy(CVI_STORAGE_SERVICE_HANDLE_T hdl);
int32_t CVI_STORAGE_SERVICE_GetFsInfo(CVI_STORAGE_SERVICE_HANDLE_T hdl, CVI_STG_FS_INFO_S *pstInfo);
int32_t CVI_STORAGE_SERVICE_GetDevInfo(CVI_STORAGE_SERVICE_HANDLE_T hdl, CVI_STG_DEV_INFO_S *pstInfo);
int32_t CVI_STORAGE_SERVICE_Format(CVI_STORAGE_SERVICE_HANDLE_T hdl, char *labelname);
int32_t CVI_STORAGE_SERVICE_Mount(CVI_STORAGE_SERVICE_HANDLE_T hdl);
int32_t CVI_STORAGE_SERVICE_Umount(CVI_STORAGE_SERVICE_HANDLE_T hdl);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif