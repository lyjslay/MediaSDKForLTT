/**
 * @file      ui_powercontrol.c
 * @brief     ui powercontrol code. change power control strategy here.
 *
 * Copyright (c) 2017 Huawei Tech.Co.,Ltd
 *
 * @author    HiMobileCam Reference Develop Team
 * @date      2018/2/24
 * @version   1.0

 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include "ui_common.h"
#include "cvi_keymng.h"
#include "cvi_powercontrol.h"
#include "cvi_hal_screen.h"
#include "cvi_hal_touchpad.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


static bool s_bPowerCtrlInited = false;


int32_t  UI_POWERCTRL_DormantScreen(void* pvPrivData)
{
    CVI_LOGD("Sleep Screen\n");
    int32_t  s32Ret = 0;

    CVI_MESSAGE_S stMsg = {};
    stMsg.topic = CVI_EVENT_MODEMNG_SCREEN_DORMANT;
    stMsg.arg1 = true;
    s32Ret = CVI_MODEMNG_SendMessage(&stMsg);
    CVI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return 0;
}

int32_t  UI_POWERCTRL_WakeupScreen(void* pvPrivData)
{
    CVI_LOGD("Wakeup Screen\n");
    int32_t  s32Ret = 0;

    CVI_MESSAGE_S stMsg = {};
    stMsg.topic = CVI_EVENT_MODEMNG_SCREEN_DORMANT;
    stMsg.arg1 = false;
    s32Ret = CVI_MODEMNG_SendMessage(&stMsg);
    CVI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    return 0;
}

int32_t  UI_POWERCTRL_Init(void)
{
    int32_t  s32Ret = 0;
    CVI_PWRCTRL_CFG_S stPowerCtrlCfg = {};
    stPowerCtrlCfg.astTaskCfg[CVI_PWRCTRL_TASK_SCREENDORMANT].pfnDormantProc = UI_POWERCTRL_DormantScreen;
    stPowerCtrlCfg.astTaskCfg[CVI_PWRCTRL_TASK_SCREENDORMANT].pvDormantPrivData = NULL;
    stPowerCtrlCfg.astTaskCfg[CVI_PWRCTRL_TASK_SCREENDORMANT].pfnWakeupProc = UI_POWERCTRL_WakeupScreen;
    stPowerCtrlCfg.astTaskCfg[CVI_PWRCTRL_TASK_SCREENDORMANT].pvWakeupPrivData = NULL;

    if (s_bPowerCtrlInited) {
        CVI_LOGD("Power Control Inited\n");
        return 0;
    }
    int32_t  Value = 0;
    CVI_PARAM_GetMenuScreenDormantParam(&Value);
    CVI_TIMEDTASK_ATTR_S Attr = {.bEnable = (Value > 0 ? true : false), .u32Time_sec = Value, .periodic = false};
    stPowerCtrlCfg.astTaskCfg[CVI_PWRCTRL_TASK_SCREENDORMANT].stAttr = Attr;


    s32Ret = CVI_POWERCTRL_Init(&stPowerCtrlCfg);
    CVI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s_bPowerCtrlInited = true;
    return 0;
}

int32_t  UI_POWERCTRL_Deinit(void)
{
    int32_t  s32Ret = 0;

    s32Ret = CVI_POWERCTRL_DeInit();
    CVI_APPCOMM_CHECK_RETURN(s32Ret,s32Ret);

    s_bPowerCtrlInited = false;
    return 0;
}

int32_t  UI_POWERCTRL_PreProcessEvent(CVI_EVENT_S* pstEvent, bool* pbEventContinueHandle)
{
    int32_t  s32Ret = 0;
    uint32_t i = 0;
    CVI_PWRCTRL_EVENT_ATTR_S stEventAttr= {};
    static bool s_bLastModeIsCommon = true;

    /*process touch event in another file*/

    uint32_t as32EventMatrix[][5] =
    {
        /* Column :  0 Event,     1 Event Type,     2 Wake Type or Control Type,     3 Event Scope,     4 Reset Timer*/
        {CVI_EVENT_UI_TOUCH,   CVI_PWRCTRL_EVENT_TYPE_WAKEUP, CVI_PWRCTRL_WAKEUP_TACTICS_DISCARD, CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, true},

        {CVI_EVENT_KEYMNG_SHORT_CLICK,CVI_PWRCTRL_EVENT_TYPE_WAKEUP, CVI_PWRCTRL_WAKEUP_TACTICS_DISCARD, CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, true},
        {CVI_EVENT_KEYMNG_LONG_CLICK, CVI_PWRCTRL_EVENT_TYPE_WAKEUP, CVI_PWRCTRL_WAKEUP_TACTICS_DISCARD, CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, true},
        {CVI_EVENT_KEYMNG_HOLD_DOWN,  CVI_PWRCTRL_EVENT_TYPE_WAKEUP, CVI_PWRCTRL_WAKEUP_TACTICS_DISCARD, CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, true},
        {CVI_EVENT_KEYMNG_HOLD_UP,    CVI_PWRCTRL_EVENT_TYPE_WAKEUP, CVI_PWRCTRL_WAKEUP_TACTICS_DISCARD, CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, true},
        {CVI_EVENT_KEYMNG_GROUP,      CVI_PWRCTRL_EVENT_TYPE_WAKEUP, CVI_PWRCTRL_WAKEUP_TACTICS_DISCARD, CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, true},
        {CVI_EVENT_USB_INSERT, CVI_PWRCTRL_EVENT_TYPE_WAKEUP, CVI_PWRCTRL_WAKEUP_TACTICS_CONTINUE,  CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, true},
        {CVI_EVENT_USB_OUT,    CVI_PWRCTRL_EVENT_TYPE_WAKEUP, CVI_PWRCTRL_WAKEUP_TACTICS_CONTINUE, CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, true},
        /*for work mode : UVC,USB-Storage, HDMI,HDMI-Preview, Playback*/
        {CVI_EVENT_MODEMNG_MODEOPEN,CVI_PWRCTRL_EVENT_TYPE_CONTROL, CVI_PWRCTRL_EVENT_CONTROL_PAUSE, CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN, false},
        {CVI_EVENT_STORAGEMNG_DEV_CONNECTING,CVI_PWRCTRL_EVENT_TYPE_WAKEUP,CVI_PWRCTRL_WAKEUP_TACTICS_CONTINUE,CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN,true}
    };

    *pbEventContinueHandle = true;

    for (i=0; i<UI_ARRAY_SIZE(as32EventMatrix); i++) {
        if (pstEvent->topic != as32EventMatrix[i][0]) {
            continue;
        }
        stEventAttr.enType = (CVI_PWRCTRL_EVENT_TYPE_E)as32EventMatrix[i][1];

        if (CVI_PWRCTRL_EVENT_TYPE_WAKEUP == stEventAttr.enType) {
            stEventAttr.unCfg.stWakeupCfg.enType                  = (CVI_PWRCTRL_WAKEUP_TACTICS_E)as32EventMatrix[i][2];
            stEventAttr.unCfg.stWakeupCfg.stCommonCfg.enType      =   (CVI_PWRCTRL_EVENT_SCOPE_E)as32EventMatrix[i][3];
            stEventAttr.unCfg.stWakeupCfg.stCommonCfg.bResetTimer = (bool)as32EventMatrix[i][4];
        } else if(CVI_PWRCTRL_EVENT_TYPE_CONTROL == stEventAttr.enType) {
            stEventAttr.unCfg.stCtrlCfg.enType                   = (CVI_PWRCTRL_EVENT_CONTROL_E)as32EventMatrix[i][2];
            stEventAttr.unCfg.stCtrlCfg.stCommonCfg.enType       =  (CVI_PWRCTRL_EVENT_SCOPE_E)as32EventMatrix[i][3];
            stEventAttr.unCfg.stCtrlCfg.stCommonCfg.bResetTimer  = (bool)as32EventMatrix[i][4];
        } else if(CVI_PWRCTRL_EVENT_TYPE_COMMON == stEventAttr.enType) {
            stEventAttr.unCfg.stCommonCfg.enType       = (CVI_PWRCTRL_EVENT_SCOPE_E)as32EventMatrix[i][3];
            stEventAttr.unCfg.stCommonCfg.bResetTimer  = (bool)as32EventMatrix[i][4];
        }

        if (CVI_EVENT_MODEMNG_MODEOPEN == pstEvent->topic) {
            switch (pstEvent->arg1) {
                case CVI_WORK_MODE_PLAYBACK:
                case CVI_WORK_MODE_USBCAM:
                case CVI_WORK_MODE_USB:
                case CVI_WORK_MODE_UVC:
                case CVI_WORK_MODE_STORAGE:
                    if (!s_bLastModeIsCommon) {
                        return 0;
                    }
                    s_bLastModeIsCommon = false;
                    stEventAttr.unCfg.stCtrlCfg.enType                   = CVI_PWRCTRL_EVENT_CONTROL_PAUSE;
                    stEventAttr.unCfg.stCtrlCfg.stCommonCfg.enType       = CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN;
                    stEventAttr.unCfg.stCtrlCfg.stCommonCfg.bResetTimer  = false;
                    break;

                default:
                    if(s_bLastModeIsCommon) {
                        return 0;
                    }
                    s_bLastModeIsCommon = true;
                    stEventAttr.unCfg.stCtrlCfg.enType                   = CVI_PWRCTRL_EVENT_CONTROL_RESUME;
                    stEventAttr.unCfg.stCtrlCfg.stCommonCfg.enType       = CVI_PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN;
                    stEventAttr.unCfg.stCtrlCfg.stCommonCfg.bResetTimer  = true;
                    break;
            }
        }
        break;
    }

    if (i >= UI_ARRAY_SIZE(as32EventMatrix)) {
        CVI_LOGD("ignore event %x\n", pstEvent->topic);
        return 0;
    }

    s32Ret = CVI_POWERCTRL_EventPreProc(&stEventAttr, pbEventContinueHandle);
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret,s32Ret,"EventPreProc");

    return s32Ret;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif