#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cvi_modeinner.h"

static CVI_TOPIC_ID topic[] = {
    CVI_EVENT_STORAGEMNG_DEV_UNPLUGED, // 0
    CVI_EVENT_STORAGEMNG_DEV_CONNECTING,
    CVI_EVENT_STORAGEMNG_DEV_ERROR,
    CVI_EVENT_STORAGEMNG_FS_CHECKING,
    CVI_EVENT_STORAGEMNG_FS_CHECK_FAILED,
    CVI_EVENT_STORAGEMNG_FS_EXCEPTION,
    CVI_EVENT_STORAGEMNG_MOUNTED,
    CVI_EVENT_STORAGEMNG_MOUNT_FAILED,
    CVI_EVENT_STORAGEMNG_MOUNT_READ_ONLY,
    CVI_EVENT_RECMNG_STARTREC,
    CVI_EVENT_RECMNG_STOPREC,
    CVI_EVENT_RECMNG_SPLITREC,
    CVI_EVENT_RECMNG_SPLITSTART,
    CVI_EVENT_RECMNG_STARTEVENTREC,
    CVI_EVENT_RECMNG_EVENTREC_END,
    CVI_EVENT_RECMNG_STARTEMRREC,
    CVI_EVENT_RECMNG_EMRREC_END,
    CVI_EVENT_RECMNG_WRITE_ERROR, // 20
    CVI_EVENT_RECMNG_OPEN_FAILED,
    CVI_EVENT_RECMNG_PIV_END,
    CVI_EVENT_RECMNG_PIV_START,
#ifdef SERVICES_PHOTO_ON
    CVI_EVENT_PHOTOMNG_PIV_START,
    CVI_EVENT_PHOTOMNG_PIV_END,
#endif
    // CVI_EVENT_FILEMNG_SCAN_FAIL,
    // CVI_EVENT_FILEMNG_SCAN_COMPLETED, // 24
#ifdef SERVICES_PLAYER_ON
    CVI_EVENT_PLAYBACKMNG_PLAY,
    CVI_EVENT_PLAYBACKMNG_FINISHED,
    CVI_EVENT_PLAYBACKMNG_PROGRESS,
    CVI_EVENT_PLAYBACKMNG_PAUSE,
    CVI_EVENT_PLAYBACKMNG_RESUME,
    CVI_EVENT_PLAYBACKMNG_FILE_ABNORMAL,
#endif
    CVI_EVENT_SENSOR_PLUG_STATUS,
    CVI_EVENT_USB_UVC_READY,
    CVI_EVENT_USB_STORAGE_READY,
    CVI_EVENT_USB_HOSTUVC_READY,
#ifdef SERVICES_SPEECH_ON
    CVI_EVENT_SPEECHMNG_STARTREC,
    CVI_EVENT_SPEECHMNG_STOPREC,
    CVI_EVENT_SPEECHMNG_OPENFRONT,
    CVI_EVENT_SPEECHMNG_OPENREAR,
    CVI_EVENT_SPEECHMNG_CLOSESCREEN,
    CVI_EVENT_SPEECHMNG_OPENSCREEN,
    CVI_EVENT_SPEECHMNG_EMRREC,
    CVI_EVENT_SPEECHMNG_PIV,
    CVI_EVENT_SPEECHMNG_CLOSEWIFI,
    CVI_EVENT_SPEECHMNG_OPENWIFI,
#endif
};

int32_t CVI_MODEMNG_Eventcb(void *argv, CVI_EVENT_S *msg)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_S *pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    /** check init */
    MODEMNG_CHECK_CHECK_INIT(pstModeMngCtx->bInited, CVI_MODE_ENOTINIT, "ModeMng module has not been inited");

    /** check paramerter */
    MODEMNG_CHECK_POINTER(msg, CVI_MODE_ENULLPTR, "msg");
    MODEMNG_CHECK_POINTER(argv, CVI_MODE_ENULLPTR, "argv");

    /** check whether it has been intialized or not */
    CVI_MUTEX_LOCK(pstModeMngCtx->Mutex);

    /** push message to state machine queue */
    s32Ret = CVI_HFSM_SendAsyncMessage(pstModeMngCtx->pvHfsmHdl, (CVI_MESSAGE_S *)msg);
    MODEMNG_CHECK_CHECK_RET_WITH_UNLOCK(s32Ret, CVI_MODE_EINTER, "send message to HFSM(from eventhub)");

    CVI_MUTEX_UNLOCK(pstModeMngCtx->Mutex);
    return s32Ret;
}

