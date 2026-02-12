/**
* @file    cvi_hal_gps.h
* @brief    hal gps interface
*
*
* @author    cvitek team
* @date      2021/6/19
* @version

*/
#ifndef __CVI_HAL_GPS_H__
#define __CVI_HAL_GPS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define CVI_HAL_GPS_DATA_SIZE 1

typedef struct cviGPSDATA
{
    uint32_t wantReadLen;   /**want read length */
    uint32_t actualReadLen; /**actual read length */
    unsigned char  rawData[CVI_HAL_GPS_DATA_SIZE];
} CVI_GPSDATA_S;

int32_t CVI_HAL_GPS_Init(void);

int32_t CVI_HAL_GPS_Deinit(void);

int32_t CVI_HAL_GPS_GetRawData(CVI_GPSDATA_S *gpsData, int32_t timeout_ms);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_GPS_H__*/
