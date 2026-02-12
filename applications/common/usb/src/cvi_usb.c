/**
 * @file      cvi_usb.c
 * @brief     usb interface implementation
 * @version   1.0
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "cvi_log.h"
#include "cvi_sysutils.h"
#include "cvi_usb.h"
#include "cvi_uvc_gadget.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** usb connect state */
typedef enum tagUSB_CONNECT_STATE_E
{
    USB_CONNECT_STATE_DISCONNECT = 0,
    USB_CONNECT_STATE_CHARGE,
    USB_CONNECT_STATE_PC_CONNECTED,
    USB_CONNECT_STATE_BUTT
} USB_CONNECT_STATE_E;

/** USB Context */
typedef struct tagUSB_CONTEXT_S
{
    bool bInit;
    bool bCheckTskRun; /**<check task run flag */
    pthread_t CheckTskId; /**<check task thread id */
    CVI_USB_MODE_E  enMode;
    CVI_USB_STATE_E enState;
    CVI_USB_CFG_S stUsbCfg;
    pthread_mutex_t stateMutex;
} USB_CONTEXT_S;
static USB_CONTEXT_S s_stUSBCtx =
{
    .bInit = false,
};

void CVI_USB_CheckPower_Soure(USB_POWER_SOURCE_E* PowerSoureState)
{
    char * Powerprocfile = "/proc/cviusb/chg_det_fast";
#define MAX_SCRLIEN 128
    FILE* fp = NULL;
    char temp[MAX_SCRLIEN];
    char szStateString[MAX_SCRLIEN] = {0,};
    memset(temp, 0x0, sizeof(char)*MAX_SCRLIEN);
    fp = fopen(Powerprocfile, "r");
    if (fp!= NULL) {
        if (!feof(fp)) {
            while (fgets(temp, (MAX_SCRLIEN-1), fp)!=NULL) {
               snprintf(szStateString, MAX_SCRLIEN, "%s", temp);
               break;
            }
        }
        fclose(fp);

        if (strstr(szStateString, "hub")) {
            *PowerSoureState = USB_POWER_SOURCE_PC;
        } else if(strstr(szStateString, "adapter")) {
            *PowerSoureState = USB_POWER_SOURCE_POWER;
        } else if(strstr(szStateString, "none")) {
            *PowerSoureState = USB_POWER_SOURCE_NONE;
        }
    }
}

static inline const char * USB_GetModeStr(CVI_USB_MODE_E enMode)
{
    switch(enMode)
    {
        case CVI_USB_MODE_CHARGE:
            return "Charge";
        case CVI_USB_MODE_UVC:
            return "UVC";
        case CVI_USB_MODE_STORAGE:
            return "USBStorage";
        case CVI_USB_MODE_HOSTUVC:
            return "Host UVC";
        default:
            return "Unknown";
    }
}

static inline const char * USB_GetStateStr(CVI_USB_STATE_E enState)
{
    switch(enState)
    {
        case CVI_USB_STATE_OUT:
            return "Out";
        case CVI_USB_STATE_INSERT:
            return "Insert";
        case CVI_USB_STATE_UVC_READY:
            return "UVC Ready";
        case CVI_USB_STATE_UVC_PC_READY:
            return "UVC PC Ready";
        case CVI_USB_STATE_UVC_MEDIA_READY:
            return "UVC Media Ready";
        case CVI_USB_STATE_STORAGE_READY:
            return "Storage Ready";
        case CVI_USB_STATE_STORAGE_PC_READY:
            return "Storage PC Ready";
        case CVI_USB_STATE_STORAGE_SD_READY:
            return "Storage SD Ready";
        case CVI_USB_STATE_HOSTUVC_READY:
            return "Host UVC Ready";
        case CVI_USB_STATE_HOSTUVC_CAMERA_READY:
            return "Host UVC Camera Ready";
        case CVI_USB_STATE_HOSTUVC_MEDIA_READY:
            return "Host UVC Media Ready";
        default:
            return "Unknown";
    }
}

static inline void USB_SetState(CVI_USB_STATE_E enState)
{
    if (enState != s_stUSBCtx.enState) {
        CVI_LOGD("%s -> %s\n", USB_GetStateStr(s_stUSBCtx.enState), USB_GetStateStr(enState));
        s_stUSBCtx.enState = enState;
    }
}

static inline int32_t USB_EventProc(const CVI_USB_EVENT_INFO_S* pstEvent)
{
    CVI_APPCOMM_CHECK_POINTER(s_stUSBCtx.stUsbCfg.pfnEventProc, -1);
    return s_stUSBCtx.stUsbCfg.pfnEventProc(pstEvent);
}


