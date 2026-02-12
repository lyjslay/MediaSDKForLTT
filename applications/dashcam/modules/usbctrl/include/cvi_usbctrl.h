#ifndef __CVI_USBCTRL_H__
#define __CVI_USBCTRL_H__
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


int32_t  CVI_USBCTRL_Init(void);
int32_t  CVI_USBCTRL_Deinit(void);
int32_t  CVI_USBCTRL_RegisterEvent(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif