#ifndef __CVI_POWERCONTROL_H__
#define __CVI_POWERCONTROL_H__

#include "cvi_appcomm.h"
#include "cvi_timedtask.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     POWERCONTROL */
/** @{ */  /** <!-- [POWERCONTROL] */
/** macro define */
#define CVI_PWRCTRL_EINVAL            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_EINVAL)    /**<parm invlid*/
#define CVI_PWRCTRL_EINTER            CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_EINTER)    /**<intern error*/
#define CVI_PWRCTRL_ENOINIT           CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_ENOINIT)  /**< no initialize*/
#define CVI_PWRCTRL_EBUSY             CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_EBUSY)  /**<service busy*/
#define CVI_PWRCTRL_EINITIALIZED      CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_EINITIALIZED) /**< already initialized */
#define CVI_PWRCTRL_ETIMEDTASK        CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_ERRNO_CUSTOM_BOTTOM) /**<timedtask module error*/
#define CVI_PWRCTRL_EWAKEUPCB         CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_ERRNO_CUSTOM_BOTTOM+1) /**<wakeup callback error*/
#define CVI_PWRCTRL_EDORMANTCB        CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_ERRNO_CUSTOM_BOTTOM+2) /**<dormant callback error*/
#define CVI_PWRCTRL_ELOGICFLOW        CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_ERRNO_CUSTOM_BOTTOM+3) /**<logical flow error*/
#define CVI_PWRCTRL_EFATA             CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_ERRNO_CUSTOM_BOTTOM+4) /**<fata error*/

/**set screen time > sys time,sys time self-adjusting with screen time;
   set sys time < screen timescreen time self-adjusting with screen time*/
#define CVI_PWRCTRL_ETASKTIMEAUTO     CVI_APPCOMM_ERR_ID(CVI_APP_MOD_PM,CVI_ERRNO_CUSTOM_BOTTOM+5)


/**  ==== power control task configure begin ==== */

typedef int32_t (*CVI_PWRCTRL_TASK_PROC_CALLBACK)(void* pvPrivData);/**< dormant callback process function */

typedef enum cviPWRCTRL_TASK_E /**<pwrctrl task type */
{
    CVI_PWRCTRL_TASK_SCREENDORMANT = 0,
    CVI_PWRCTRL_TASK_SYSTEMDORMANT,
    CVI_PWRCTRL_TASK_BUIT
}CVI_PWRCTRL_TASK_E;

typedef struct cviPWRCTRL_TASK_CFG__S /**< task static attribute */
{
    CVI_TIMEDTASK_ATTR_S stAttr;
    CVI_PWRCTRL_TASK_PROC_CALLBACK pfnDormantProc;
    void* pvDormantPrivData;
    CVI_PWRCTRL_TASK_PROC_CALLBACK pfnWakeupProc;
    void* pvWakeupPrivData;
} CVI_PWRCTRL_TASK_CFG_S;

typedef struct cviPWRCTRL_CFG__S
{
    CVI_PWRCTRL_TASK_CFG_S astTaskCfg[CVI_PWRCTRL_TASK_BUIT];
} CVI_PWRCTRL_CFG_S;

/** ==== power control task attribute end===*/


/**  ==== event configuue begin ==== */
typedef enum cviPWRCTRL_EVENT_TYPE_E /**<event type */
{
    CVI_PWRCTRL_EVENT_TYPE_WAKEUP = 0,    /**<wakeup dormant*/
    CVI_PWRCTRL_EVENT_TYPE_CONTROL,       /**<pause or resumme dormant check */
    CVI_PWRCTRL_EVENT_TYPE_COMMON,        /**<general event,no both of wakeup and control function */
    CVI_PWRCTRL_EVENT_TYPE_BUIT
}CVI_PWRCTRL_EVENT_TYPE_E;


