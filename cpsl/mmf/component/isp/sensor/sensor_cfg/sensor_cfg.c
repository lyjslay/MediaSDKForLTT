#include "sensor_cfg.h"

typedef struct _SNS_CONFIG_S {
    SIZE_S            stSize;
    WDR_MODE_E        enWdrMode;
    BAYER_FORMAT_E        enBayerFormat;
    PIXEL_FORMAT_E        enPixelFormat;
    SENSOR_TYPE_E        enSnsType;
} SNS_CONFIG_S;

ISP_PUB_ATTR_S isp_pub_attr_base = {
    .stWndRect = {0, 0, 1920, 1080},
    .stSnsSize = {1920, 1080},
    .f32FrameRate = 25.0f,
    .enBayer = BAYER_BGGR,
    .enWDRMode = WDR_MODE_NONE,
    .u8SnsMode = 0,
};

VI_DEV_ATTR_S vi_dev_attr_base = {
    .enIntfMode = VI_MODE_MIPI,
    .enWorkMode = VI_WORK_MODE_1Multiplex,
    .enScanMode = VI_SCAN_PROGRESSIVE,
    .as32AdChnId = {-1, -1, -1, -1},
    .enDataSeq = VI_DATA_SEQ_YUYV,
    .stSynCfg = {
    /*port_vsync    port_vsync_neg      port_hsync              port_hsync_neg*/
    VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH,
    /*port_vsync_valid       port_vsync_valid_neg*/
    VI_VSYNC_VALID_SIGNAL, VI_VSYNC_VALID_NEG_HIGH,

    /*hsync_hfb  hsync_act    hsync_hhb*/
    {0,           1280,       0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,           720,           0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,          0}
    },
    .enInputDataType = VI_DATA_TYPE_RGB,
    .stSize = {1280, 720},
    .stWDRAttr = {WDR_MODE_NONE, 720},
    .enBayerFormat = BAYER_FORMAT_BG,
};

VI_PIPE_ATTR_S vi_pipe_attr_base = {
    .enPipeBypassMode = VI_PIPE_BYPASS_NONE,
    .bYuvSkip = CVI_FALSE,
    .bIspBypass = CVI_FALSE,
    .u32MaxW = 1280,
    .u32MaxH = 720,
    .enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP,
    .enCompressMode = COMPRESS_MODE_TILE,
    .enBitWidth = DATA_BITWIDTH_12,
    .bNrEn = CVI_FALSE,
    .bSharpenEn = CVI_FALSE,
    .stFrameRate = {-1, -1},
    .bDiscardProPic = CVI_FALSE,
    .bYuvBypassPath = CVI_FALSE,
};


VI_CHN_ATTR_S vi_chn_attr_base = {
    .stSize = {1280, 720},
    .enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420,
    .enDynamicRange = DYNAMIC_RANGE_SDR8,
    .enVideoFormat = VIDEO_FORMAT_LINEAR,
    .enCompressMode = COMPRESS_MODE_TILE,
    .bMirror = CVI_FALSE,
    .bFlip = CVI_FALSE,
    .u32Depth = 0,
    .stFrameRate = {-1, -1},
};

