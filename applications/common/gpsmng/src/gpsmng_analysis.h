#ifndef __GPSMNG_ANALYSIS_H__
#define __GPSMNG_ANALYSIS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#include"cvi_gpsmng.h"

#define CVI_GPSMNG_MESSAGE_MAX_LEN (120)

/* GPS RAW DATA*/
typedef struct tagGPSMNG_RAW_DATA
{
    char ggaStr[CVI_GPSMNG_MESSAGE_MAX_LEN];
    char gllStr[CVI_GPSMNG_MESSAGE_MAX_LEN];
    char gsaStr[CVI_GPSMNG_MESSAGE_MAX_LEN];
    char rmcStr[CVI_GPSMNG_MESSAGE_MAX_LEN];
    char vtgStr[CVI_GPSMNG_MESSAGE_MAX_LEN];
    char gsvStr[CVI_GPSMNG_GSV_MAX_MSG_NUM][CVI_GPSMNG_MESSAGE_MAX_LEN];
}GPSMNG_RAW_DATA;

int32_t nmea_parse_rawdata(GPSMNG_RAW_DATA* gpsRawData, CVI_GPSMNG_MSG_PACKET* gpsMsgPack);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_GPS_H__*/
