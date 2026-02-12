#include "cvi_uvc.h"

#include <cvi_log.h>
#include <cvi_mapi.h>
#include <cvi_sysutils.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>
#include "cvi_usb.h"
#include "cvi_uvc_gadget.h"

/** UVC Stream Context */
typedef struct tagUVC_STREAM_CONTEXT_S {
    CVI_UVC_DEVICE_CAP_S stDeviceCap;
    CVI_UVC_DATA_SOURCE_S stDataSource;
    UVC_STREAM_ATTR_S stStreamAttr; /**<stream attribute, update by uvc driver */
    bool bVcapsStart;
    bool bVencStart;
    bool bFirstFrame;
    bool bInited;
} UVC_STREAM_CONTEXT_S;
static UVC_STREAM_CONTEXT_S s_stUVCStreamCtx;

int32_t UVC_STREAM_SetAttr(UVC_STREAM_ATTR_S *pstAttr) {
    s_stUVCStreamCtx.stStreamAttr = *pstAttr;
    CVI_LOGI("Format: %d, Resolution: %ux%u, FPS: %u, BitRate: %u", pstAttr->enFormat, pstAttr->u32Width,
             pstAttr->u32Height, pstAttr->u32Fps, pstAttr->u32BitRate);

    s_stUVCStreamCtx.bInited = false;
    s_stUVCStreamCtx.bVcapsStart = false;
    s_stUVCStreamCtx.bVencStart = false;
    s_stUVCStreamCtx.bFirstFrame = false;

    return 0;
}

static int32_t init_vproc() {
    UVC_STREAM_ATTR_S *pAttr = &s_stUVCStreamCtx.stStreamAttr;
    CVI_UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
    VPSS_CHN_ATTR_S stChnAttr;
    CVI_MAPI_VPROC_GetChnAttr(pstSrc->VprocHdl, pstSrc->VprocChnId, &stChnAttr);

    stChnAttr.u32Width = pAttr->u32Width;
    stChnAttr.u32Height = pAttr->u32Height;
    CVI_MAPI_VPROC_SetChnAttr(pstSrc->VprocHdl, pstSrc->VprocChnId, &stChnAttr);


    return 0;
}

static int32_t init_venc() {
    UVC_STREAM_ATTR_S *pAttr = &s_stUVCStreamCtx.stStreamAttr;
    CVI_UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
    CVI_MAPI_VENC_CHN_ATTR_T venc_attr = {0};

    venc_attr.venc_param.width = pAttr->u32Width;
    venc_attr.venc_param.height = pAttr->u32Height;
    venc_attr.venc_param.pixel_format = PIXEL_FORMAT_YUV_PLANAR_420;
    if (pAttr->enFormat == CVI_UVC_STREAM_FORMAT_MJPEG) {
        venc_attr.venc_param.codec = CVI_MAPI_VCODEC_MJP;
        venc_attr.venc_param.jpeg_quality = 80;
        venc_attr.venc_param.initialDelay = 1000;
        venc_attr.venc_param.single_EsBuf = 1;
        venc_attr.venc_param.bufSize = 729088;
    } else if (pAttr->enFormat == CVI_UVC_STREAM_FORMAT_H264) {
        venc_attr.venc_param.codec = CVI_MAPI_VCODEC_H264;
        venc_attr.venc_param.rate_ctrl_mode = 2;
        venc_attr.venc_param.src_framerate = 25;
        venc_attr.venc_param.dst_framerate = 25;
        venc_attr.venc_param.gop         = 25;
        venc_attr.venc_param.bitrate_kbps    = 12000;
        venc_attr.venc_param.iqp         = 36;
        venc_attr.venc_param.pqp         = 36;
        venc_attr.venc_param.minIqp     = 24;
        venc_attr.venc_param.maxIqp     = 42;
        venc_attr.venc_param.minQp      = 24;
        venc_attr.venc_param.maxQp      = 46;
        venc_attr.venc_param.videoSignalTypePresentFlag = 0;
        venc_attr.venc_param.videoFormat = 0;
        venc_attr.venc_param.videoFullRangeFlag = 0;
        venc_attr.venc_param.bufSize = 1048576;
        venc_attr.venc_param.initialDelay = 1000;
        venc_attr.venc_param.thrdLv = 2;
        venc_attr.venc_param.statTime = 2;
        venc_attr.venc_param.changePos = 90;
        venc_attr.venc_param.single_EsBuf = 1;
        venc_attr.venc_param.firstFrameStartQp = 36;
        venc_attr.venc_param.maxBitRate = 16000;
        venc_attr.venc_param.gop_mode = 0;
        venc_attr.venc_param.maxIprop = 100;
        venc_attr.venc_param.minIprop = 1;
        venc_attr.venc_param.minStillPercent = 60;
        venc_attr.venc_param.maxStillQP = 38;
        venc_attr.venc_param.avbrPureStillThr = 50;
        venc_attr.venc_param.motionSensitivity = 24;
        venc_attr.venc_param.bgDeltaQp = 0;
        venc_attr.venc_param.rowQpDelta = 0;
        venc_attr.venc_param.ipqpDelta = 3;
    }

    venc_attr.cb.stream_cb_func = NULL;
    venc_attr.cb.stream_cb_data = NULL;

    CVI_APPCOMM_CHECK_RETURN(CVI_MAPI_VENC_InitChn(&pstSrc->VencHdl, &venc_attr), CVI_USB_EINVAL);

    return CVI_MAPI_VENC_StartRecvFrame(pstSrc->VencHdl, -1);
}