const char *snsr_type_name[SENSOR_TYPE_BUTT] = {
	"SNS_TYPE_NONE",
	"SONY_IMX327_MIPI_2M_30FPS_12BIT",
	"SONY_IMX335_MIPI_5M_30FPS_12BIT",
	"SONY_IMX662_MIPI_2M_30FPS_12BIT",
	"SONY_IMX307_MIPI_2M_30FPS_12BIT",
	"SONY_IMX307_2L_MIPI_2M_30FPS_12BIT",
	"SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT",
	"GCORE_GC1054_MIPI_1M_30FPS_10BIT",
	"GCORE_GC1084_MIPI_1M_30FPS_10BIT",
	"GCORE_GC2053_MIPI_2M_30FPS_10BIT",
	"GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT",
	"GCORE_GC2083_MIPI_2M_30FPS_10BIT",
	"GCORE_GC2093_MIPI_2M_30FPS_10BIT",
	"GCORE_GC02M1_MIPI_2M_30FPS_10BIT",
	"GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT",
	"GCORE_GC4653_MIPI_4M_30FPS_10BIT",
	"PIXELPLUS_PR2020_2M_25FPS_8BIT",
	"TECHPOINT_TP9950_2M_25FPS_8BIT",
	"TECHPOINT_TP9963_2M_25FPS_8BIT",
	"TECHPOINT_TP9963_2M_2CH_25FPS_8BIT",
	"TECHPOINT_TP9963_2M_30FPS_8BIT",
	"TECHPOINT_TP9963_2M_2CH_30FPS_8BIT",
	"TECHPOINT_TP9963_1M_25FPS_8BIT",
	"TECHPOINT_TP9963_1M_2CH_25FPS_8BIT",
	"TECHPOINT_TP9963_1M_30FPS_8BIT",
	"TECHPOINT_TP9963_1M_2CH_30FPS_8BIT",
	"CISTA_C4390_MIPI_4M_30FPS_10BIT",
	"GCORE_GC4023_MIPI_4M_30FPS_10BIT",
	"CISTA_C2395_MIPI_2M_30FPS_10BIT",
	"CISTA_C4395_MIPI_4M_30FPS_10BIT",
	"NEXTCHIP_N5_2M_25FPS_8BIT",
	"PIXELPLUS_XS9950_2M_25FPS_8BIT",
	"PIXELPLUS_XS9951_2M_25FPS_8BIT",
	"OV_OS5648_4M_15FPS_8BIT",
};

static CVI_S32 g_snstype[] = {SNS0_TYPE, SNS1_TYPE, SNS2_TYPE};

CVI_S32 get_sensor_type(CVI_S32 dev_id)
{
	return g_snstype[dev_id];
}

#if defined(CONFIG_KERNEL_RHINO)
ISP_SNS_OBJ_S *getSnsObj(SENSOR_TYPE_E enSnsType)
{
	switch (enSnsType) {
#if GCORE_GC02M1
	case GCORE_GC02M1_MIPI_2M_30FPS_10BIT:
		return &stSnsGc02m1_Obj;
#endif
#if GCORE_GC02M1_SLAVE
	case GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT:
		return &stSnsGc02m1_Slave_Obj;
#endif
#if GCORE_GC1054
	case GCORE_GC1054_MIPI_1M_30FPS_10BIT:
		return &stSnsGc1054_Obj;
#endif
#if GCORE_GC2053
	case GCORE_GC2053_MIPI_2M_30FPS_10BIT:
		return &stSnsGc2053_Obj;
#endif
#if GCORE_GC2053_1L
	case GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT:
		return &stSnsGc2053_1l_Obj;
#endif
#if GCORE_GC2083
	case GCORE_GC2083_MIPI_2M_30FPS_10BIT:
		return &stSnsGc2083_Obj;
#endif
#if GCORE_GC2093
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT:
		return &stSnsGc2093_Obj;
#endif
#if GCORE_GC4653
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
		return &stSnsGc4653_Obj;
#endif
#if PIXELPLUS_PR2020
	case PIXELPLUS_PR2020_2M_25FPS_8BIT:
		return &stSnsPR2020_Obj;
#endif
#if SONY_IMX307
	case SONY_IMX307_MIPI_2M_30FPS_12BIT:
		return &stSnsImx307_Obj;
#endif
#if SONY_IMX307_2L
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
		return &stSnsImx307_2l_Obj;
#endif
#if SONY_IMX307_SLAVE
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT:
		return &stSnsImx307_Slave_Obj;
#endif
#if SONY_IMX327
	case SONY_IMX327_MIPI_2M_30FPS_12BIT:
		return &stSnsImx327_Obj;
#endif
#if SONY_IMX335
	case SONY_IMX335_MIPI_5M_30FPS_12BIT:
		return &stSnsImx335_Obj;
#endif
#if SONY_IMX662
	case SONY_IMX662_MIPI_2M_30FPS_12BIT:
		return &stSnsImx662_Obj;
#endif
#if TECHPOINT_TP9950
	case TECHPOINT_TP9950_2M_25FPS_8BIT:
		return &stSnsTP9950_Obj;
#endif
#if CISTA_C4390
	case CISTA_C4390_MIPI_4M_30FPS_10BIT:
		return &stSnsC4390_Obj;
#endif
#if GCORE_GC4023
	case GCORE_GC4023_MIPI_4M_30FPS_10BIT:
		return &stSnsGc4023_Obj;
#endif
#if CISTA_C2395
	case CISTA_C2395_MIPI_2M_30FPS_10BIT:
		return &stSnsC2395_Obj;
#endif
#if CISTA_C4395
	case CISTA_C4395_MIPI_4M_30FPS_10BIT:
		return &stSnsC4395_Obj;
#endif
#if GCORE_GC1084
	case GCORE_GC1084_MIPI_1M_30FPS_10BIT:
		return &stSnsGc1084_Obj;
#endif
#if TECHPOINT_TP9963

	case TECHPOINT_TP9963_2M_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_30FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_30FPS_8BIT:
		return &stSnsTP9963_Obj;
#endif
#if NEXTCHIP_N5
	case NEXTCHIP_N5_2M_25FPS_8BIT:
		return &stSnsN5_Obj;
#endif
#if PIXELPLUS_XS9950
	case PIXELPLUS_XS9950_2M_25FPS_8BIT:
		return &stSnsXS9950_Obj;
#endif
#if PIXELPLUS_XS9951
	case PIXELPLUS_XS9951_2M_25FPS_8BIT:
		return &stSnsXS9951_Obj;
#endif
#if OV_OS5648
	case OV_OS5648_4M_15FPS_8BIT:
		return &stSnsOV5648_Obj;
#endif
	default:
		return CVI_NULL;
	}
}