static uint32_t CVI_MODEMNG_SubscribeEvents(void)
{
    int32_t ret = 0;
    uint32_t i = 0;
    CVI_EVENTHUB_SUBSCRIBER_S stSubscriber = {"modemng", NULL, CVI_MODEMNG_Eventcb, false};
    CVI_MW_PTR SubscriberHdl = NULL;

    ret = CVI_EVENTHUB_CreateSubscriber(&stSubscriber, &SubscriberHdl);
    MODEMNG_CHECK_RET(ret, CVI_MODE_EINTER, "CVI_EVENTHUB_CreateSubscriber failed!");

    uint32_t u32ArraySize = MODE_ARRAY_SIZE(topic);
    for (i = 0; i < u32ArraySize; i++) {
        ret = CVI_EVENTHUB_Subcribe(SubscriberHdl, topic[i]);
        if (ret) {
            CVI_LOGE("Subscribe topic(%#x) failed. %#x\n", topic[i], ret);
            continue;
        }
    }
    return ret;
}

static bool CVI_MODEMNG_CheckPulishEvent(uint32_t eventid)
{
    uint32_t u32ArraySize = MODE_ARRAY_SIZE(topic);
    uint32_t i = 0;
    for (i = 0; i < u32ArraySize; i++) {
        if (eventid == topic[i]) {
            return false;
        }
    }

    return true;
}

/** message process callback function for HFSM module */
static int32_t CVI_MODEMNG_HfsmEventProc(CVI_HFSM_HANDLE pvHfsmHdl,
                                         const CVI_HFSM_EVENT_INFO_S* pstEventInfo)
{
    CVI_MODEMNG_S *pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    MODEMNG_CHECK_POINTER(pstEventInfo, CVI_MODE_ENULLPTR, "parameter error");
    MODEMNG_CHECK_RET((pvHfsmHdl != pstModeMngCtx->pvHfsmHdl), CVI_MODE_EINVAL, "parameter error");

    /** check event info */
    if (CVI_HFSM_EVENT_UNHANDLE_MSG == pstEventInfo->enEventCode) {
        CVI_LOGD("CVI_HFSM_EVENT_UNHANDLE_MSG\n");
    } else if(CVI_HFSM_EVENT_TRANSTION_ERROR == pstEventInfo->enEventCode) {
        CVI_LOGD("CVI_HFSM_EVENT_TRANSTION_ERROR \n");
    } else if(CVI_HFSM_EVENT_HANDLE_MSG == pstEventInfo->enEventCode) {
        CVI_LOGD("CVI_HFSM_EVENT_HANDLE_MSG  %d \n", pstModeMngCtx->bInProgress);
    } else {
        CVI_LOGD("pstEventInfo enEventCode[%d] not support\n",pstEventInfo->enEventCode);
    }
    CVI_MUTEX_LOCK(pstModeMngCtx->Mutex);

    CVI_LOGD("pstEventInfo->pstunHandlerMsg->topic: %x\n", pstEventInfo->pstunHandlerMsg->topic);
    if (true == pstModeMngCtx->bInProgress) {
        pstModeMngCtx->bInProgress = false;
    }
    if (CVI_MODEMNG_CheckPulishEvent(pstEventInfo->pstunHandlerMsg->topic)){
        CVI_MODEMNG_PublishEvent(pstEventInfo->pstunHandlerMsg, 0);
    }
    //printf("############ HfsmEventProc done, bInProgress: %d ############\n", pstModeMngCtx->bInProgress);
    CVI_MUTEX_UNLOCK(pstModeMngCtx->Mutex);

    return 0;
}

/** create HFSM instance */
static int32_t CVI_MODEMNG_CreateHFSMInstance(void)
{
    int32_t s32Ret = 0;
    CVI_HFSM_ATTR_S stHfsmAttr;
    memset(&stHfsmAttr, 0, sizeof(CVI_HFSM_ATTR_S));
    stHfsmAttr.fnHfsmEventCallback = CVI_MODEMNG_HfsmEventProc;
    stHfsmAttr.u32StateMaxAmount = CVI_WORK_MODE_BUTT+1;/**< include base */
    stHfsmAttr.u32MessageQueueSize = 32;
    s32Ret = CVI_HFSM_Create(&stHfsmAttr, &(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl));
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "HFSM instance create");

    return s32Ret;
}

/** destory HFSM instance */
static int32_t CVI_MODEMNG_DestoryHFSMInstance(void)
{
    int32_t s32Ret = 0;

    s32Ret = CVI_HFSM_Destroy(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl);
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "HFSM destory");

    return s32Ret;
}

/** init and add all states */
static int32_t CVI_MODEMNG_InitStates(void)
{
    int32_t s32Ret = 0;
    // isMovieModeOpen = false;

    CVI_MODEMNG_S *pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    s32Ret = CVI_MODEMNG_BaseStateInit();/** base state must be init at first*/
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Base mode init");

    s32Ret = CVI_MODEMNG_MovieStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Movie mode init");

    s32Ret = CVI_MODEMNG_PlaybackStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Playback mode init");

    s32Ret = CVI_MODEMNG_UpDateStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Update mode init");

    s32Ret = CVI_MODEMNG_UvcStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "UVC mode init");

