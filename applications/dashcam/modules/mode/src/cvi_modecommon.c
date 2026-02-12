#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "mmc.h"

#include "cvi_modeinner.h"

static CVI_MODEMNG_S s_stModeMng = {.Mutex = PTHREAD_MUTEX_INITIALIZER,
                            .bInited = false,
                            .bSysPowerOff = false,
                            .CurWorkMode = CVI_WORK_MODE_BUTT,};

CVI_MODEMNG_S* CVI_MODEMNG_GetModeCtx(void)
{
    return &s_stModeMng;
}

int32_t CVI_MODEMNG_GetCurWorkMode(void)
{
    return s_stModeMng.CurWorkMode;
}

uint32_t CVI_MODEMNG_GetCardState(void)
{
    return s_stModeMng.u32CardState;
}

bool CVI_MODEMNG_SetCardState(uint32_t u32CardState)
{
    bool ret = true;

    CVI_LOGD("SetCardState as 0x%x\n", u32CardState);

    switch(u32CardState) {
        case CVI_CARD_STATE_REMOVE:
        case CVI_CARD_STATE_UNAVAILABLE:
        case CVI_CARD_STATE_ERROR:
        case CVI_CARD_STATE_FSERROR:
        case CVI_CARD_STATE_CHECKING:
        case CVI_CARD_STATE_SLOW:
        case CVI_CARD_STATE_READ_ONLY:
        case CVI_CARD_STATE_MOUNT_FAILED:
            s_stModeMng.u32CardState = u32CardState;
            break;

        case CVI_CARD_STATE_AVAILABLE:
        case CVI_CARD_STATE_FORMATED:
            if (CVI_CARD_STATE_REMOVE == CVI_MODEMNG_GetCardState()) {
                CVI_LOGE("Card already be removed\n");
                ret = false;
                break;
            }
            s_stModeMng.u32CardState = u32CardState;
            break;
        default:
            CVI_LOGE("Not allowed CardState: 0x%x\n", u32CardState);
            ret = false;
            break;
    }

    return ret;
}

int32_t CVI_MODEMNG_GetModeState(uint32_t *state)
{
    *state = s_stModeMng.u32ModeState;

    return 0;
}

int32_t CVI_MODEMNG_SetModeState(uint32_t state)
{
    s_stModeMng.u32ModeState = state;

    return 0;
}

int32_t CVI_MODEMNG_SetParkingRec(bool en)
{
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    pstModeMngCtx->bInParkingRec = en;

    return 0;
}

int32_t CVI_MODEMNG_SetEmrState(bool en)
{
    CVI_MODEMNG_S* pstModeMngCtx = CVI_MODEMNG_GetModeCtx();

    pstModeMngCtx->bInEmrRec = en;

    return 0;
}

bool CVI_MODEMNG_GetEmrState(void)
{
    return s_stModeMng.bInEmrRec;
}

bool CVI_MODEMNG_GetInProgress(void)
{
    return s_stModeMng.bInProgress;
}