int32_t UVC_STREAM_Start(void) {
    if (!s_stUVCStreamCtx.bInited) {
        if (0 != init_vproc()) {
            CVI_LOGE("init_vproc failed !");
            return -1;
        }

        if (0 != init_venc()) {
            CVI_LOGE("init_venc failed !");
            return -1;
        }
        s_stUVCStreamCtx.bInited = true;
    }

    return 0;
}

int32_t UVC_STREAM_Stop(void) {
    if (s_stUVCStreamCtx.bInited) {
        CVI_UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
        CVI_APPCOMM_CHECK_RETURN(CVI_MAPI_VENC_StopRecvFrame(pstSrc->VencHdl), CVI_USB_EINVAL);
        CVI_APPCOMM_CHECK_RETURN(CVI_MAPI_VENC_DeinitChn(pstSrc->VencHdl), CVI_USB_EINVAL);

        s_stUVCStreamCtx.bInited = false;
    }
    return 0;
}

int32_t UVC_STREAM_CopyBitStream(void *dst) {
    CVI_UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
    VIDEO_FRAME_INFO_S venc_frame;
    VENC_STREAM_S stream = {0};
    CVI_APPCOMM_CHECK_RETURN(CVI_MAPI_VPROC_GetChnFrame(pstSrc->VprocHdl, pstSrc->VprocChnId, &venc_frame), CVI_USB_EINVAL);
    if (CVI_MAPI_VENC_SendFrame(pstSrc->VencHdl, &venc_frame) != 0) {
        CVI_LOGE("UVC: CVI_MAPI_VENC_SendFrame failed");
        CVI_MAPI_ReleaseFrame(&venc_frame);
        return 0;
    }

    if (CVI_MAPI_VENC_GetStream(pstSrc->VencHdl, &stream) != 0) {
        CVI_LOGE("UVC: CVI_MAPI_VENC_GetStream failed");
        CVI_MAPI_ReleaseFrame(&venc_frame);
        return 0;
    }

    bool is_i_frame;

    uint32_t bitstream_size = 0;
    for (unsigned i = 0; i < stream.u32PackCount; i++) {
        VENC_PACK_S *ppack;
        ppack = &stream.pstPack[i];
        CVI_MAPI_VENC_GetStreamStatus(pstSrc->VencHdl, ppack, &is_i_frame);

        memcpy(dst + bitstream_size, ppack->pu8Addr + ppack->u32Offset, ppack->u32Len - ppack->u32Offset);
        bitstream_size += ppack->u32Len - ppack->u32Offset;
    }

    CVI_MAPI_VENC_ReleaseStream(pstSrc->VencHdl, &stream);
    CVI_MAPI_ReleaseFrame(&venc_frame);

    return bitstream_size;
}

int32_t UVC_STREAM_ReqIDR(void) {
    // TODO: implement
    return 0;
}

/** UVC Context */
static UVC_CONTEXT_S s_stUVCCtx = {.bRun = false, .bPCConnect = false, .TskId = 0, .Tsk2Id = 0};

bool g_bPushVencData = false;

static void *UVC_CheckTask(void *pvArg) {
    int32_t ret = 0;
    prctl(PR_SET_NAME, "UVC_Check", 0, 0, 0);
    while (s_stUVCCtx.bRun) {
        ret = UVC_GADGET_DeviceCheck();

        if (ret < 0) {
            CVI_LOGD("UVC_GADGET_DeviceCheck %x\n", ret);
            break;
        } else if (ret == 0) {
            CVI_LOGD("Timeout Do Nothing\n");
            if (false != g_bPushVencData) {
                g_bPushVencData = false;
            }
        }
        usleep(10 * 1000);
    }


    return NULL;
}

