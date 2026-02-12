
#ifndef __CVI_HAL_WATCHDOG_H__
#define __CVI_HAL_WATCHDOG_H__
#include "cvi_type.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int32_t CVI_HAL_WATCHDOG_Init(int32_t s32Time_s);
int32_t CVI_HAL_WATCHDOG_Feed(void);
int32_t CVI_HAL_WATCHDOG_Deinit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_WATCHDOG_H__*/