SNS_AHD_OBJ_S *getAhdObj(CVI_S32 snsr_type)
{
	switch (snsr_type) {
#if PIXELPLUS_PR2020
	case PIXELPLUS_PR2020_2M_25FPS_8BIT:
		return &stAhdPr2020Obj;
#endif
#if TECHPOINT_TP9950
	case TECHPOINT_TP9950_2M_25FPS_8BIT:
		return &stAhdTp9950Obj;
#endif
#if TECHPOINT_TP9963
	case TECHPOINT_TP9963_2M_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_30FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_30FPS_8BIT:
		return &stAhdTp9963Obj;
#endif
#if NEXTCHIP_N5
	case NEXTCHIP_N5_2M_25FPS_8BIT:
		return &stAhdN5Obj;
#endif
#if PIXELPLUS_XS9950
	case PIXELPLUS_XS9950_2M_25FPS_8BIT:
		return &stAhdXs9950Obj;
#endif
#if PIXELPLUS_XS9951
	case PIXELPLUS_XS9951_2M_25FPS_8BIT:
		return &stAhdXs9951Obj;
#endif
	default:
		return CVI_NULL;
	}
}
#endif

CVI_S32 getDevAttr(SENSOR_TYPE_E enSnsType, VI_DEV_ATTR_S *pstViDevAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 enWdrMode = pstViDevAttr->stWDRAttr.enWDRMode;

	memcpy(pstViDevAttr, &vi_dev_attr_base, sizeof(VI_DEV_ATTR_S));

	switch (enSnsType) {
	case SONY_IMX327_MIPI_2M_30FPS_12BIT:
	case SONY_IMX335_MIPI_5M_30FPS_12BIT:
	case SONY_IMX662_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT:
	case GCORE_GC02M1_MIPI_2M_30FPS_10BIT:
	case GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2053_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT:
	case GCORE_GC1054_MIPI_1M_30FPS_10BIT:
	case GCORE_GC4023_MIPI_4M_30FPS_10BIT:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_RG;
		break;
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
	case GCORE_GC1084_MIPI_1M_30FPS_10BIT:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_GR;
		break;
	case CISTA_C4390_MIPI_4M_30FPS_10BIT:
	case CISTA_C4395_MIPI_4M_30FPS_10BIT:
	case OV_OS5648_4M_15FPS_8BIT:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_BG;
		break;
	// YUV Sensor
	case PIXELPLUS_PR2020_2M_25FPS_8BIT:
	case TECHPOINT_TP9950_2M_25FPS_8BIT:
	case NEXTCHIP_N5_2M_25FPS_8BIT:
		pstViDevAttr->enDataSeq = VI_DATA_SEQ_YUYV;
		pstViDevAttr->enInputDataType = VI_DATA_TYPE_YUV;
		pstViDevAttr->enIntfMode = VI_MODE_BT656;
		break;
	case TECHPOINT_TP9963_2M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_30FPS_8BIT:
		pstViDevAttr->enWorkMode = VI_WORK_MODE_2Multiplex;
		pstViDevAttr->enDataSeq = VI_DATA_SEQ_YUYV;
		pstViDevAttr->enInputDataType = VI_DATA_TYPE_YUV;
		pstViDevAttr->enIntfMode = VI_MODE_MIPI_YUV422;
		break;
	case TECHPOINT_TP9963_2M_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_30FPS_8BIT:
		pstViDevAttr->enDataSeq = VI_DATA_SEQ_YUYV;
		pstViDevAttr->enInputDataType = VI_DATA_TYPE_YUV;
		pstViDevAttr->enIntfMode = VI_MODE_BT656;
		break;
	case PIXELPLUS_XS9950_2M_25FPS_8BIT:
	case PIXELPLUS_XS9951_2M_25FPS_8BIT:
		pstViDevAttr->enDataSeq = VI_DATA_SEQ_VYUY;
		pstViDevAttr->enInputDataType = VI_DATA_TYPE_YUV;
		pstViDevAttr->enIntfMode = VI_MODE_BT656;
		break;
	case GCORE_GC2083_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_RG;
		if (enWdrMode == WDR_MODE_2To1_LINE) {
			// pstViDevAttr->enVcWdrMode = 1;
		}
		break;
	case CISTA_C2395_MIPI_2M_30FPS_10BIT:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_BG;
		if (enWdrMode == WDR_MODE_2To1_LINE) {
			// pstViDevAttr->enVcWdrMode = 1;
		}
		break;
	default:
		printf("get sensor %d attr failed!\n", enSnsType);
		s32Ret = CVI_FAILURE;
		break;
	};

	pstViDevAttr->stWDRAttr.enWDRMode = enWdrMode;

	return s32Ret;
}

