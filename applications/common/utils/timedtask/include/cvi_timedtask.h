#ifndef __CVI_TIMEDTASK_H__
#define __CVI_TIMEDTASK_H__

#include "cvi_appcomm.h"
#include "cvi_timer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     TIMEDTASK */
/** @{ *//** <!-- [TIMEDTASK] */

typedef struct cviTIMEDTASK_ATTR_S {
    bool bEnable;
    uint32_t u32Time_sec; /**<timed-task trigger time, canbe reset */
    bool periodic;   /**< periodic or ont-shot */
} CVI_TIMEDTASK_ATTR_S;

typedef struct cviTIMEDTASK_CFG_S {
    CVI_TIMEDTASK_ATTR_S stAttr;
    CVI_TIMER_PROC_CALLBACK *timerProc;
    void *pvPrivData;
} CVI_TIMEDTASK_CFG_S;

int32_t CVI_TIMEDTASK_Init(void);
int32_t CVI_TIMEDTASK_DeInit(void);
int32_t CVI_TIMEDTASK_Create(const CVI_TIMEDTASK_CFG_S *pstTimeTskCfg, uint32_t *pTimeTskid);
int32_t CVI_TIMEDTASK_Destroy(uint32_t TimeTskid);
int32_t CVI_TIMEDTASK_GetAttr(uint32_t TimeTskid, CVI_TIMEDTASK_ATTR_S *pstTimeTskAttr);
int32_t CVI_TIMEDTASK_SetAttr(uint32_t TimeTskid, const CVI_TIMEDTASK_ATTR_S *pstTimeTskAttr);
int32_t CVI_TIMEDTASK_ResetTime(uint32_t TimeTskid);

/** @} *//** <!-- ==== TIMEDTASK End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __CVI_TIMEDTASK_H__ */

