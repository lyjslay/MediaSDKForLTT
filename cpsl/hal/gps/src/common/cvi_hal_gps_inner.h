/**
* @file    cvi_hal_gps.h
* @brief   product hal gps interface
*
*
* @author    cvitek cardv team
* @date      2021/6/19
* @version

*/
#ifndef __CVI_HAL_GPS_INNER_H__
#define __CVI_HAL_GPS_INNER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#include "cvi_hal_gps.h"

typedef struct cviGPSHAL_DEVICE
{
   int32_t (*Init)(void);
   int32_t (*DeInit)(void);
   int32_t (*GetRawData)(CVI_GPSDATA_S *gpsData, int32_t timeout_ms);
}CVI_GPSHAL_DEVICE;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_GPS_INNER_H__*/