CVI_S32 getPipeAttr(SENSOR_TYPE_E enSnsType, VI_PIPE_ATTR_S *pstViPipeAttr)
{
	memcpy(pstViPipeAttr, &vi_pipe_attr_base, sizeof(VI_PIPE_ATTR_S));

	switch (enSnsType) {
	case PIXELPLUS_PR2020_2M_25FPS_8BIT:
	case TECHPOINT_TP9950_2M_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_30FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_30FPS_8BIT:
	case NEXTCHIP_N5_2M_25FPS_8BIT:
	case PIXELPLUS_XS9950_2M_25FPS_8BIT:
	case PIXELPLUS_XS9951_2M_25FPS_8BIT:
		pstViPipeAttr->bYuvBypassPath = CVI_TRUE;
		break;
	default:
		pstViPipeAttr->bYuvBypassPath = CVI_FALSE;
		break;
	}

	return CVI_SUCCESS;
}

CVI_S32 getChnAttr(SENSOR_TYPE_E enSnsType, VI_CHN_ATTR_S *pstViChnAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	memcpy(pstViChnAttr, &vi_chn_attr_base, sizeof(VI_CHN_ATTR_S));

	switch (enSnsType) {
	case SONY_IMX327_MIPI_2M_30FPS_12BIT:
	case SONY_IMX335_MIPI_5M_30FPS_12BIT:
	case SONY_IMX662_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT:
	case GCORE_GC02M1_MIPI_2M_30FPS_10BIT:
	case GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2053_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT:
	case GCORE_GC1054_MIPI_1M_30FPS_10BIT:
	case GCORE_GC2083_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT:
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
	case GCORE_GC4023_MIPI_4M_30FPS_10BIT:
	case CISTA_C4390_MIPI_4M_30FPS_10BIT:
	case CISTA_C4395_MIPI_4M_30FPS_10BIT:
	case CISTA_C2395_MIPI_2M_30FPS_10BIT:
	case GCORE_GC1084_MIPI_1M_30FPS_10BIT:
	case OV_OS5648_4M_15FPS_8BIT:
		pstViChnAttr->enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	// YUV Sensor
	case PIXELPLUS_PR2020_2M_25FPS_8BIT:
	case NEXTCHIP_N5_2M_25FPS_8BIT:
		pstViChnAttr->enPixelFormat = PIXEL_FORMAT_NV21;
		break;
	case PIXELPLUS_XS9950_2M_25FPS_8BIT:
	case PIXELPLUS_XS9951_2M_25FPS_8BIT:
		pstViChnAttr->enPixelFormat = PIXEL_FORMAT_VYUY;
		break;
	case TECHPOINT_TP9950_2M_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_30FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_30FPS_8BIT:
		pstViChnAttr->enPixelFormat = PIXEL_FORMAT_UYVY;
		break;
	default:
		printf("get chn %d attr failed!\n", enSnsType);
		s32Ret = CVI_FAILURE;
		break;
	};

	return s32Ret;
}

