/**
 * @file      cvi_usb_storage.h
 * @brief     usb storage mode interface
 *

 */

#ifndef __CVI_USB_STORAGE_H__
#define __CVI_USB_STORAGE_H__

#include <sys/types.h>

/** usb storage configure */
typedef struct cviUSB_STORAGE_CFG_S
{
    char szProcFile[CVI_APPCOMM_MAX_PATH_LEN]; /**<usb state proc file */
    char szSysFile[CVI_APPCOMM_MAX_PATH_LEN]; /**<usb storage device system file */
    char szDevPath[CVI_APPCOMM_MAX_PATH_LEN]; /**<usb storage device path(sd device path) */
} CVI_USB_STORAGE_CFG_S;


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     USB */
/** @{ */  /** <!-- [USB] */

/**
 * @brief     usb storage mode init, load module
 * @param[in] pszDevPath : storage dev path
 * @return    0 success,non-zero error code.
 * @exception None
 */
int32_t USB_STORAGE_Init(const char *pszDevPath);

/**
 * @brief     usb storage mode deinit, unload module
 * @return    0 success,non-zero error code.
 * @exception None
 */
int32_t USB_STORAGE_Deinit(void);

/**
 * @brief     prepare storage device
 * @param[in] pstCfg : storage configure
 * @return    0 success,non-zero error code.
 * @exception None
 */
int32_t USB_STORAGE_PrepareDev(const CVI_USB_STORAGE_CFG_S* pstCfg);

/** @}*/  /** <!-- ==== USB End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __CVI_USB_STORAGE_H__ */