int32_t CVI_MODEMNG_MonitorStatusNotify(CVI_EVENT_S* pstMsg)
{
    /* Publish Event */
    CVI_EVENT_S stEvent = {0};
    memcpy(&stEvent, pstMsg, sizeof(CVI_EVENT_S));
    switch (stEvent.topic) {
        case CVI_EVENT_STORAGEMNG_DEV_UNPLUGED:
            stEvent.topic = CVI_EVENT_MODEMNG_CARD_REMOVE;
            break;
        case CVI_EVENT_STORAGEMNG_DEV_ERROR:
            stEvent.topic = CVI_EVENT_MODEMNG_CARD_ERROR;
            break;
        case CVI_EVENT_STORAGEMNG_MOUNT_FAILED:
            stEvent.topic = CVI_EVENT_MODEMNG_CARD_MOUNT_FAILED;
            break;
        case CVI_EVENT_STORAGEMNG_MOUNT_READ_ONLY:
            stEvent.topic = CVI_EVENT_MODEMNG_CARD_READ_ONLY;
            break;
        case CVI_EVENT_STORAGEMNG_FS_EXCEPTION:
        case CVI_EVENT_STORAGEMNG_FS_CHECK_FAILED:
            stEvent.topic = CVI_EVENT_MODEMNG_CARD_FSERROR;
            break;
        case CVI_EVENT_STORAGEMNG_FS_CHECKING:
        case CVI_EVENT_STORAGEMNG_DEV_CONNECTING:
            stEvent.topic = CVI_EVENT_MODEMNG_CARD_CHECKING;
            break;
        case CVI_EVENT_SENSOR_PLUG_STATUS:
            stEvent.topic = CVI_EVENT_MODEMNG_RESET;
            break;
        case CVI_EVENT_USB_UVC_READY:
            stEvent.topic = CVI_EVENT_MODEMNG_MODESWITCH;
            break;

        case CVI_EVENT_RECMNG_STARTREC:
            stEvent.topic = CVI_EVENT_MODEMNG_RECODER_STARTSTATU;
            break;
        case CVI_EVENT_RECMNG_STOPREC:
            stEvent.topic = CVI_EVENT_MODEMNG_RECODER_STOPSTATU;
            break;
        case CVI_EVENT_RECMNG_STARTEMRREC:
            stEvent.topic = CVI_EVENT_MODEMNG_RECODER_STARTEMRSTAUE;
            break;
        case CVI_EVENT_RECMNG_EMRREC_END:
            stEvent.topic = CVI_EVENT_MODEMNG_RECODER_STOPEMRSTAUE;
            break;
        case CVI_EVENT_RECMNG_STARTEVENTREC:
            stEvent.topic = CVI_EVENT_MODEMNG_RECODER_STARTEVENTSTAUE;
            break;
        case CVI_EVENT_RECMNG_EVENTREC_END:
            stEvent.topic = CVI_EVENT_MODEMNG_RECODER_STOPEVENTSTAUE;
            break;
        case CVI_EVENT_RECMNG_SPLITREC:
            stEvent.topic = CVI_EVENT_MODEMNG_RECODER_SPLITREC;
            break;
        case CVI_EVENT_RECMNG_PIV_END:
            stEvent.topic = CVI_EVENT_MODEMNG_RECODER_STOPPIVSTAUE;
            break;
        case CVI_EVENT_RECMNG_PIV_START:
            stEvent.topic = CVI_EVENT_MODEMNG_RECODER_STARTPIVSTAUE;
            break;
#ifdef SERVICES_PLAYER_ON
        case CVI_EVENT_PLAYBACKMNG_PLAY:
            stEvent.topic = CVI_EVENT_MODEMNG_PLAYBACK_PLAY;
            break;
        case CVI_EVENT_PLAYBACKMNG_FINISHED:
            stEvent.topic = CVI_EVENT_MODEMNG_PLAYBACK_FINISHED;
            break;
        case CVI_EVENT_PLAYBACKMNG_PROGRESS:
            stEvent.topic = CVI_EVENT_MODEMNG_PLAYBACK_PROGRESS;
            break;
        case CVI_EVENT_PLAYBACKMNG_PAUSE:
            stEvent.topic = CVI_EVENT_MODEMNG_PLAYBACK_PAUSE;
            break;
        case CVI_EVENT_PLAYBACKMNG_RESUME:
            stEvent.topic = CVI_EVENT_MODEMNG_PLAYBACK_RESUME;
            break;
        case CVI_EVENT_PLAYBACKMNG_FILE_ABNORMAL:
            stEvent.topic = CVI_EVENT_MODEMNG_PLAYBACK_ABNORMAL;
            break;
#endif
#ifdef SERVICES_SPEECH_ON
        case CVI_EVENT_SPEECHMNG_STARTREC:
            stEvent.topic = CVI_EVENT_MODEMNG_SPEECHMNG_STARTREC;
            break;
        case CVI_EVENT_SPEECHMNG_STOPREC:
            stEvent.topic = CVI_EVENT_MODEMNG_SPEECHMNG_STOPREC;
            break;
        case CVI_EVENT_SPEECHMNG_OPENFRONT:
            stEvent.topic = CVI_EVENT_MODEMNG_SPEECHMNG_OPENFRONT;
            break;
        case CVI_EVENT_SPEECHMNG_OPENREAR:
            stEvent.topic = CVI_EVENT_MODEMNG_SPEECHMNG_OPENREAR;
            break;
        case CVI_EVENT_SPEECHMNG_CLOSESCREEN:
            stEvent.topic = CVI_EVENT_MODEMNG_SPEECHMNG_CLOSESCREEN;
            break;
        case CVI_EVENT_SPEECHMNG_OPENSCREEN:
            stEvent.topic = CVI_EVENT_MODEMNG_SPEECHMNG_OPENSCREEN;
            break;
        case CVI_EVENT_SPEECHMNG_EMRREC:
            stEvent.topic = CVI_EVENT_MODEMNG_SPEECHMNG_EMRREC;
            break;
        case CVI_EVENT_SPEECHMNG_PIV:
            stEvent.topic = CVI_EVENT_MODEMNG_SPEECHMNG_PIV;
            break;
        case CVI_EVENT_SPEECHMNG_CLOSEWIFI:
            stEvent.topic = CVI_EVENT_MODEMNG_SPEECHMNG_CLOSEWIFI;
            break;
        case CVI_EVENT_SPEECHMNG_OPENWIFI:
            stEvent.topic = CVI_EVENT_MODEMNG_SPEECHMNG_OPENWIFI;
            break;
#endif
        default:
            CVI_LOGW("Invalid event[%d]\n", stEvent.topic);
            return -1;
    }
    CVI_MODEMNG_UPDATESTATUS(&stEvent, true, false);

    return 0;
}