static void *USB_StateCheckProc(void *arg)
{
    prctl(PR_SET_NAME, "USB_StateCheckProc", 0, 0, 0);
    USB_POWER_SOURCE_E enPowerState = USB_POWER_SOURCE_NONE;
    USB_POWER_SOURCE_E enPowerLastState = USB_POWER_SOURCE_NONE;
    while (s_stUSBCtx.bCheckTskRun) {
        CVI_USB_CheckPower_Soure(&enPowerState);
        if(enPowerState != enPowerLastState){
            if (USB_POWER_SOURCE_PC == enPowerState) {
                USB_SetState(CVI_USB_STATE_PC_INSERT);
                CVI_USB_EVENT_INFO_S stEventInfo;
                stEventInfo.s32EventId = CVI_EVENT_USB_PC_INSERT;
                USB_EventProc(&stEventInfo);
            } else if (USB_POWER_SOURCE_POWER == enPowerState) {
                USB_SetState(CVI_USB_STATE_INSERT);
                CVI_USB_EVENT_INFO_S stEventInfo;
                stEventInfo.s32EventId = CVI_EVENT_USB_INSERT;
                USB_EventProc(&stEventInfo);
            } else if (enPowerLastState != USB_POWER_SOURCE_NONE && USB_POWER_SOURCE_NONE == enPowerState) {
                USB_SetState(CVI_USB_STATE_OUT);
                CVI_USB_EVENT_INFO_S stEventInfo;
                stEventInfo.s32EventId = CVI_EVENT_USB_OUT;
                USB_EventProc(&stEventInfo);
                CVI_LOGD("CVI_EVENT_USB_OUT ...................................");
            }
            enPowerLastState = enPowerState;
        }
        cvi_usleep(100 * 1000);
    }

    return NULL;
}

int32_t CVI_USB_Init(const CVI_USB_CFG_S *pstCfg)
{
    CVI_APPCOMM_CHECK_POINTER(pstCfg, CVI_USB_EINVAL);
    CVI_APPCOMM_CHECK_POINTER(pstCfg->pfnEventProc, CVI_USB_EINVAL);
    if (s_stUSBCtx.bInit) {
        CVI_LOGE("has already inited!\n");
        return 0;
    }
    pthread_mutex_init(&s_stUSBCtx.stateMutex, NULL);

    /* record usb configure */
    memcpy(&s_stUSBCtx.stUsbCfg, pstCfg, sizeof(CVI_USB_CFG_S));

    /* Create usb check task thread */
    s_stUSBCtx.bCheckTskRun = true;
    s_stUSBCtx.enMode = CVI_USB_MODE_CHARGE;
    s_stUSBCtx.enState = CVI_USB_STATE_BUTT;
    s_stUSBCtx.bInit = true;

    // cvi_insmod(CVI_KOMOD_PATH"/usb-common.ko", NULL);
    // cvi_insmod(CVI_KOMOD_PATH"/udc-core.ko", NULL);
    // cvi_insmod(CVI_KOMOD_PATH"/usbcore.ko", NULL);
    // cvi_insmod(CVI_KOMOD_PATH"/roles.ko", NULL);
    // cvi_insmod(CVI_KOMOD_PATH"/dwc2.ko", NULL);
    cvi_system("devmem 0x05027018 32 0x40");
    cvi_system("devmem 0x03001820 32 0x40");
    cvi_insmod(CVI_KOMOD_PATH"/libcomposite.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/videobuf2-common.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/videobuf2-memops.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/videobuf2-v4l2.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/videobuf2-vmalloc.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/usb_f_uvc.ko", NULL);
    cvi_system("echo device > /proc/cviusb/otg_role");

    USB_POWER_SOURCE_E enPowerState = USB_POWER_SOURCE_BUTT;
    CVI_USB_CheckPower_Soure(&enPowerState);

    if (pthread_create(&s_stUSBCtx.CheckTskId, NULL, USB_StateCheckProc, NULL)) {
        CVI_LOGE("USB_CheckTask create failed\n");
        s_stUSBCtx.bCheckTskRun = false;
        return -1;
    }

    return 0;
}

