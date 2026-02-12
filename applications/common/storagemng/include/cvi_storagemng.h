#ifndef _CVI_STORAGEMNG_H_
#define _CVI_STORAGEMNG_H_

#include "cvi_appcomm.h"
#include "cvi_stg.h"
#include "cvi_osal.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define MAX_STORAGE_FILENAME_LEN        (64)

/** Error Code */
#define CVI_STORAGEMNG_EINVAL          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_STORAGEMNG, CVI_EINVAL)                  /**<Invalid argument */
#define CVI_STORAGEMNG_ENOTINIT        CVI_APPCOMM_ERR_ID(CVI_APP_MOD_STORAGEMNG, CVI_ENOINIT)                 /**<Not inited */
#define CVI_STORAGEMNG_EINITIALIZED    CVI_APPCOMM_ERR_ID(CVI_APP_MOD_STORAGEMNG, CVI_EINITIALIZED)            /**<Already Initialized */
#define CVI_STORAGEMNG_EINTER          CVI_APPCOMM_ERR_ID(CVI_APP_MOD_STORAGEMNG, CVI_EINTER)                  /**<Already Initialized */
#define CVI_STORAGEMNG_EREGISTER_EVENT CVI_APPCOMM_ERR_ID(CVI_APP_MOD_STORAGEMNG, CVI_ERRNO_CUSTOM_BOTTOM)     /**<register event failed */
#define CVI_STORAGEMNG_EMAXINSTANCE    CVI_APPCOMM_ERR_ID(CVI_APP_MOD_STORAGEMNG, CVI_ERRNO_CUSTOM_BOTTOM + 1) /**<beyond maximum instance */
#define CVI_STORAGEMNG_ESTORAGE        CVI_APPCOMM_ERR_ID(CVI_APP_MOD_STORAGEMNG, CVI_ERRNO_CUSTOM_BOTTOM + 2) /**<storage interface error */

/** Event */
typedef enum cviEVENT_STORAGEMNG_E {
    CVI_EVENT_STORAGEMNG_DEV_UNPLUGED = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_STORAGEMNG, 0),
    CVI_EVENT_STORAGEMNG_DEV_CONNECTING,
    CVI_EVENT_STORAGEMNG_DEV_ERROR,
    CVI_EVENT_STORAGEMNG_FS_CHECKING,
    CVI_EVENT_STORAGEMNG_FS_CHECK_FAILED,
    CVI_EVENT_STORAGEMNG_FS_EXCEPTION,
    CVI_EVENT_STORAGEMNG_MOUNTED,
    CVI_EVENT_STORAGEMNG_MOUNT_FAILED,
    CVI_EVENT_STORAGEMNG_MOUNT_READ_ONLY,
    CVI_EVENT_STORAGEMNG_START_UPFILE,
    CVI_EVENT_STORAGEMNG_UPFILE_SUCCESSED,
    CVI_EVENT_STORAGEMNG_UPFILE_FAIL,
    CVI_EVENT_STORAGEMNG_UPFILE_FAIL_FILE_ERROR,
    CVI_EVENT_STORAGEMNG_BUTT
} CVI_EVENT_STORAGEMNG_E;

int32_t CVI_STORAGEMNG_RegisterEvent(void);
int32_t CVI_STORAGEMNG_Create(STG_DEVINFO_S *devinfo);
int32_t CVI_STORAGEMNG_Destroy(void);
int32_t CVI_STORAGEMNG_GetFSInfo(CVI_STG_FS_INFO_S *pstFSInfo);
int32_t CVI_STORAGEMNG_GetInfo(CVI_STG_DEV_INFO_S *pstInfo);
int32_t CVI_STORAGEMNG_GetDevInfo(STG_DEVINFO_S *pstDevInfo);
int32_t CVI_STORAGEMNG_Format(char *labelname);
int32_t CVI_STORAGEMNG_Mount(void);
int32_t CVI_STORAGEMNG_Umount(void);
int32_t CVI_STORAGEMNG_Check_Space(void);

/** @} *//** <!-- ==== STORAGEMNG End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef _CVI_STORAGEMNG_H_ */