typedef enum cviPWRCTRL_EVENT_SCOPE_E /**< event action scope */
{
    /**control event: control(pause or resume) system dormant check at normal state(no system dormant)
       common event:  do not continue handle the event at sys dormant
       wakeup event:  wakeup sstem at sys dormant  */
    CVI_PWRCTRL_EVENT_SCOPE_SYSTEM = 0,
    /**control event: control(pause or resume) system dormant and screen dormant check at normal state
       common event:  do not continue handle the event at sys dormant or screen dormant
       wakeup event:  wakeup sstem at sys dormant or screen dormant */
    CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN,
    CVI_PWRCTRL_EVENT_SCOPE_BUIT
}CVI_PWRCTRL_EVENT_SCOPE_E;


typedef struct cviPWRCTRL_EVENT_COMMON_ATTR_S/**< common event attribute */
{
    CVI_PWRCTRL_EVENT_SCOPE_E enType;
    bool bResetTimer;
} CVI_PWRCTRL_EVENT_COMMON_ATTR_S;


typedef enum cviPWRCTRL_WAKEUP_TACTICS_E /**< wakeup event process tactics type,whether event need continue proc or not*/
{
    CVI_PWRCTRL_WAKEUP_TACTICS_DISCARD = 0, /**<after dong wakeup,event need not continue to proc*/
    CVI_PWRCTRL_WAKEUP_TACTICS_CONTINUE,    /**<after dong wakeup,event need continue to proc*/
    CVI_PWRCTRL_WAKEUP_TACTICS_BUIT
}CVI_PWRCTRL_WAKEUP_TACTICS_E;


typedef struct cviPWRCTRL_EVENT_WAKEUP_ATTR_S/**< wakeup event attribute */
{
    CVI_PWRCTRL_WAKEUP_TACTICS_E enType;
    CVI_PWRCTRL_EVENT_COMMON_ATTR_S stCommonCfg;
} CVI_PWRCTRL_EVENT_WAKEUP_ATTR_S;


typedef enum cviPWRCTRL_EVENT_CONTROL_E/**< control event type of action*/
{
    CVI_PWRCTRL_EVENT_CONTROL_PAUSE = 0,/**<after wakeup ,ui need not to proc*/
    CVI_PWRCTRL_EVENT_CONTROL_RESUME,/**<after wakeup ,ui continue to proc*/
    CVI_PWRCTRL_EVENT_CONTROL_BUIT
}CVI_PWRCTRL_EVENT_CONTROL_E;



typedef struct cviPWRCTRL_EVENT_CONTROL_ATTR_S/**< control event attribute*/
{
    CVI_PWRCTRL_EVENT_CONTROL_E enType;
    CVI_PWRCTRL_EVENT_COMMON_ATTR_S stCommonCfg;
} CVI_PWRCTRL_EVENT_CONTROL_ATTR_S;


typedef struct cviPWRCTRL_EVENT_ATTR_S/**< event configure */
{
    CVI_PWRCTRL_EVENT_TYPE_E enType;
    union tagPWRCTRL_EVENT_ATTR_U
    {
        CVI_PWRCTRL_EVENT_WAKEUP_ATTR_S stWakeupCfg;/**<wakeup event cfg*/
        CVI_PWRCTRL_EVENT_CONTROL_ATTR_S stCtrlCfg;/**<control event cfg*/
        CVI_PWRCTRL_EVENT_COMMON_ATTR_S stCommonCfg;
    } unCfg;
} CVI_PWRCTRL_EVENT_ATTR_S;

/**  ==== event configuue end ==== */
int32_t CVI_POWERCTRL_Init(const CVI_PWRCTRL_CFG_S* pstCfg);
int32_t CVI_POWERCTRL_GetTaskAttr(CVI_PWRCTRL_TASK_E enType,CVI_TIMEDTASK_ATTR_S* pstTaskAttr);
int32_t CVI_POWERCTRL_SetTaskAttr(CVI_PWRCTRL_TASK_E enType,const CVI_TIMEDTASK_ATTR_S* pstTaskAttr);
int32_t CVI_POWERCTRL_EventPreProc(const CVI_PWRCTRL_EVENT_ATTR_S* pstEventAttr,bool* pbEventContinueHandle);
int32_t CVI_POWERCTRL_DeInit(void);

/** @}*/  /** <!-- ==== POWERCONTROL End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_WIFI_H__*/
