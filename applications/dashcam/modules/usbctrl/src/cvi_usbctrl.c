#include <pthread.h>

#include "cvi_usbctrl.h"
#include "cvi_media_init.h"
#include "cvi_eventhub.h"
#include "cvi_appcomm.h"
#include "cvi_mode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/** usbcontrol context */
typedef struct cvi_USBCTRL_CONTEXT_S
{
    CVI_MW_PTR pSubscriber;
    pthread_mutex_t EventMutex;
    pthread_cond_t  EventCond;
    int32_t  s32WaitedValue; /* Waited Value */
    bool pause;
    bool waitedFlag;
    CVI_USB_MODE_E prevMode;
    pthread_mutex_t pauseMutex;
} CVI_USBCTRL_CONTEXT_S;
static CVI_USBCTRL_CONTEXT_S s_stUSBCTRLCtx;

static inline const char* CVI_USBCTRL_GetEventStr(int32_t  enEvent)
{
    if (CVI_EVENT_USB_OUT == enEvent) {
        return "USB Out Event";
    } else if (CVI_EVENT_USB_INSERT == enEvent) {
        return "USB Insert Event";
    } else if (CVI_EVENT_USB_UVC_READY == enEvent) {
        return "UVC Ready Event";
    } else if (CVI_EVENT_USB_STORAGE_READY == enEvent) {
        return "USB Storage Ready Event";
    } else if (enEvent == CVI_EVENT_USB_HOSTUVC_READY) {
        return "HOST UVC Ready Event";
    }  else if (enEvent == CVI_EVENT_USB_HOSTUVC_PC) {
        return "HOST UVC Source pc Event";
    } else if (enEvent == CVI_EVENT_USB_HOSTUVC_HEAD) {
        return "HOST UVC Source head Event";
    } else {
        return "Unknown Usb Event";
    }
}

static int32_t  CVI_USBCTRL_SubscribeEventProc(void *argv, CVI_EVENT_S *pstEvent)
{
    CVI_APPCOMM_CHECK_POINTER(pstEvent, -1);

    CVI_MUTEX_LOCK(s_stUSBCTRLCtx.pauseMutex);
    if (pstEvent->arg2 == s_stUSBCTRLCtx.s32WaitedValue){
        s_stUSBCTRLCtx.waitedFlag = false;
    }
    CVI_MUTEX_UNLOCK(s_stUSBCTRLCtx.pauseMutex);

    CVI_MUTEX_LOCK(s_stUSBCTRLCtx.EventMutex);
    if (pstEvent->arg2 == s_stUSBCTRLCtx.s32WaitedValue){
        pthread_cond_signal(&s_stUSBCTRLCtx.EventCond);
        s_stUSBCTRLCtx.s32WaitedValue = -1;
    }
    CVI_MUTEX_UNLOCK(s_stUSBCTRLCtx.EventMutex);
    return 0;
}

static int32_t  CVI_USBCTRL_SubscribeEvent(CVI_USBCTRL_CONTEXT_S* pstCtx)
{
    CVI_EVENTHUB_SUBSCRIBER_S stSubscriber = {"USBCtrl", NULL, CVI_USBCTRL_SubscribeEventProc, true};
    CVI_EVENTHUB_CreateSubscriber(&stSubscriber, &pstCtx->pSubscriber);
    CVI_LOGD("subscribe create success[%p]\n", pstCtx->pSubscriber);
    return 0;
}