int32_t CVI_MODEMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_REMOVE);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_AVAILABLE);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_ERROR);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_FSERROR);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_SLOW);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_CHECKING);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_FORMAT);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_FORMATING);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_FORMAT_SUCCESSED);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_FORMAT_FAILED);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_CARD_READ_ONLY);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_RESET);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_MODESWITCH);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_MODEOPEN);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_MODECLOSE);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_SETTING);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_START_PIV);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_PLAYBACK_PLAY);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_PLAYBACK_FINISHED);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_PLAYBACK_PROGRESS);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_PLAYBACK_PAUSE);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_PLAYBACK_RESUME);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_MODEMNG_PLAYBACK_ABNORMAL);
#ifdef SERVICES_SPEECH_ON
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_STARTREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_STOPREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_OPENFRONT);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_OPENREAR);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_CLOSESCREEN);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_OPENSCREEN);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_EMRREC);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_PIV);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_CLOSEWIFI);
    s32Ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_SPEECHMNG_OPENWIFI);
#endif
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"CVI_MODEMNG_RegisterEvent error");

    return 0;
}

int32_t CVI_MODEMNG_LiveViewSwitch(uint32_t viewwin)
{
#ifdef SERVICES_LIVEVIEW_ON
    CVI_PARAM_WND_ATTR_S WndParam;
    CVI_PARAM_GetWndParam(&WndParam);

    uint32_t enWinds = ((viewwin >> 16) & 0xFFFF);
    uint32_t enSns = viewwin & 0xFFFF;

    int32_t enwndnum = 0;
    for(uint32_t i = 0; i < WndParam.WndCnt && i < MAX_CAMERA_INSTANCES; i++) {
        if(((enWinds >> i) & 0x1) == 1){
            WndParam.Wnds[i].WndEnable = true;
            enwndnum++;
        }else{
            WndParam.Wnds[i].WndEnable = false;
        }
    }

    uint32_t val = 0;

    /* show ALL */
    if(enWinds == enSns){
        for(uint32_t i = 0; i < WndParam.WndCnt && i < MAX_CAMERA_INSTANCES; i++){
            if(((enWinds >> i) & 0x1) == 1){
                WndParam.Wnds[i].SmallWndEnable = true;
                /*         win en   |   small en   */
                val |= ((0x1 << (i * 2 + 1)) | (0x1 << (i * 2)));
                if(enwndnum == 1){
                    WndParam.Wnds[i].SmallWndEnable = false;
                    val &= (~(0x1 << (i * 2)));
                }
            }
        }
    }else{/*show one*/
        for(uint32_t i = 0; i < WndParam.WndCnt && i < MAX_CAMERA_INSTANCES; i++){
            WndParam.Wnds[i].SmallWndEnable = true;
            if(((enWinds >> i) & 0x1) == 1){
                val |= (0x1 << (i * 2 + 1));
                WndParam.Wnds[i].SmallWndEnable = false;
            }
        }
    }
    CVI_LIVEVIEWMNG_Switch(val);
    CVI_PARAM_SetWndParam(&WndParam);
    CVI_PARAM_SetMenuParam(0,  CVI_PARAM_MENU_VIEW_WIN_STATUS, viewwin);
#endif
    return 0;
}