CVI_S32 getIspPubAttr(SENSOR_TYPE_E enSnsType, ISP_PUB_ATTR_S *pstIspPubAttr)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	memcpy(pstIspPubAttr, &isp_pub_attr_base, sizeof(ISP_PUB_ATTR_S));

	switch (enSnsType) {
	case GCORE_GC1054_MIPI_1M_30FPS_10BIT:
	case GCORE_GC2053_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2083_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT:
	case GCORE_GC4023_MIPI_4M_30FPS_10BIT:
	case SONY_IMX335_MIPI_5M_30FPS_12BIT:
	case SONY_IMX327_MIPI_2M_30FPS_12BIT:
		pstIspPubAttr->enBayer = BAYER_RGGB;
		break;
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
	case GCORE_GC1084_MIPI_1M_30FPS_10BIT:
		pstIspPubAttr->enBayer = BAYER_GRBG;
		break;
	case PIXELPLUS_PR2020_2M_25FPS_8BIT:
	case TECHPOINT_TP9950_2M_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_2M_30FPS_8BIT:
	case TECHPOINT_TP9963_2M_2CH_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_25FPS_8BIT:
	case TECHPOINT_TP9963_1M_30FPS_8BIT:
	case TECHPOINT_TP9963_1M_2CH_30FPS_8BIT:
	case PIXELPLUS_XS9950_2M_25FPS_8BIT:
	case PIXELPLUS_XS9951_2M_25FPS_8BIT:
	case CISTA_C4390_MIPI_4M_30FPS_10BIT:
	case CISTA_C4395_MIPI_4M_30FPS_10BIT:
	case CISTA_C2395_MIPI_2M_30FPS_10BIT:
	case NEXTCHIP_N5_2M_25FPS_8BIT:
	case OV_OS5648_4M_15FPS_8BIT:
		pstIspPubAttr->enBayer = BAYER_BGGR;
		break;
	default:
		s32Ret = CVI_FAILURE;
		break;
	}
	return s32Ret;
}

CVI_S32 getSnsTypeByName(SNS_TYPE_S *SnsInfo)
{
	CVI_U32 i;

	if (SnsInfo == CVI_NULL) {
		printf("SnsInfo is NULL!!\n");
		return CVI_FAILURE;
	}

	printf("sensor =  %s\n", SnsInfo->SnsName);

	for (i = 0; i < SENSOR_TYPE_BUTT; i++) {
		if (strcmp(SnsInfo->SnsName, snsr_type_name[i]) == 0) {
			SnsInfo->SnsType = i;
			break;
		}
	}
	if (i == SENSOR_TYPE_BUTT) {
		printf("ERROR: can not find sensor by Sensor Name\n");
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}