#ifdef SERVICES_PHOTO_ON
    s32Ret = CVI_MODEMNG_PhotoStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Photo mode init");
#endif

    s32Ret = CVI_MODEMNG_StorageStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Storage state init");

    s32Ret = CVI_MODEMNG_BootFirstStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "BootFirst mode init");

	s32Ret = CVI_MODEMNG_UsbMenuStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "usbmenu mode init");

#ifndef ENABLE_ISP_PQ_TOOL
    CVI_PARAM_CFG_S param;
    CVI_PARAM_GetParam(&param);
    if (param.Menu.UserData.bBootFirst) {
        pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_BOOT;
    } else
#endif
    {
        pstModeMngCtx->CurWorkMode = CVI_WORK_MODE_MOVIE;
    }

    return s32Ret;
}

static int32_t CVI_MODEMNG_DeinitStates(void)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_S *pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    switch (pstModeMngCtx->CurWorkMode) {
        case CVI_WORK_MODE_MOVIE:
            s32Ret = CVI_MODEMNG_MovieStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Movie mode deinit");
            break;
        case CVI_WORK_MODE_PLAYBACK:
            s32Ret = CVI_MODEMNG_PlaybackStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Playback mode deinit");
        case CVI_WORK_MODE_UPDATE:
            s32Ret = CVI_MODEMNG_UpDateStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Update mode deinit");
            break;
        case CVI_WORK_MODE_PHOTO:
#ifdef SERVICES_PHOTO_ON
            s32Ret = CVI_MODEMNG_PhotoStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Photo mode deinit");
#endif
            break;
        case CVI_WORK_MODE_UVC:
            s32Ret = CVI_MODEMNG_UvcStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "UVC mode deinit");
            break;
        case CVI_WORK_MODE_STORAGE:
            s32Ret = CVI_MODEMNG_StorageStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Storage mode deinit");
            break;
        case CVI_WORK_MODE_BOOT:
            s32Ret = CVI_MODEMNG_BootFirstStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "BOOT mode deinit");
            break;
        case CVI_WORK_MODE_USBMENU:
            s32Ret = CVI_MODEMNG_UsbMenuStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Usbmenu mode deinit");
            break;
        default:
            CVI_LOGI("CVI_MODEMNG_DeinitStates unnormal state\n");
    }

    s32Ret = CVI_MODEMNG_BaseStatesDeinit();
    MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "Base mode deinit");

    return s32Ret;
}

/** activate HFSM */
static int32_t CVI_MODEMNG_ActivateHFSM(CVI_WORK_MODE_E enWorkmode)
{
    int32_t s32Ret = 0;

    s32Ret = CVI_HFSM_SetInitialState(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl, enWorkmode);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"HFSM set initial state");

    /** active HFSM */
    s32Ret = CVI_HFSM_Start(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"HFSM start");

    return s32Ret;
}

/** deactive HFSM */
static int32_t CVI_MODEMNG_DeactiveHFSM(void)
{
    int32_t s32Ret = 0;

    /** stop HFSM instance */
    s32Ret = CVI_HFSM_Stop(CVI_MODEMNG_GetModeCtx()->pvHfsmHdl);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINVAL,"HFSM stop");

    return s32Ret;
}

/** Modemng initialization */
int32_t CVI_MODEMNG_Init(CVI_MODEMNG_CONFIG_S *stModemngCfg)
{
    int32_t s32Ret = 0;
    /** check the statemode_cfg*/
    if (NULL == stModemngCfg) {
        CVI_LOGE("stModemngCfg is null point\n");
        return CVI_MODE_EINTER;
    }

    /** check whether it has been intialized or not */
    if(true == CVI_MODEMNG_GetModeCtx()->bInited)
    {
        CVI_LOGE("StateMng module has already been inited\n\n");
        return CVI_MODE_EINITIALIZED;
    }

    s32Ret = CVI_MODEMNG_ContextInit(stModemngCfg);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"CVI_MODEMNG_ContextInit fail");

    /** create HFSM instance */
    s32Ret = CVI_MODEMNG_CreateHFSMInstance();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"HFSM instance create");

    /** init all States */
    s32Ret = CVI_MODEMNG_InitStates();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"add all states");

    /** subscribe event from EventHub module */
    s32Ret = CVI_MODEMNG_SubscribeEvents();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"subscribe events");
    CVI_MODEMNG_GetModeCtx()->bInited = true;

    /** Storage service init here */
    s32Ret = CVI_MODEMNG_InitStorage();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"Storage service init");

    /** activate HFSM */
    s32Ret = CVI_MODEMNG_ActivateHFSM(CVI_MODEMNG_GetModeCtx()->CurWorkMode);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"HFSM set initial state");



    return s32Ret;
}

