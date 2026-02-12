#ifndef __CVI_ISP_IRCUT_H__
#define __CVI_ISP_IRCUT_H__

#include <unistd.h>
#include "cvi_param.h"
#include "cvi_osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define CVI_EVENT_ISP_IR      CVI_APPCOMM_EVENT_ID(CVI_APP_MOD_ISP_IR, 1)

int32_t CVI_ISPIR_Init(CVI_PARAM_ISPIR_ATTR_S *ISPIR);
int32_t CVI_ISPIR_DeInit(void);
void CVI_ISPIR_ControlModeSelect(int32_t cam_id, int32_t value);
void CVI_ISPIR_ManualCtrl(int32_t cam_id, int32_t state);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __CVI_ISP_EXP_H__ */