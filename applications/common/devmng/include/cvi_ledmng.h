
/**
* @file    cvi_ledmng.h
* @brief   product ledmng struct and interface
* @version   1.0

*/

#ifndef _CVI_LEDMNG_H
#define _CVI_LEDMNG_H
#include "cvi_hal_led.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
void CVI_LEDMNG_Control(int32_t control);
int32_t CVI_LEDMNG_Init();
int32_t CVI_LEDMNG_DeInit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef _CVI_LEDMNG_H */