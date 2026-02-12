#ifndef __CVI_GAUGERMNG_H__
#define __CVI_GAUGERMNG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "cvi_appcomm.h"

typedef enum cviEVENT_GAUGEMNG_E
{
    CVI_EVENT_GAUGEMNG_LEVEL_CHANGE = CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_GAUGEMNG, 0), /**<refresh current count of electric quantity*/
    CVI_EVENT_GAUGEMNG_LEVEL_LOW,        /**<low level , an alarm show*/
    CVI_EVENT_GAUGEMNG_LEVEL_ULTRALOW,   /**<ultra low level , power off*/
    CVI_EVENT_GAUGEMNG_LEVEL_NORMAL,      /**<after charging,restore normal*/
    CVI_EVENT_GAUGEMNG_CHARGESTATE_CHANGE,      /**<after charging,restore normal*/
    CVI_EVENT_GAUGEMNG_BUIT
} CVI_EVENT_GAUGEMNG_E;

/** gauge mng configure */
typedef struct cviGAUGEMNG_CFG_S
{
    int32_t s32LowLevel; /**< in percent */
    int32_t s32UltraLowLevel; /**< in percent */
} CVI_GAUGEMNG_CFG_S;

int32_t CVI_GAUGEMNG_GetPercentage(void);
int32_t CVI_GAUGEMNG_Init(const CVI_GAUGEMNG_CFG_S* pstCfg);
int32_t CVI_GAUGEMNG_GetBatteryLevel(uint8_t* ps32Level);
int32_t CVI_GAUGEMNG_GetChargeState(bool* pbCharge);
int32_t CVI_GAUGEMNG_DeInit(void);
int32_t CVI_GAUGEMNG_RegisterEvent(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_GAUGEMNG_H__ */