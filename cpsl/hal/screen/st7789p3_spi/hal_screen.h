#ifndef __HAL_SCREEN_H__
#define __HAL_SCREEN_H__


#include "cvi_hal_gpio.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define  SCREEN_TYPE  CVI_HAL_SCREEN_INTF_TYPE_LCD

#define BACK_LIGHT	CVI_GPIOC_12
#define REST_LIGHT	CVI_GPIOB_11
#define POWER_LIGHT	CVI_GPIOC_12

#define RESET_DELAY (50 * 1000)
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* __HAL_SCREEN_H__  */