int32_t CVI_USB_SetMode(CVI_USB_MODE_E enMode)
{
    CVI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, CVI_USB_ENOTINIT);
    CVI_APPCOMM_CHECK_EXPR((enMode < CVI_USB_MODE_BUTT) && (enMode >= CVI_USB_MODE_CHARGE), CVI_USB_EINVAL);
    CVI_LOGD("usb mode change[%s->%s]\n", USB_GetModeStr(s_stUSBCtx.enMode), USB_GetModeStr(enMode));
    pthread_mutex_lock(&s_stUSBCtx.stateMutex);
    if (s_stUSBCtx.enMode == enMode) {
        pthread_mutex_unlock(&s_stUSBCtx.stateMutex);
        return 0;
    }

    if (CVI_USB_MODE_UVC == s_stUSBCtx.enMode) {
        UVC_Stop();
        UVC_Deinit();
    }
    else if (CVI_USB_MODE_STORAGE == s_stUSBCtx.enMode) {
        USB_STORAGE_Deinit();
    } else if (s_stUSBCtx.enMode == CVI_USB_MODE_HOSTUVC) {
       // USB_HostUvcDeinit();
       CVI_LOGE("host uvc not support now\n");
    }

    if (CVI_USB_MODE_UVC == enMode) {
        // UVC_Init(&s_stUSBCtx.stUsbCfg.stUvcCfg.stDevCap, &s_stUSBCtx.stUsbCfg.stUvcCfg.stDataSource,
        //     &s_stUSBCtx.stUsbCfg.stUvcCfg.stBufferCfg);
        // UVC_Start(s_stUSBCtx.stUsbCfg.stUvcCfg.szDevPath);
    } else if (CVI_USB_MODE_STORAGE == enMode) {
        USB_STORAGE_Init(s_stUSBCtx.stUsbCfg.stStorageCfg.szDevPath);
    } else if (enMode == CVI_USB_MODE_HOSTUVC) {
        CVI_LOGE("host uvc not support now\n");
        //USB_HostUvcInit();
    }
    s_stUSBCtx.enMode = enMode;
    pthread_mutex_unlock(&s_stUSBCtx.stateMutex);
    return 0;
}

int32_t CVI_USB_Deinit(void)
{
   // CVI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, CVI_USB_ENOTINIT);

    /* Destroy check task */
    s_stUSBCtx.bCheckTskRun = false;
    pthread_join(s_stUSBCtx.CheckTskId, NULL);

   // USB_DeinitProc(&s_stUSBCtx);
    s_stUSBCtx.bInit = false;
    pthread_mutex_destroy(&s_stUSBCtx.stateMutex);
    return 0;
}

int32_t CVI_USB_SetUvcCfg(const CVI_UVC_CFG_S* pstCfg)
{
    CVI_APPCOMM_CHECK_POINTER(pstCfg, CVI_USB_EINVAL);
    CVI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, CVI_USB_ENOTINIT);
    memcpy(&s_stUSBCtx.stUsbCfg.stUvcCfg, pstCfg, sizeof(CVI_UVC_CFG_S));
    return 0;
}

int32_t CVI_USB_GetUvcCfg(CVI_USB_CFG_S* pstCfg)
{
/*     CVI_APPCOMM_CHECK_POINTER(pstCfg, CVI_USB_EINVAL);
    CVI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, CVI_USB_ENOTINIT); */
    memcpy(pstCfg, &s_stUSBCtx.stUsbCfg, sizeof(CVI_USB_CFG_S));
    return 0;
}

int32_t CVI_USB_SetStorageCfg(const CVI_USB_STORAGE_CFG_S* pstCfg)
{
    CVI_APPCOMM_CHECK_POINTER(pstCfg, CVI_USB_EINVAL);
    CVI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, CVI_USB_ENOTINIT);
    memcpy(&s_stUSBCtx.stUsbCfg.stStorageCfg, pstCfg, sizeof(CVI_USB_STORAGE_CFG_S));
    return 0;
}

int32_t CVI_USB_GetStorageCfg(CVI_USB_STORAGE_CFG_S* pstCfg)
{
    memcpy(pstCfg, &s_stUSBCtx.stUsbCfg.stStorageCfg, sizeof(CVI_USB_STORAGE_CFG_S));
    return 0;
}

int32_t CVI_USB_GetMode(CVI_USB_MODE_E *penMode)
{
    CVI_APPCOMM_CHECK_POINTER(penMode, CVI_USB_EINVAL);
    CVI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, CVI_USB_ENOTINIT);
    CVI_LOGD("usb mode[%s]\n", USB_GetModeStr(s_stUSBCtx.enMode));
    *penMode = s_stUSBCtx.enMode;
    return 0;
}

int32_t CVI_USB_GetState(CVI_USB_STATE_E *penState)
{
    CVI_APPCOMM_CHECK_POINTER(penState, CVI_USB_EINVAL);
    CVI_APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, CVI_USB_ENOTINIT);
    //CVI_LOGD("usb state[%s]\n", USB_GetStateStr(s_stUSBCtx.enState));
    *penState = s_stUSBCtx.enState;
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