static int32_t  CVI_USBCTRL_EventProc(const CVI_USB_EVENT_INFO_S *pstEventInfo)
{
    CVI_EVENT_S stEvent;
    switch (pstEventInfo->s32EventId)
    {
        case CVI_EVENT_USB_OUT:
        {
            CVI_LOGI("%s\n", CVI_USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = CVI_EVENT_USB_OUT;
            CVI_EVENTHUB_Publish(&stEvent);
            break;
        }
        case CVI_EVENT_USB_INSERT:
        {
            CVI_LOGI("%s\n", CVI_USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = CVI_EVENT_USB_INSERT;
            CVI_EVENTHUB_Publish(&stEvent);
            break;
        }
        case CVI_EVENT_USB_PC_INSERT:
        {
            CVI_LOGI("%s\n", CVI_USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = CVI_EVENT_USB_PC_INSERT;
            CVI_EVENTHUB_Publish(&stEvent);
            break;
        }
        case CVI_EVENT_USB_UVC_READY:{
            CVI_LOGI("s_stUSBCTRLCtx.pause=%d\n", s_stUSBCTRLCtx.pause);
            CVI_MUTEX_LOCK(s_stUSBCTRLCtx.pauseMutex);
            if(s_stUSBCTRLCtx.pause == false) {
                s_stUSBCTRLCtx.waitedFlag = true;
                CVI_MUTEX_UNLOCK(s_stUSBCTRLCtx.pauseMutex);

                CVI_LOGI("s32EventId=%s\n", CVI_USBCTRL_GetEventStr(pstEventInfo->s32EventId));
                stEvent.topic = CVI_EVENT_USB_UVC_READY;

                CVI_LOGD("Wait uvc workmode event...\n");
                CVI_MUTEX_LOCK(s_stUSBCTRLCtx.EventMutex);
                CVI_EVENTHUB_Publish(&stEvent);

                s_stUSBCTRLCtx.s32WaitedValue = 0;

                //pthread_cond_wait(&s_stUSBCTRLCtx.EventCond, &s_stUSBCTRLCtx.EventMutex);
                CVI_MUTEX_UNLOCK(s_stUSBCTRLCtx.EventMutex);
                CVI_LOGD("Wait OK\n");
            } else {
                CVI_LOGW("ignore UVC_READY proc\n");
                CVI_MUTEX_UNLOCK(s_stUSBCTRLCtx.pauseMutex);
            }
            break;
        }

        case CVI_EVENT_USB_STORAGE_READY:{
            CVI_LOGI("s_stUSBCTRLCtx.pause=%d\n", s_stUSBCTRLCtx.pause);
            CVI_MUTEX_LOCK(s_stUSBCTRLCtx.pauseMutex);
            if(s_stUSBCTRLCtx.pause == false) {
                CVI_LOGI("s32EventId=%s\n", CVI_USBCTRL_GetEventStr(pstEventInfo->s32EventId));
                s_stUSBCTRLCtx.waitedFlag = true;
                CVI_MUTEX_UNLOCK(s_stUSBCTRLCtx.pauseMutex);

                stEvent.topic = CVI_EVENT_USB_STORAGE_READY;

                CVI_LOGI("Wait usb storage workmode event...\n");
                CVI_MUTEX_LOCK(s_stUSBCTRLCtx.EventMutex);
                CVI_EVENTHUB_Publish(&stEvent);

                s_stUSBCTRLCtx.s32WaitedValue = 1;
                pthread_cond_wait(&s_stUSBCTRLCtx.EventCond, &s_stUSBCTRLCtx.EventMutex);
                CVI_MUTEX_UNLOCK(s_stUSBCTRLCtx.EventMutex);
                CVI_LOGI("Wait OK\n");
            } else {
                CVI_LOGW("ignore STORAGE_READY proc\n");
                CVI_MUTEX_UNLOCK(s_stUSBCTRLCtx.pauseMutex);
            }
            break;
        }

        case CVI_EVENT_USB_HOSTUVC_READY: {
            CVI_LOGI("%s\n", CVI_USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = CVI_EVENT_USB_HOSTUVC_READY;
            CVI_EVENTHUB_Publish(&stEvent);
            break;
        }

        case CVI_EVENT_USB_HOSTUVC_PC:
        {
            CVI_LOGI("%s\n", CVI_USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = CVI_EVENT_USB_HOSTUVC_PC;
            CVI_EVENTHUB_Publish(&stEvent);
            break;
        }

        case CVI_EVENT_USB_HOSTUVC_HEAD:
        {
            CVI_LOGI("%s\n", CVI_USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = CVI_EVENT_USB_HOSTUVC_HEAD;
            CVI_EVENTHUB_Publish(&stEvent);
            break;
        }
        default:
            CVI_LOGW("Invalid Event[%08x]\n", pstEventInfo->s32EventId);
            return -1;
    }

    return 0;
}

static int32_t  CVI_USBCTRL_GetStorageState(void* pvPrivData)
{
    // if(NULL == pvPrivData)
    // {
    //     return -1;
    // }

    // int32_t  ret = 0;
    // bool* pbStorageReady = (bool*)pvPrivData;
    // *pbStorageReady = false;

    // CVI_STORAGEMNG_CFG_S stStorageMngCfg;
    // ret = HI_PDT_PARAM_GetStorageCfg(&stStorageMngCfg);
    // HI_APPCOMM_CHECK_RETURN(ret, -1);

    // /** check sd state */
    // CVI_STORAGE_STATE_E enState = CVI_STORAGE_STATE_IDEL;
    // ret = CVI_STORAGEMNG_GetState(stStorageMngCfg.szMntPath, &enState);
    // HI_APPCOMM_CHECK_RETURN(ret, -1);
    // CVI_LOGI("storage state(%d)\n", enState);

    // if(CVI_STORAGE_STATE_MOUNTED == enState)
    // {
    //     *pbStorageReady = true;
    // }

    return 0;
}

int32_t  CVI_USBCTRL_Init(void)
{
    int32_t  ret = 0;
    CVI_USB_CFG_S stUsbCfg;
    CVI_PARAM_USB_MODE_S UsbModeParam = {0};
    CVI_UVC_CFG_S UvcCfg = {0};
    stUsbCfg.pfnEventProc = CVI_USBCTRL_EventProc;
    stUsbCfg.pfnGetStorageState = CVI_USBCTRL_GetStorageState;

    CVI_PARAM_GetUsbParam(&UsbModeParam);

    /* usb storage configure */
    memcpy(&stUsbCfg.stStorageCfg, &UsbModeParam.StorageCfg, sizeof(CVI_USB_STORAGE_CFG_S));

    /* usb uvc configure */
    CVI_MEDIA_PARAM_INIT_S *MediaParams = CVI_MEDIA_GetCtx();
    CVI_PARAM_MEDIA_SPEC_S params = {0};
    CVI_PARAM_GetMediaMode(UsbModeParam.UvcParam.VcapId, &params);
    /* get vprochdl venchdl */
    for (int32_t  z = 0; z < MAX_VPROC_CNT; z++) {
        if ((MediaParams->SysHandle.vproc[z] != NULL) &&
            (UsbModeParam.UvcParam.VprocId == (uint32_t)CVI_MAPI_VPROC_GetGrp(MediaParams->SysHandle.vproc[z]))) {
            UvcCfg.stDataSource.VprocHdl = MediaParams->SysHandle.vproc[z];
            break;
        }
    }
    UvcCfg.stDataSource.VprocChnId = UsbModeParam.UvcParam.VprocChnId;
    memcpy(&UvcCfg.attr, &UsbModeParam.UvcParam.UvcCfg, sizeof(CVI_UVC_CFG_ATTR_S));
    memcpy(&stUsbCfg.stUvcCfg, &UvcCfg, sizeof(CVI_UVC_CFG_S));

    /* subscribe event */
    CVI_USBCTRL_SubscribeEvent(&s_stUSBCTRLCtx);
    CVI_MUTEX_INIT_LOCK(s_stUSBCTRLCtx.EventMutex);
    CVI_COND_INIT(s_stUSBCTRLCtx.EventCond);
    CVI_MUTEX_INIT_LOCK(s_stUSBCTRLCtx.pauseMutex);

    ret = CVI_USB_Init(&stUsbCfg);
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(ret, ret,"InitUsb");

    CVI_USB_MODE_E enUsbMode;
    enUsbMode = CVI_USB_MODE_CHARGE;

    ret = CVI_USB_SetMode(enUsbMode);
    s_stUSBCTRLCtx.prevMode = enUsbMode;
    CVI_APPCOMM_CHECK_RETURN_WITH_ERRINFO(ret, ret,"SetUsbMode");

    return 0;
}

int32_t  CVI_USBCTRL_Deinit(void)
{
    return CVI_USB_Deinit();
}

int32_t  CVI_USBCTRL_RegisterEvent(void)
{
    int32_t  ret = 0;
    ret  = CVI_EVENTHUB_RegisterTopic(CVI_EVENT_USB_OUT);
    ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_USB_INSERT);
    ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_USB_UVC_READY);
    ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_USB_STORAGE_READY);
    ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_USB_HOSTUVC_READY);
    ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_USB_HOSTUVC_PC);
    ret |= CVI_EVENTHUB_RegisterTopic(CVI_EVENT_USB_HOSTUVC_HEAD);
    CVI_APPCOMM_CHECK_RETURN(ret, -1);
    CVI_LOGD("Success\n");
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif