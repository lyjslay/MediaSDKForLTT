#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>

#include "cv181x_snd.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vdec.h"
#include "cvi_vo.h"
#include "cvi_math.h"
#include "cvi_buffer.h"
#include "cvi_msg_server.h"
#include "cvi_sns_ctrl.h"
#include "sensor_cfg.h"
#include "sys_uapi.h"
#include "vi_snsr_i2c.h"
#include "vi_uapi.h"
#include "vpss_uapi.h"
#include "vo_uapi.h"
#include "rgn_uapi.h"
#include "ldc_uapi.h"
#include "cvi_hal_gpio.h"
#include "cvi_hal_pwm.h"
#include "cvi_hal_screen.h"
#include "cvi_mipi_tx.h"
#include "cvi_comm_vb.h"
#include "cvi_comm_vdec.h"
#include "cvi_board_memmap.h"
#include "cvi_param.h"
//#include "cvi_comm_aio.h"
// #include "bootsound.h"
//#include "alsa/pcm.h"
#include "cvi_ahd_info.h"

#define READ_LEN_MAX 256*1024

CVI_PARAM_CFG_S *pstCfg = (CVI_PARAM_CFG_S *)CVIMMAP_SHARE_PARAM_ADDR;

typedef struct _VDEC_ATTR {
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	VIDEO_MODE_E   enMode;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	CVI_U32 u32FrameBufCnt;
	CVI_U32 u32DisplayFrameNum;
	CVI_U32 u32VdecChn;
	CVI_U32 vdecFrameNum;
	CVI_CHAR *inFile;
	CVI_CHAR *outFile;
} VDEC_ATTR;

VDEC_ATTR vdec_attr_logo = {
	.u32VdecChn = 0,
	.enType = PT_JPEG,
	.enPixelFormat = PIXEL_FORMAT_NV21,
	.enMode = VIDEO_MODE_FRAME,
	.u32Width = 1280,
	.u32Height = 720,
	.u32FrameBufCnt = 1,
	.u32DisplayFrameNum = 1,
	.vdecFrameNum = 1,
};

CVI_PARAM_CFG_S* get_paramcfg(void)
{
    if (CVIMMAP_SHARE_PARAM_SIZE != 0) {
        return pstCfg;
    }

    return NULL;
}

int cvi_ahd_init(void)
{
    CVI_U8 mclk_freq;
    SNS_COMBO_DEV_ATTR_S attr;

    if (sizeof(gAhdInfo) == 0) {
        CVI_TRACE_SNS(CVI_DBG_WARN, "no ahd device support!\n");
        return CVI_SUCCESS;
    }

    for (CVI_S8 i = 0; i < sizeof(gAhdInfo) / sizeof(CVI_AHD_INFO_S); i++) {
        if (gAhdInfo[i].pipe == -1)
            continue;

        CVI_S32	snsr_type = get_sensor_type(gAhdInfo[i].pipe);
        SNS_AHD_OBJ_S *pstAhdObj = getAhdObj(snsr_type);
        if (!pstAhdObj) {
            CVI_TRACE_SNS(CVI_DBG_ERR, "fail to get Ahd obj ,error snsr_type:%d !!!", snsr_type);
            continue;
        }
        ISP_SNS_OBJ_S *pstSnsObj = getSnsObj(snsr_type);
        if (!pstSnsObj) {
            CVI_TRACE_SNS(CVI_DBG_ERR, "fail to get sns obj ,error snsr_type:%d !!!", snsr_type);
            continue;
        }
        memset(&attr, 0, sizeof(SNS_COMBO_DEV_ATTR_S));
        if(pstSnsObj->pfnGetRxAttr)
            pstSnsObj->pfnGetRxAttr(gAhdInfo[i].pipe, &attr);
        mclk_freq = attr.mclk.freq;
        cif_enable_ahd_clk(attr.mclk.cam, true, mclk_freq);
        if(pstAhdObj->pfnSetAhdBusInfo)
            pstAhdObj->pfnSetAhdBusInfo(gAhdInfo[i].pipe, gAhdInfo[i].businfo);
        if(pstAhdObj->pfnAhdInit)
            pstAhdObj->pfnAhdInit(gAhdInfo[i].pipe, true);
    }

    return CVI_SUCCESS;
}

void CVI_Media_Init(void)
{
	//sys/base init
    sys_core_init();
    //vip init
    cvi_cif_init();
    cvi_snsr_i2c_probe();
    vi_core_init();
    vpss_core_init();
    vo_core_init();
    rgn_core_init();
    cvi_ldc_probe();
    cvi_ahd_init();

    CVI_MSG_Init();
    // CVI_SYS_Init();

    // snd_card_register(NULL);
}

/*
static aos_pcm_t *playback_handle;
int CVI_Media_Audio_BootSound(void)
{
    CVI_GPIO_Set_Value(CVI_GPIOA_15, 1);
    aos_pcm_hw_params_t *playback_hw_params;
    uint32_t u32DataLen = (sizeof(g_bootsound)/sizeof(g_bootsound[0]));
    unsigned char *pBuffer = g_bootsound;
    int audiolen = (u32DataLen / 1280);
    int g_audiolen = (u32DataLen % 1280);
    unsigned int rate = 16000;
    int dir = 0;

    aos_pcm_open (&playback_handle, "pcmP0", AOS_PCM_STREAM_PLAYBACK, 0);
	aos_pcm_hw_params_alloca(&playback_hw_params);
	aos_pcm_hw_params_any(playback_handle, playback_hw_params);
	playback_hw_params->period_size = 1280;
	playback_hw_params->buffer_size = 1280*4;
	aos_pcm_hw_params_set_access(playback_handle, playback_hw_params, AOS_PCM_ACCESS_RW_INTERLEAVED);
	aos_pcm_hw_params_set_format(playback_handle, playback_hw_params, 16);
	aos_pcm_hw_params_set_rate_near(playback_handle, playback_hw_params, &rate, &dir);
	aos_pcm_hw_params_set_channels(playback_handle, playback_hw_params, 1);
	aos_pcm_hw_params(playback_handle, playback_hw_params);
    for (int i = 0; i < audiolen; i++) {
        aos_pcm_writei(playback_handle, pBuffer, aos_pcm_bytes_to_frames(playback_handle, 1280));
        pBuffer += 1280;
    }

    if (g_audiolen > 0) {
        aos_pcm_writei(playback_handle, pBuffer, aos_pcm_bytes_to_frames(playback_handle, g_audiolen));
    }

	aos_pcm_close(playback_handle);
	playback_handle = NULL;
    CVI_GPIO_Set_Value(CVI_GPIOA_15, 0);

    return 0;
}
*/

int _vo_init(void)
{
    VO_PUB_ATTR_S stVoPubAttr = {0};
    VO_LAYER VoLayer = 0;
    VO_VIDEO_LAYER_ATTR_S LayerAttr = {0};
    VO_CHN_ATTR_S VOChnAttr = {0};
    CVI_HAL_SCREEN_ATTR_S screenAttr = {0};
    CVI_HAL_SCREEN_GetAttr(CVI_HAL_SCREEN_IDX_0, &screenAttr);
    switch (screenAttr.enType) {
        case CVI_HAL_SCREEN_INTF_TYPE_MIPI:
            stVoPubAttr.enIntfType = VO_INTF_MIPI;
            break;
        case CVI_HAL_SCREEN_INTF_TYPE_LCD:
            break;
        case CVI_HAL_SCREEN_INTF_TYPE_I80:
            stVoPubAttr.enIntfType = VO_INTF_I80_HW;
            break;
        default:
            printf("Invalid screen type\n");
            return -1;
    }
    stVoPubAttr.u32BgColor = COLOR_10_RGB_WHITE;
    stVoPubAttr.enIntfSync = VO_OUTPUT_USER;
    stVoPubAttr.stSyncInfo.u16Hact = screenAttr.stAttr.stSynAttr.u16Hact;
    stVoPubAttr.stSyncInfo.u16Hpw = screenAttr.stAttr.stSynAttr.u16Hpw;
    stVoPubAttr.stSyncInfo.u16Hfb = screenAttr.stAttr.stSynAttr.u16Hfb;
    stVoPubAttr.stSyncInfo.u16Hbb = screenAttr.stAttr.stSynAttr.u16Hbb;
    stVoPubAttr.stSyncInfo.u16Vact = screenAttr.stAttr.stSynAttr.u16Vact;
    stVoPubAttr.stSyncInfo.u16Vpw = screenAttr.stAttr.stSynAttr.u16Vpw;
    stVoPubAttr.stSyncInfo.u16Vfb = screenAttr.stAttr.stSynAttr.u16Vfb;
    stVoPubAttr.stSyncInfo.u16Vbb = screenAttr.stAttr.stSynAttr.u16Vbb;
    stVoPubAttr.stSyncInfo.u16FrameRate = screenAttr.stAttr.u32Framerate;
    stVoPubAttr.stSyncInfo.bIop = 1;
    stVoPubAttr.stSyncInfo.bIhs = screenAttr.stAttr.stSynAttr.bIhs;
    stVoPubAttr.stSyncInfo.bIvs = screenAttr.stAttr.stSynAttr.bIvs;

    LayerAttr.stDispRect.s32X = 0;
    LayerAttr.stDispRect.s32Y = 0;
    LayerAttr.stDispRect.u32Width = screenAttr.stAttr.u32Width;
    LayerAttr.stDispRect.u32Height = screenAttr.stAttr.u32Height;
    LayerAttr.stImageSize.u32Width = screenAttr.stAttr.u32Width;
    LayerAttr.stImageSize.u32Height = screenAttr.stAttr.u32Height;
    LayerAttr.u32DispFrmRt = screenAttr.stAttr.u32Framerate;
    LayerAttr.enPixFormat = PIXEL_FORMAT_NV21;

    VOChnAttr.stRect.s32X = 0;
    VOChnAttr.stRect.s32Y = 0;
    VOChnAttr.stRect.u32Width = screenAttr.stAttr.u32Width;
    VOChnAttr.stRect.u32Height = screenAttr.stAttr.u32Height;
    // vdec_attr_logo.u32Width = screenAttr.stAttr.u32Width;
    // vdec_attr_logo.u32Height = screenAttr.stAttr.u32Height;

    CVI_VO_SetPubAttr(0, &stVoPubAttr);
    printf("set PubAttr finsh!!\n");
    CVI_VO_Enable(0);
    CVI_VO_SetDisplayBufLen(0, 3);
    CVI_VO_SetVideoLayerAttr(VoLayer, &LayerAttr);
    CVI_VO_EnableVideoLayer(VoLayer);
    CVI_VO_SetChnAttr(0, 0, &VOChnAttr);
    //CVI_VO_SetChnRotation(0, 0, ROTATION_90);
    //TODO:
    // CVI_VO_SetChnRotation(0, 0, pstVoCtx->pstVoCfg[i].u8ChnRotation);
    CVI_VO_EnableChn(0, 0);


    return CVI_SUCCESS;
}

static void *_MEDIA_VIDEO_PanelInit()
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    s32Ret = CVI_HAL_SCREEN_Register(CVI_HAL_SCREEN_IDX_0, &stHALSCREENObj);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_HAL_SCREEN_Register failed! \n");
    }

    if (get_paramcfg() != NULL) {
        printf("g_stParamCtx.pstCfg->MagicStart %x\n", get_paramcfg()->MagicStart);
        printf("g_stParamCtx.pstCfg->MagicEnd %x\n", get_paramcfg()->MagicEnd);
    }

    s32Ret = CVI_HAL_SCREEN_Init(CVI_HAL_SCREEN_IDX_0);
    if (s32Ret != CVI_SUCCESS) {
        printf("CVI_HAL_SCREEN_Init failed! \n");
    }

    printf("Init for MIPI-Driver-%s\n", "screen");
    _vo_init();

    usleep(80 * 1000);

    if(get_paramcfg() != NULL) {
        CVI_HAL_SCREEN_SetLuma(CVI_HAL_SCREEN_IDX_0, pstCfg->DevMng.PWM.PWMCfg);
    }

    CVI_HAL_SCREEN_SetBackLightState(CVI_HAL_SCREEN_IDX_0, CVI_HAL_SCREEN_STATE_ON);

    pthread_exit(NULL);
    return NULL;
}

int CVI_Media_PanelInit(void)
{
    _MEDIA_VIDEO_PanelInit();
    return 0;
}

static CVI_S32 _vdec_init(VDEC_ATTR vdecAttr)
{
    VDEC_CHN_ATTR_S stAttr = {0};
    VDEC_CHN_PARAM_S stParam = {0};
    VDEC_MOD_PARAM_S stModParam = {0};

    CVI_VDEC_GetModParam(&stModParam);
    stModParam.enVdecVBSource = VB_SOURCE_COMMON;
    CVI_VDEC_SetModParam(&stModParam);

    stAttr.enType = vdecAttr.enType;
    stAttr.enMode = vdecAttr.enMode;
    stAttr.u32PicWidth = vdecAttr.u32Width;
    stAttr.u32PicHeight = vdecAttr.u32Height;
    stAttr.u32StreamBufSize = ALIGN(vdecAttr.u32Width * vdecAttr.u32Height, 0x4000);
    stAttr.u32FrameBufSize = VDEC_GetPicBufferSize(
                vdecAttr.enType, vdecAttr.u32Width, vdecAttr.u32Height,
                vdecAttr.enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
    stAttr.u32FrameBufCnt =vdecAttr.u32FrameBufCnt;
    CVI_VDEC_CreateChn(vdecAttr.u32VdecChn, &stAttr);

    CVI_VDEC_GetChnAttr(vdecAttr.u32VdecChn, &stAttr);
    CVI_VDEC_SetChnAttr(vdecAttr.u32VdecChn, &stAttr);

    CVI_VDEC_GetChnParam(vdecAttr.u32VdecChn, &stParam);
    stParam.enPixelFormat = vdecAttr.enPixelFormat;
    CVI_VDEC_SetChnParam(vdecAttr.u32VdecChn, &stParam);

    CVI_VDEC_StartRecvStream(vdecAttr.u32VdecChn);

    return CVI_SUCCESS;
}

static CVI_S32 _vdec_deinit(VDEC_ATTR vdecAttr)
{
    CVI_VDEC_StopRecvStream(vdecAttr.u32VdecChn);
    CVI_VDEC_ResetChn(vdecAttr.u32VdecChn);
    CVI_VDEC_DestroyChn(vdecAttr.u32VdecChn);

    return CVI_SUCCESS;
}

int CVI_Media_Vdec_Logo(void)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VDEC_STREAM_S pstStream = {0};
    VIDEO_FRAME_INFO_S pstFrameInfo = {0};
    CVI_U8 * pu8Buf = (CVI_U8 *)CVIMMAP_BOOTLOGO_ADDR;
    CVI_BOOL bFindStart = 0;
    CVI_U32 u32Len = 0, u32Start = 0;
    CVI_U32 s32ReadLen = 0;
    int i = 0;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr = {0};

    s32Ret = CVI_VB_Init();
    if (s32Ret != CVI_SUCCESS) {
        printf("vb init error");
        return s32Ret;
    }

    // Dynamically parse atom data in mjpeg files and calculate video width and height
    for (CVI_S32 i = 0; i < READ_LEN_MAX - 1; i++) {
        if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xC0 && pu8Buf[i + 2] == 0x00 && // SOFO
            pu8Buf[i + 3] == 0x11&& pu8Buf[i + 4] == 0x08) {
            vdec_attr_logo.u32Height = pu8Buf[i + 5] * 256 + pu8Buf[i + 6];
            vdec_attr_logo.u32Width = pu8Buf[i + 7] * 256 + pu8Buf[i + 8];
            break;
        }
    }

    while(1){
        s32Ret = _vdec_init(vdec_attr_logo);
        if (s32Ret != CVI_SUCCESS) {
            printf("vdec init error");
            CVI_VB_Exit();
            return s32Ret;
        }

        bFindStart = CVI_FALSE;
        for (; i < READ_LEN_MAX - 1; i++) {
            if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD8) {
                u32Start = i;
                bFindStart = CVI_TRUE;
                i = i + 2;
                break;
            }
        }

        for (; i < READ_LEN_MAX - 3; i++) {
            if ((pu8Buf[i] == 0xFF) && (pu8Buf[i + 1] & 0xF0) == 0xE0) {
                u32Len = (pu8Buf[i + 2] << 8) + pu8Buf[i + 3];
                i += 1 + u32Len;
            } else {
                break;
            }
        }

        for (; i < READ_LEN_MAX - 1; i++) {
            if (pu8Buf[i] == 0xFF && pu8Buf[i + 1] == 0xD9) {
                break;
            }
        }
        s32ReadLen = i + 3 - u32Start;
        i += 2;

        if (bFindStart == CVI_FALSE) {
            printf("can not find JPEG start code!!\n");
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return CVI_FAILURE;
        }

        printf("file u32Start is %d, s32ReadLen is %d\n", u32Start, s32ReadLen);

        pstStream.pu8Addr = pu8Buf + u32Start;
        pstStream.u32Len = s32ReadLen;
        pstStream.bEndOfFrame = 1;
        pstStream.bEndOfStream = 1;
        s32Ret = CVI_VDEC_SendStream(0, &pstStream, 1000);
        if (s32Ret != CVI_SUCCESS) {
            printf("send JPEG stream error");
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return s32Ret;
        }

    // RETRY_GET_FRAME:
        s32Ret = CVI_VDEC_GetFrame(0, &pstFrameInfo, 1000);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VDEC_GetFrame error, s32Ret = %d\n", s32Ret);
            // goto RETRY_GET_FRAME;
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return s32Ret;
        }

        s32Ret = CVI_VO_GetVideoLayerAttr(0, &stLayerAttr);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VO_GetVideoLayerAttr error\n");
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return s32Ret;
        }

        if (pstFrameInfo.stVFrame.u32Width > stLayerAttr.stImageSize.u32Width) {
            stLayerAttr.stDispRect.s32X = 0;
        } else {
            stLayerAttr.stDispRect.s32X = (stLayerAttr.stImageSize.u32Width - pstFrameInfo.stVFrame.u32Width) / 2;
        }
        if (pstFrameInfo.stVFrame.u32Width > stLayerAttr.stImageSize.u32Width) {
            stLayerAttr.stDispRect.s32Y = 0;
        } else {
            stLayerAttr.stDispRect.s32Y = (stLayerAttr.stImageSize.u32Height - pstFrameInfo.stVFrame.u32Height) / 2;
        }
        stLayerAttr.stDispRect.u32Width = pstFrameInfo.stVFrame.u32Width;
        stLayerAttr.stDispRect.u32Height = pstFrameInfo.stVFrame.u32Height;

        s32Ret = CVI_VO_SetVideoLayerAttrEx(0, &stLayerAttr);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_VO_SetVideoLayerAttrEx error\n");
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return s32Ret;
        }

        usleep(80*1000);
        s32Ret = CVI_VO_SendLogoFromIon(0, 0, &pstFrameInfo, 1000);
        if (s32Ret != CVI_SUCCESS) {
            //goto RETRY_GET_FRAME;
            printf("frame send vo error\n");
            _vdec_deinit(vdec_attr_logo);
            CVI_VB_Exit();
            return s32Ret;
        }
        CVI_VDEC_ReleaseFrame(0, &pstFrameInfo);
        _vdec_deinit(vdec_attr_logo);
    }

    CVI_VB_Exit();

    return 0;
}
