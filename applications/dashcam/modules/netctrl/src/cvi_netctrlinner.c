#include <stdio.h>
#include <sys/statfs.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <errno.h>
#include "cvi_netctrlinner.h"

#define NETCTRL_ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

static PDT_NETCTRL_MESSAGE_CONTEXT s_stNETCTRLMessageCtx = {.bMsgProcessed = true, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
static sem_t s_NETCTRLSem;
static int32_t flag_app_connect = WIFI_APP_DISCONNECT;
static int32_t s_counttime = 0;
static int32_t sd_status = 0;
static uint32_t s_TimedTaskHdl;

void CVI_NETCTRLINNER_ScanFile()
{
    uint32_t pu32FileObjCnt = 0;
    uint32_t u32DirCount = 0;
    uint32_t u32Index = 0;
    CVI_PARAM_FILEMNG_S stCfg = {0};
    CVI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT];
    CVI_PARAM_GetFileMngParam(&stCfg);
    for(u32Index = 0; u32Index < DTCF_DIR_BUTT; u32Index++) {
        if ( 0 < strnlen(stCfg.FileMngDtcf.aszDirNames[u32Index], CVI_DIR_LEN_MAX)) {
            aenDirs[u32DirCount++] = u32Index;
        }
    }
    CVI_FILEMNG_SetSearchScope(aenDirs, u32DirCount, &pu32FileObjCnt);
}

int32_t CVI_NETCTRLINNER_InitTimer()
{
    int32_t result = 0;;
    CVI_TIMEDTASK_CFG_S stTimedTaskCfg;
    stTimedTaskCfg.timerProc = CVI_NETCTRLINNER_Enablecheckconnect;
    stTimedTaskCfg.pvPrivData = NULL;
    stTimedTaskCfg.stAttr.bEnable = true;
    stTimedTaskCfg.stAttr.u32Time_sec = 1;
    stTimedTaskCfg.stAttr.periodic = true;
    result = CVI_TIMEDTASK_Create(&stTimedTaskCfg, &s_TimedTaskHdl);
    return result;
}

int32_t CVI_NETCTRLINNER_DeInitTimer()
{
    int32_t result = 0;;
    result = CVI_TIMEDTASK_Destroy(s_TimedTaskHdl);
    return result;
}

int32_t CVI_NETCTRLINNER_StopTimer()
{
    int32_t result = 0;
    CVI_TIMEDTASK_ATTR_S TimedTaskAttr;
    CVI_TIMEDTASK_GetAttr(s_TimedTaskHdl, &TimedTaskAttr);
    if (TimedTaskAttr.bEnable == true) {
        TimedTaskAttr.bEnable = false;
        result = CVI_TIMEDTASK_SetAttr(s_TimedTaskHdl, &TimedTaskAttr);
    }
    return result;
}

int32_t CVI_NETCTRLINNER_StartTimer()
{
    int32_t result = 0;
    CVI_TIMEDTASK_ATTR_S TimedTaskAttr;
    CVI_TIMEDTASK_GetAttr(s_TimedTaskHdl, &TimedTaskAttr);
    if (TimedTaskAttr.bEnable == false) {
        TimedTaskAttr.bEnable = true;
        result = CVI_TIMEDTASK_SetAttr(s_TimedTaskHdl, &TimedTaskAttr);
    }
    return result;
}

void CVI_NETCTRLINNER_UiUpdate()
{
    CVI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(CVI_EVENT_S));
    stEvent.topic = CVI_EVENT_NETCTRL_UIUPDATE;
    CVI_EVENTHUB_Publish(&stEvent);
}

static int CVI_NETCTRLINNER_MessageResult(CVI_EVENT_S *pstEvent)
{
    int s32Ret = 0;
    CVI_MUTEX_LOCK(s_stNETCTRLMessageCtx.MsgMutex);

    if (!s_stNETCTRLMessageCtx.bMsgProcessed) {
        CVI_LOGD("event(%x)\n\n", pstEvent->topic);
        if ((s_stNETCTRLMessageCtx.stMsg.topic == pstEvent->topic)
            && (s_stNETCTRLMessageCtx.stMsg.arg1 == pstEvent->arg1)
            && (s_stNETCTRLMessageCtx.stMsg.arg2 == pstEvent->arg2)) {
                sem_post(&s_NETCTRLSem);
                s_stNETCTRLMessageCtx.bMsgProcessed = true;
        }
    }

    CVI_MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);

    return s32Ret;
}

int32_t CVI_NETCTRLINNER_SendSyncMsg(CVI_MESSAGE_S* pstMsg, int* ps32Result)
{
    int s32Ret = 0;
    CVI_MUTEX_LOCK(s_stNETCTRLMessageCtx.MsgMutex);

    if (!s_stNETCTRLMessageCtx.bMsgProcessed) {
        CVI_LOGE("Current Msg not finished\n");
        CVI_MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);
        return -1;
    }

    s_stNETCTRLMessageCtx.bMsgProcessed = false;
    s_stNETCTRLMessageCtx.stMsg.topic = pstMsg->topic;
    s_stNETCTRLMessageCtx.stMsg.arg1 = pstMsg->arg1;
    s_stNETCTRLMessageCtx.stMsg.arg2 = pstMsg->arg2;
    s_stNETCTRLMessageCtx.stMsg.s32Result = -1;
    memcpy(s_stNETCTRLMessageCtx.stMsg.aszPayload, pstMsg->aszPayload, sizeof(s_stNETCTRLMessageCtx.stMsg.aszPayload));

    s32Ret = CVI_MODEMNG_SendMessage(pstMsg);

    if (0 != s32Ret)
    {
        CVI_LOGE("Send Message Error:%#x\n", s32Ret);
        s_stNETCTRLMessageCtx.bMsgProcessed = true;
        *ps32Result = 1;
        CVI_MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);
        return -1;
    }
    CVI_MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);

    while ((0 != sem_wait(&s_NETCTRLSem)) && (errno == EINTR));

    *ps32Result = 0;

    s_stNETCTRLMessageCtx.stMsg.topic = 0;
    s_stNETCTRLMessageCtx.stMsg.arg1 = 0;
    s_stNETCTRLMessageCtx.stMsg.arg2 = 0;
    s_stNETCTRLMessageCtx.bMsgProcessed = true;
    return 0;
}