void CVI_MODEMNG_InitFilemng(void)
{
    int32_t s32Ret = 0;
    /** init filemng */
    CVI_STG_FS_INFO_S stFSInfo = {0};

    s32Ret = CVI_STORAGEMNG_GetFSInfo(&stFSInfo);
    if (0 != s32Ret) {
        CVI_LOGE("get fs info failed\n");
        return;
    }

    STG_DEVINFO_S SDParam = {0};
    CVI_PARAM_GetStgInfoParam(&SDParam);
    CVI_FILEMNG_SetSdConfigPath(SDParam.aszMntPath);
    int32_t recLoop = 0;
    CVI_PARAM_GetRecLoop(&recLoop);
    CVI_PARAM_FILEMNG_S FileMng;
    CVI_PARAM_GetFileMngParam(&FileMng);
    FileMng.FileMngDtcf.u32RemoveLoopEn = recLoop;
    FileMng.FileMngDtcf.u32WarningStage = stFSInfo.u64TotalSize *
        FileMng.FileMngDtcf.u32WarningStage/100 >> 20;
    FileMng.FileMngDtcf.u32GuaranteedStage = stFSInfo.u64TotalSize *
        FileMng.FileMngDtcf.u32GuaranteedStage/100 >> 20;
    CVI_LOGD("FileMng.FileMngDtcf.u32WarningStage = %d,FileMng.FileMngDtcf.u32GuaranteedStage = %d\n",
        FileMng.FileMngDtcf.u32WarningStage, FileMng.FileMngDtcf.u32GuaranteedStage);
    s32Ret = CVI_FILEMNG_Init(&FileMng.FileMngComm, &FileMng.FileMngDtcf);
    if (CVI_FILEMNG_EINITIALIZED == s32Ret) {
        CVI_LOGE("FileMng already inited\n");
    }
    /**Improving sd writing performance*/
    s32Ret = cvi_system("echo bfq > /sys/block/mmcblk0/queue/scheduler");
    s32Ret |= cvi_system("echo 256 > /sys/block/mmcblk0/queue/nr_requests");
    if (0 != s32Ret) {
        CVI_LOGE("set nr_requests fail\n");
    }
    if (CVI_FILEMNG_SDConfigFileIsExist() == false) {
        CVI_EVENT_S stEvent = {0};
        if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_UNAVAILABLE))
            stEvent.topic = CVI_EVENT_MODEMNG_CARD_UNAVAILABLE;
        CVI_EVENTHUB_Publish(&stEvent);
        CVI_LOGE("Read SD_Config_File failed Need foramt\n");
        return;
    }
    s32Ret = CVI_FILEMNG_SetDiskState(true);
    if (0 != s32Ret) {
        CVI_EVENT_S stEvent = {0};
        if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_UNAVAILABLE))
            stEvent.topic = CVI_EVENT_MODEMNG_CARD_UNAVAILABLE;
        CVI_EVENTHUB_Publish(&stEvent);
        CVI_FILEMNG_SetDiskState(false);
        return;
    } else {
        s32Ret = CVI_FILEMNG_CheckDiskSpace();
        if (CVI_FILEMNG_EUNIDENTIFICATION == s32Ret) {
            CVI_EVENT_S stEvent = {0};
            if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_UNAVAILABLE))
                stEvent.topic = CVI_EVENT_MODEMNG_CARD_UNAVAILABLE;
            CVI_EVENTHUB_Publish(&stEvent);
            CVI_LOGD("publish CVI_EVENT_MODEMNG_CARD_UNAVAILABLE\n");
            CVI_FILEMNG_SetDiskState(false);
            return;
        }
    }

    /*recover video file*/
    CVI_PARAM_CFG_S sysparams;
    CVI_PARAM_GetParam(&sysparams);
    CVI_FILEMNG_PreallocateState(&FileMng.FileMngDtcf);
    uint32_t size = sysparams.MediaComm.Record.ChnAttrs[0].PreallocUnit;
    s32Ret = CVI_FILEMNG_AlignRecordFileSize(!(FileMng.FileMngDtcf.preAllocFilesEnable), size);
    if (0 != s32Ret) {
        CVI_LOGE("CVI_FILEMNG_AlignRecordFileSize failed!\n");
    }
    /*recover file end*/

    cvi_system("echo 3 > /proc/sys/vm/drop_caches");

    CVI_STG_DEV_INFO_S pstInfo;
    s32Ret = CVI_STORAGEMNG_GetInfo(&pstInfo);
    if (s32Ret == 0) {
        CVI_LOGD("pstInfo.enTranSpeed === %d \n",pstInfo.enTranSpeed);
        CVI_EVENT_S stEvent = {0};

        // Card just be formated, not publish available event.
        if (CVI_CARD_STATE_FORMATED == CVI_MODEMNG_GetCardState()) {
            // creat sd config file and init foramt flag
            CVI_MODEMNG_SetCardState(CVI_CARD_STATE_AVAILABLE);
        } else {
            if (CVI_MODEMNG_SetCardState(CVI_CARD_STATE_AVAILABLE)) {
                stEvent.topic = CVI_EVENT_MODEMNG_CARD_AVAILABLE;
            }
            CVI_EVENTHUB_Publish(&stEvent);
        }
    }
}