/** Modemng deinitialization */
int32_t CVI_MODEMNG_Deinit()
{
    int32_t s32Ret = 0;

    /** check whether it has been deintialized or not */
    MODEMNG_CHECK_CHECK_INIT(CVI_MODEMNG_GetModeCtx()->bInited, 0, "StateMng module has already been deinited");

    /** deinit all states */
    s32Ret = CVI_MODEMNG_DeinitStates();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"deinit all states");

    /** deactive HFSM */
    s32Ret = CVI_MODEMNG_DeactiveHFSM();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"deactive HFSM");

    /** destory HFSM */
    s32Ret = CVI_MODEMNG_DestoryHFSMInstance();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"destory HFSM");

    /** destory Storage */
    s32Ret = CVI_MODEMNG_DeintStorage();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"destory Storage");

    CVI_MODEMNG_GetModeCtx()->bInited = false;

    return s32Ret;
}

/** send message to modemng with parameter, synchronize UI */
int32_t CVI_MODEMNG_SendMessage(const CVI_MESSAGE_S *pstMsg)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_S *pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    /** check paramerter */
    MODEMNG_CHECK_POINTER(pstMsg,CVI_MODE_ENULLPTR, "pstMsg");

    /** check whether it has been intialized or not */
    MODEMNG_CHECK_CHECK_INIT(pstModeMngCtx->bInited, CVI_MODE_ENOTINIT, "ModeMng module has not been inited");

    CVI_MUTEX_LOCK(pstModeMngCtx->Mutex);
    /** check whether it was in progress or not */
    if(true == pstModeMngCtx->bInProgress)
    {
        CVI_LOGE("StateMng module is InProgress\n\n");
        // CVI_MUTEX_UNLOCK(pstModeMngCtx->Mutex);
        // return CVI_MODE_EINPROGRESS;
    }

    s32Ret = CVI_HFSM_SendAsyncMessage(pstModeMngCtx->pvHfsmHdl, (CVI_MESSAGE_S *)pstMsg);
    MODEMNG_CHECK_CHECK_RET_WITH_UNLOCK(s32Ret,CVI_MODE_EINTER,"send message to HFSM(from Terminal)");

    pstModeMngCtx->bInProgress = true;
    CVI_MUTEX_UNLOCK(pstModeMngCtx->Mutex);
    //printf("############ send message to HFSM done, bInProgress: %d ############\n", pstModeMngCtx->bInProgress);

    return s32Ret;
}

int32_t CVI_MODEMNG_GetCurMode(uint32_t *ModeId)
{
    int32_t s32Ret = 0;
    CVI_MODEMNG_S *pstModeMngCtx = CVI_MODEMNG_GetModeCtx();
    CVI_STATE_S state = {0};
    s32Ret = CVI_HFSM_GetCurrentState(pstModeMngCtx->pvHfsmHdl, &state);

    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"CVI_HFSM_GetCurrentState");
    *ModeId = state.stateID;

    return s32Ret;
}

int32_t CVI_MODEMNG_SetCurModeMedia(CVI_WORK_MODE_E CurMode)
{
    int32_t s32Ret = 0;
    CVI_PARAM_CFG_S param;
    s32Ret = CVI_PARAM_GetParam(&param);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"CVI_HFSM_GetCurrentState");
    switch (CurMode) {
        case CVI_WORK_MODE_MOVIE:
            for (uint32_t i = 0; i < param.WorkModeCfg.RecordMode.CamNum; i++) {
                param.CamCfg[i].CamMediaInfo = param.WorkModeCfg.RecordMode.CamMediaInfo[i];
            }
            param.MediaComm.Vpss = param.WorkModeCfg.RecordMode.Vpss;
            break;
        case CVI_WORK_MODE_PHOTO:
            for (uint32_t i = 0; i < param.WorkModeCfg.PhotoMode.CamNum; i++) {
                 param.CamCfg[i].CamMediaInfo = param.WorkModeCfg.PhotoMode.CamMediaInfo[i];
            }
            param.MediaComm.Vpss = param.WorkModeCfg.PhotoMode.Vpss;
            break;
        case CVI_WORK_MODE_PLAYBACK:
            break;
        case CVI_WORK_MODE_USB:
            break;
        default:
            CVI_LOGE("Not suporrt mode: %d\n", CurMode);
            break;
    }
    CVI_PARAM_SetParam(&param);

    return s32Ret;
}