static int CVI_NETCTRLINNER_Eventcb(void *argv, CVI_EVENT_S *msg)
{
    int s32Ret = 0;
    if (msg->topic == CVI_EVENT_NETCTRL_CONNECT) {
        CVI_NETCTRLINNER_TimeoutRest();
        return 0;
    }
    /*receive message result*/
    s32Ret = CVI_NETCTRLINNER_MessageResult(msg);
    switch(msg->topic){
        case CVI_EVENT_MODEMNG_SETTING:
            if(msg->arg1 == CVI_PARAM_MENU_DEFAULT){
                CVI_LOGD("CVI_PARAM_MENU_DEFAULT\n" );
                sleep(1);
                reboot(RB_AUTOBOOT);
            }
            break;

        case CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED:
            sd_status = WIFI_SD_FORMAT_SUCCESS;
            break;

        case CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED:
            sd_status = WIFI_SD_FORMAT_FAIL;
            break;

        default:
            break;
    }
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "MessageResult");
    return 0;
}

int32_t CVI_NETCTRLINNER_SubscribeEvents(void)
{
    int32_t ret = 0;
    uint32_t i = 0;
    CVI_EVENTHUB_SUBSCRIBER_S stnetSubscriber = {"net", NULL, CVI_NETCTRLINNER_Eventcb, false};
    CVI_MW_PTR netSubscriberHdl = NULL;
    CVI_TOPIC_ID topic[] = {
        CVI_EVENT_MODEMNG_SETTING,
        CVI_EVENT_MODEMNG_STOP_REC,
        CVI_EVENT_MODEMNG_START_REC,
        CVI_EVENT_MODEMNG_LIVEVIEW_UPORDOWN,
        CVI_EVENT_MODEMNG_START_PIV,
        CVI_EVENT_MODEMNG_MODESWITCH,
        CVI_EVENT_MODEMNG_CARD_FORMAT,
        CVI_EVENT_MODEMNG_START_UPFILE,
        CVI_EVENT_MODEMNG_SWITCH_LIVEVIEW,
        CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED,
        CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED,
        CVI_EVENT_MODEMNG_RTSP_INIT,
        CVI_EVENT_MODEMNG_RTSP_SWITCH,
        CVI_EVENT_MODEMNG_RTSP_DEINIT,
        CVI_EVENT_NETCTRL_CONNECT,
    };

    sem_init(&s_NETCTRLSem, 0, 0);

    ret = CVI_EVENTHUB_CreateSubscriber(&stnetSubscriber, &netSubscriberHdl);
    if (ret != 0) {
        CVI_LOGE("CVI_EVENTHUB_CreateSubscriber failed! \n");
    }

    uint32_t u32ArraySize = NETCTRL_ARRAY_SIZE(topic);

    for (i = 0; i < u32ArraySize; i++) {
        ret = CVI_EVENTHUB_Subcribe(netSubscriberHdl, topic[i]);
        if (ret) {
            CVI_LOGE("Subscribe topic(%#x) failed. %#x\n", topic[i], ret);
            continue;
        }
    }

    return ret;
}

void CVI_NETCTRLINNER_Enablecheckconnect(void *privData, struct timespec *now)
{
    CVI_EVENT_S stEvent;
    if (s_counttime > 0) {
        s_counttime--;
        if (flag_app_connect == WIFI_APP_DISCONNECT) {
            flag_app_connect = WIFI_APP_CONNECTTED;
            stEvent.topic = CVI_EVENT_NETCTRL_APPCONNECT_SUCCESS;
            CVI_EVENTHUB_Publish(&stEvent);
            CVI_NETCTRLINNER_InitCMDSocket();
        }
    } else {
        // If there is no message in 10 second timing, it will be disconnected by default
        if (0 == s_counttime) {
            s_counttime = -1;
            if (flag_app_connect == WIFI_APP_CONNECTTED) {
                flag_app_connect = WIFI_APP_DISCONNECT;
                stEvent.topic = CVI_EVENT_NETCTRL_APPDISCONNECT;
                stEvent.arg1 = CVI_NETCTRLINNER_GetFlagFile();
                CVI_EVENTHUB_Publish(&stEvent);
                CVI_NETCTRLINNER_DeInitCMDSocket();
            }
        }
    }
}

int32_t CVI_NETCTRLINNER_TimeoutRest(void)
{
    s_counttime = TIMECOUNTMAX;
    return 0;
}

int32_t CVI_NETCTRLINNER_APPConnetState(void)
{
    return flag_app_connect;
}

int32_t CVI_NETCTRLINNER_GetSdState(void)
{
    return sd_status;
}

void CVI_NETCTRLINNER_SetSdState(int32_t value)
{
    sd_status = value;
}