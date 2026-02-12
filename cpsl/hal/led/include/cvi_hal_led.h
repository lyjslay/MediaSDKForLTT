/**
* @file    cvi_hal_key.h
* @brief   product hal key interface
*/
#ifndef __CVI_HAL_LED_H__
#define __CVI_HAL_LED_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include "cvi_hal_gpio.h"

/** \addtogroup     HAL_LED */

typedef enum cviHAL_LED_STATE_E
{
    CVI_HAL_LED_STATE_H = 0,/**<Led high state*/
    CVI_HAL_LED_STATE_L, /**<led low state*/
    CVI_HAL_LED_STATE_BUIT
} CVI_HAL_LED_STATE_E;

typedef enum cviHAL_LED_IDX_E
{
    CVI_HAL_LED_IDX_0 = 0, /**<led index 0*/
    CVI_HAL_LED_IDX_1,
    CVI_HAL_LED_IDX_BUIT
} CVI_HAL_LED_IDX_E;

typedef struct cviGPIO_ID_E {
	CVI_HAL_LED_IDX_E id;
	int32_t gpioid;
} CVI_LED_GPIO_ID_E;

int32_t CVI_HAL_LED_Init(void);
int32_t CVI_HAL_LED_GetState(CVI_HAL_LED_IDX_E enKeyIdx, CVI_HAL_LED_STATE_E* penKeyState);
int32_t CVI_HAL_LED_Deinit(void);
int32_t CVI_HAL_LED_SetValue(CVI_GPIO_NUM_E gpio, CVI_GPIO_VALUE_E value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_LED_H__*/