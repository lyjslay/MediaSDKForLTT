#ifndef __CVI_WIFIMNG_H__
#define __CVI_WIFIMNG_H__

#include "cvi_appcomm.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int32_t CVI_WIFIMNG_Start(CVI_HAL_WIFI_CFG_S WifiCfg, char *pstDefaultssid);
int32_t CVI_WIFIMNG_Stop(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_WIFIMNG_H__ */