static int32_t UVC_LoadMod(void) {
    static bool first = true;
    if(first == false) {
        return 0;
    }
    first = false;
    CVI_LOGD("Uvc insmod ko successfully!");
/*
    cvi_insmod(CVI_KOMOD_PATH"/usb-common.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/udc-core.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/libcomposite.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/usbcore.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/roles.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/dwc2.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/libcomposite.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/videobuf2-common.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/videobuf2-memops.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/videobuf2-v4l2.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/videobuf2-vmalloc.ko", NULL);
    cvi_insmod(CVI_KOMOD_PATH"/usb_f_uvc.ko", NULL);
    cvi_system("echo device > /proc/cviusb/otg_role");
*/
    cvi_system(CVI_UVC_SCRIPTS_PATH"/run_usb.sh probe uvc");
    cvi_system(CVI_UVC_SCRIPTS_PATH"/ConfigUVC.sh");
    cvi_system(CVI_UVC_SCRIPTS_PATH"/run_usb.sh start");
    // cvi_system("devmem 0x030001DC 32 0x8844");
    return 0;
}

static int32_t UVC_UnLoadMod(void) {
    CVI_LOGD("Do nothing now, due to the ko can NOT rmmod successfully!");
    // cvi_rmmod(CVI_KOMOD_PATH "/videobuf2-memops.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/videobuf2-vmalloc.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/configfs.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/libcomposite.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/u_serial.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/usb_f_acm.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/cvi_usb_f_cvg.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/usb_f_uvc.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/u_audio.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/usb_f_uac1.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/usb_f_serial.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/usb_f_mass_storage.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/u_ether.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/usb_f_ecm.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/usb_f_eem.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/usb_f_rndis.ko");
    // cvi_rmmod(CVI_KOMOD_PATH "/cv183x_usb_gadget.ko");

    return 0;
}

int32_t UVC_Init(const CVI_UVC_DEVICE_CAP_S *pstCap, const CVI_UVC_DATA_SOURCE_S *pstDataSrc,
                 CVI_UVC_BUFFER_CFG_S *pstBufferCfg) {
    CVI_APPCOMM_CHECK_POINTER(pstCap, -1);
    CVI_APPCOMM_CHECK_POINTER(pstDataSrc, -1);

    UVC_LoadMod();

    s_stUVCStreamCtx.stDeviceCap = *pstCap;
    s_stUVCStreamCtx.stDataSource = *pstDataSrc;
    UVC_GADGET_Init(pstCap, pstBufferCfg->u32BufSize);

    // TODO: Do we need handle CVI_UVC_BUFFER_CFG_S?
    return 0;
}

int32_t UVC_Deinit(void) {
    UVC_UnLoadMod(); // TODO, Not work right now

    return 0;
}

int32_t UVC_Start(const char *pDevPath) {
    CVI_APPCOMM_CHECK_POINTER(pDevPath, -1);

    if (false == s_stUVCCtx.bRun) {
        strcpy(s_stUVCCtx.szDevPath, pDevPath);

        if (UVC_GADGET_DeviceOpen(pDevPath)) {
            CVI_LOGD("UVC_GADGET_DeviceOpen Failed!");
            return -1;
        }

        s_stUVCCtx.bPCConnect = false;
        s_stUVCCtx.bRun = true;
        pthread_attr_t pthread_attr;
        pthread_attr_init(&pthread_attr);
        pthread_attr_setschedpolicy(&pthread_attr, SCHED_OTHER);
        if (pthread_create(&s_stUVCCtx.TskId, &pthread_attr, UVC_CheckTask, NULL)) {
            CVI_LOGE("UVC_CheckTask create thread failed!\n");
            pthread_attr_destroy(&pthread_attr);
            s_stUVCCtx.bRun = false;
            return -1;
        }
        pthread_attr_destroy(&pthread_attr);
        CVI_LOGD("UVC_CheckTask create thread successful\n");
    } else {
        CVI_LOGD("UVC already started\n");
    }

    return 0;
}

int32_t UVC_Stop(void) {
    if (false == s_stUVCCtx.bRun) {
        CVI_LOGE("UVC not run\n");
        return 0;
    }

    s_stUVCCtx.bRun = false;
    pthread_join(s_stUVCCtx.TskId, NULL);

    return UVC_GADGET_DeviceClose();
}

UVC_CONTEXT_S *UVC_GetCtx(void) { return &s_stUVCCtx; }