int32_t CVI_MODEMNG_InitStorage(void)
{
    int32_t s32Ret = 0;
    STG_DEVINFO_S stg_attr;
    CVI_PARAM_GetStgInfoParam(&stg_attr);
    s32Ret = CVI_STORAGEMNG_Create(&stg_attr);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"CVI_STORAGEMNG_Create !");

    return 0;
}

int32_t CVI_MODEMNG_DeintStorage(void)
{
    int32_t s32Ret = 0;
    s32Ret = CVI_STORAGEMNG_Destroy();
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"CVI_MODEMNG_DeintStorage !");
    return 0;
}

int32_t CVI_MODEMNG_Format(char *labelname)
{
    int32_t s32Ret = 0;

    mmc_term();

    s32Ret = CVI_FILEMNG_SetDiskState(false);
    MODEMNG_CHECK_EXPR_WITHOUT_RETURN(s32Ret,"CVI_FILEMNG_SetDiskState");

    s32Ret = CVI_STORAGEMNG_Format(labelname);
    if(s32Ret != 0) {
        s32Ret = CVI_STORAGEMNG_Mount();
        return -1;

    } else {
        s32Ret = CVI_STORAGEMNG_Mount();
        MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_STORAGEMNG_Mount");
        CVI_MODEMNG_SetCardState(CVI_CARD_STATE_FORMATED);
        STG_DEVINFO_S SDParam = {0};
        CVI_PARAM_GetStgInfoParam(&SDParam);
        CVI_FILEMNG_SetSdConfigPath(SDParam.aszMntPath);
        s32Ret = CVI_FILEMNG_CreateSDConfigFile();
        MODEMNG_CHECK_RET(s32Ret, CVI_MODE_EINVAL, "CVI_FILEMNG_CreateSDConfigFile");
        return 0;
    }
}


/** init global context structure */
int32_t CVI_MODEMNG_ContextInit(const CVI_MODEMNG_CONFIG_S* pstModemngCfg)
{
    /** init context structure */
    memcpy(&s_stModeMng.stModemngCfg, pstModemngCfg, sizeof(CVI_MODEMNG_CONFIG_S));
    return 0;
}

/** publish message process result event */
int32_t CVI_MODEMNG_PublishEvent(CVI_MESSAGE_S* pstMsg, int32_t s32Result)
{
    int32_t s32Ret = 0;

    /** check parameter */
    MODEMNG_CHECK_POINTER(pstMsg, CVI_MODE_ENULLPTR, "pstMsg error");

    CVI_EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(CVI_EVENT_S));
    memcpy(&stEvent, pstMsg, sizeof(CVI_EVENT_S));
    stEvent.s32Result = s32Result;

    CVI_LOGD("event(%x), s32Result(0x%x)\n\n", stEvent.topic, (uint32_t)s32Result);
    s32Ret = CVI_EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,CVI_MODE_EINTER,"Publish event !");

    return s32Ret;
}