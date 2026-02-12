#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>

#include "cvi_mapi.h"
#include "cvi_mapi_vcap_internal.h"
#include "cvi_osal.h"
#include "cvi_log.h"

#include "cvi_buffer.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_hal_gpio.h"
#include "cvi_bin.h"

#define CHECK_MAPI_VCAP_NULL_PTR_RET(ptr)                               \
    do {                                                                \
        if (ptr == NULL) {                                              \
            CVI_LOGE("%s is NULL pointer\n", #ptr);                     \
            return CVI_MAPI_ERR_INVALID;                                \
        }                                                               \
    } while (0)

#define CHECK_MAPI_VCAP_MAX_VAL_RET(paraname, value, max)                             \
    do {                                                                              \
        if (value > max) {                                                            \
            CVI_LOGE("%s:%d can not larger than max val %d\n", paraname, value, max); \
            return CVI_MAPI_ERR_INVALID;                                              \
        }                                                                             \
    } while (0)

#define CHECK_MAPI_VCAP_ZERO_VAL_RET(paraname, value)                   \
    do {                                                                \
        if (value == 0) {                                               \
            CVI_LOGE("%s is zero\n", paraname);                         \
            return CVI_MAPI_ERR_INVALID;                                \
        }                                                               \
    } while (0)

#define CHECK_MAPI_VCAP_RET(ret, fmt...)                                \
    do {                                                                \
        if (ret != CVI_SUCCESS) {                                       \
            CVI_LOGE(fmt);                                              \
            CVI_LOGE("fail and return:[%#x]\n", ret);                   \
            return ret;                                                 \
        }                                                               \
    } while (0)

typedef CVI_MAPI_HANDLE_T CVI_MAPI_VCAP_HANDLE_T;

typedef struct VCAP_DUMP_RAW_CTX_S {
    CVI_BOOL bStart;
    VI_PIPE ViPipe;
    CVI_U32 u32Count;
    CVI_MAPI_VCAP_RAW_DATA_T stCallbackFun;
    pthread_t pthreadDumpRaw;
} VCAP_DUMP_RAW_CTX_T;

typedef struct CVI_MAPI_VCAP_SENSOR_S {
    int sns_id;
    CVI_MAPI_VCAP_ATTR_T attr;
    CVI_MAPI_VCAP_HANDLE_T vcap_hdl;
    VCAP_DUMP_RAW_CTX_T stVcapDumpRawCtx;
} CVI_MAPI_VCAP_SENSOR_T;

typedef struct _MAPI_SNS_INFO_S {
    SENSOR_TYPE_E enSnsType;
    CVI_S32 s32SnsId;
    CVI_S32 s32BusId;
    CVI_S32 s32SnsI2cAddr;
    CVI_U32 MipiDev;
    CVI_S16 as16LaneId[5];
    CVI_S8  as8PNSwap[5];
    CVI_U8  u8HwSync;
    CVI_U8  u8CamClkId;
    CVI_U8  u8RstGpioInx;
    CVI_U8  u8RstGpioPin;
    CVI_U8  u8RstGpioPol;
} MAPI_SNS_INFO_S;

typedef struct _MAPI_CHN_INFO_S {
    CVI_S32 s32ChnId;
    CVI_U32 u32Width;
    CVI_U32 u32Height;
    CVI_FLOAT f32Fps;
    PIXEL_FORMAT_E enPixFormat;
    WDR_MODE_E enWDRMode;
    COMPRESS_MODE_E enCompressMode;
    CVI_BOOL fbmEnable;
    CVI_U32 vbcnt;
} MAPI_CHN_INFO_S;

typedef struct _MAPI_VI_INFO_S {
    MAPI_SNS_INFO_S stSnsInfo;
    MAPI_CHN_INFO_S stChnInfo;
} MAPI_VI_INFO_S;

typedef struct _MAPI_VI_CONFIG_S {
    MAPI_VI_INFO_S astViInfo[VI_MAX_DEV_NUM];
    CVI_S32 s32WorkingViNum;
} MAPI_VI_CONFIG_S;

typedef struct CVI_MAPI_VCAP_CTX_S {
    MAPI_VI_CONFIG_S ViConfig;
    int ref_count;
} CVI_MAPI_VCAP_CTX_T;

#define CVI_ISP_BYPASS_CNT  10

static CVI_MAPI_VCAP_HANDLE_T vcap_ctx = NULL;
static pthread_mutex_t vcap_mutex = PTHREAD_MUTEX_INITIALIZER;
SENSOR_TYPE_E enSnsType[VI_MAX_DEV_NUM] = {SENSOR_NONE, SENSOR_NONE};

#ifdef ENABLE_ISP_PQ_TOOL
#include "cvi_ispd2.h"
static CVI_BOOL g_ISPDaemon = CVI_FALSE;
#define ISPD_CONNECT_PORT 5566
#endif

static char PqBinName[WDR_MODE_MAX][BIN_FILE_LENGTH] = { "/mnt/system/bin/cvi_sdr_bin", "/mnt/system/bin/cvi_sdr_bin",
							   "/mnt/system/bin/cvi_sdr_bin", "/mnt/system/bin/cvi_wdr_bin",
							   "/mnt/system/bin/cvi_wdr_bin", "/mnt/system/bin/cvi_wdr_bin",
							   "/mnt/system/bin/cvi_wdr_bin", "/mnt/system/bin/cvi_wdr_bin",
							   "/mnt/system/bin/cvi_wdr_bin", "/mnt/system/bin/cvi_wdr_bin",
							   "/mnt/system/bin/cvi_wdr_bin", "/mnt/system/bin/cvi_wdr_bin" };

static enum CVI_BIN_SECTION_ID s_BinId[4] = {CVI_BIN_ID_ISP0, CVI_BIN_ID_ISP1, CVI_BIN_ID_ISP2, CVI_BIN_ID_ISP3};

static CVI_S32 _getFileSize(FILE *fp, CVI_U32 *size)
{
	CVI_S32 ret = CVI_SUCCESS;

	fseek(fp, 0L, SEEK_END);
	*size = ftell(fp);
	rewind(fp);

	return ret;
}

CVI_S32 CVI_MAPI_VCAP_BIN_ReadParaFrombin(enum CVI_BIN_SECTION_ID id)
{
	CVI_S32 ret = CVI_SUCCESS;
	FILE *fp = NULL;
	CVI_U8 *buf = NULL;
	CVI_CHAR binName[BIN_FILE_LENGTH] = {0};
	CVI_U32 u32file_size = 0;

	ret = CVI_BIN_GetBinName(binName);
	if (ret != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_WARN, "GetBinName(%s) fail\n", binName);
	}

	fp = fopen((const CVI_CHAR *)binName, "rb");
	if (fp == NULL) {
		if (id == CVI_BIN_ID_VPSS) {
			CVI_TRACE_SYS(CVI_DBG_WARN, "Can't find bin(%s)\n", binName);
		} else if (id >= CVI_BIN_ID_ISP0 && id <= CVI_BIN_ID_ISP3) {
			CVI_TRACE_SYS(CVI_DBG_WARN, "Can't find bin(%s), use default parameters\n", binName);
		} else {
			CVI_TRACE_SYS(CVI_DBG_WARN, "Can't find bin(%s)\n", binName);
		}
		ret = CVI_FAILURE;
		goto ERROR_HANDLER;
	} else {
		CVI_TRACE_SYS(CVI_DBG_WARN, "Bin exist (%s)\n", binName);
	}
	_getFileSize(fp, &u32file_size);

	buf = (CVI_U8 *)malloc(u32file_size);
	if (buf == NULL) {
		ret = CVI_FAILURE;
		CVI_TRACE_SYS(CVI_DBG_WARN, "Allocate memory fail\n");
		goto ERROR_HANDLER;
	}
	fread(buf, u32file_size, 1, fp);

	if (id >= CVI_BIN_ID_ISP0 && id <= CVI_BIN_ID_ISP3) {
		ret = CVI_BIN_LoadParamFromBin(CVI_BIN_ID_HEADER, buf);
		if (ret != CVI_SUCCESS) {
			CVI_TRACE_SYS(CVI_DBG_WARN, "Bin Version not match, use default parameters\n");
			goto ERROR_HANDLER;
		}
	}
	ret = CVI_BIN_LoadParamFromBin(id, buf);
    if (id == CVI_BIN_ID_ISP0) {
        ret = CVI_BIN_LoadParamFromBin(CVI_BIN_ID_VPSS, buf);
        ret = CVI_BIN_LoadParamFromBin(CVI_BIN_ID_VO, buf);
    }
ERROR_HANDLER:
	if (fp != NULL) {
		fclose(fp);
	}
	if (buf != NULL) {
		free(buf);
	}

	return ret;
}

int CVI_MAPI_VCAP_SetAhdMode(int sns_id, int mode)
{
    return CVI_SENSOR_SetAHDMode(sns_id, mode);
}

int CVI_MAPI_VCAP_GetAhdMode(int sns_id, int *mode, int *status)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    enSnsType[sns_id] = get_sensor_type(sns_id);
    if (enSnsType[sns_id] == SENSOR_NONE) {
        CVI_LOGE("sensor[%d] dev type error\n", sns_id);
        return CVI_MAPI_ERR_INVALID;
    }

    s32Ret = CVI_SENSOR_SetSnsType(sns_id, enSnsType[sns_id]);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_SENSOR_SetSnsType(%d) failed!\n", sns_id);

    SNS_STATUS_MSG_S sstatus = {
        .s32SnsId = sns_id,
        .s32Status = 0,
        .eMode = AHD_MODE_NONE
    };
    s32Ret = CVI_SENSOR_GetAhdStatus(&sstatus);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_SENSOR_GetAhdStatus failed!\n");
        return CVI_MAPI_ERR_INVALID;
    }
    *status = sstatus.s32Status;
    *mode = sstatus.eMode;

    return CVI_SUCCESS;
}

int CVI_MAPI_VCAP_InitSensorDetect(int sns_id, void *cb)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    if(cb == NULL){
        CVI_LOGE("CVI_MAPI_VCAP_InitSensorDetect failed!\n");
        return CVI_MAPI_ERR_INVALID;
    }

    CVI_SENSOR_AHDRegisterDetect((AHD_Callback)cb);

    s32Ret = CVI_SENSOR_EnableDetect(sns_id);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_SENSOR_EnableDetect failed!\n");
        return CVI_MAPI_ERR_INVALID;
    }

    return CVI_SUCCESS;
}

static int _MAPI_VCAP_StartSensor(MAPI_VI_INFO_S *VI_Info, int sns_id)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    MAPI_VI_INFO_S *pstViInfo = VI_Info;

    ISP_SNS_CFG_S isp_sns_cfg;
    VI_PIPE_ATTR_S stViPipeAttr;

    s32Ret = getPipeAttr(pstViInfo->stSnsInfo.enSnsType, &stViPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getPipeAttr(%d) failed!\n", sns_id);

    isp_sns_cfg.stSnsSize.u32Width = pstViInfo->stChnInfo.u32Width;
    isp_sns_cfg.stSnsSize.u32Height = pstViInfo->stChnInfo.u32Height;
    isp_sns_cfg.f32FrameRate = pstViInfo->stChnInfo.f32Fps;
    isp_sns_cfg.enWDRMode = pstViInfo->stChnInfo.enWDRMode;
    isp_sns_cfg.bHwSync = pstViInfo->stSnsInfo.u8HwSync;
    isp_sns_cfg.S32MipiDevno = pstViInfo->stSnsInfo.MipiDev;
    isp_sns_cfg.bMclkEn = 1;
    isp_sns_cfg.u8Mclk = pstViInfo->stSnsInfo.u8CamClkId;
    isp_sns_cfg.lane_id[0] = pstViInfo->stSnsInfo.as16LaneId[0];
    isp_sns_cfg.lane_id[1] = pstViInfo->stSnsInfo.as16LaneId[1];
    isp_sns_cfg.lane_id[2] = pstViInfo->stSnsInfo.as16LaneId[2];
    isp_sns_cfg.lane_id[3] = pstViInfo->stSnsInfo.as16LaneId[3];
    isp_sns_cfg.lane_id[4] = pstViInfo->stSnsInfo.as16LaneId[4];
    isp_sns_cfg.pn_swap[0]  = pstViInfo->stSnsInfo.as8PNSwap[0];
    isp_sns_cfg.pn_swap[1]  = pstViInfo->stSnsInfo.as8PNSwap[1];
    isp_sns_cfg.pn_swap[2]  = pstViInfo->stSnsInfo.as8PNSwap[2];
    isp_sns_cfg.pn_swap[3]  = pstViInfo->stSnsInfo.as8PNSwap[3];
    isp_sns_cfg.pn_swap[4]  = pstViInfo->stSnsInfo.as8PNSwap[4];

    // if (stViPipeAttr.bYuvBypassPath == CVI_FALSE) {
        if (stViPipeAttr.bYuvBypassPath == CVI_FALSE) {
            SNS_I2C_GPIO_INFO_S sns_cfg;

            sns_cfg.s32I2cAddr = pstViInfo->stSnsInfo.s32SnsI2cAddr;
            sns_cfg.s8I2cDev = pstViInfo->stSnsInfo.s32BusId;
            sns_cfg.u32Rst_port_idx = pstViInfo->stSnsInfo.u8RstGpioInx;
            sns_cfg.u32Rst_pin = pstViInfo->stSnsInfo.u8RstGpioPin;
            sns_cfg.u32Rst_pol = pstViInfo->stSnsInfo.u8RstGpioPol;

            CVI_LOGD("sns_cfg I2cAddr %d I2cDev %d idx %u pin %u pol %u", sns_cfg.s32I2cAddr,
                    sns_cfg.s8I2cDev, sns_cfg.u32Rst_port_idx, sns_cfg.u32Rst_pin, sns_cfg.u32Rst_pol);
            CVI_SENSOR_GPIO_Init(sns_id, &sns_cfg);
        }
        // s32Ret = CVI_SENSOR_SetSnsType(sns_id, enSnsType[sns_id]);
        // CHECK_MAPI_VCAP_RET(s32Ret, "CVI_SENSOR_SetSnsType(%d) failed!\n", sns_id);
        // s32Ret = CVI_SENSOR_SetSnsI2c(sns_id, pstViInfo->stSnsInfo.s32BusId, pstViInfo->stSnsInfo.s32SnsI2cAddr);
        // CHECK_MAPI_VCAP_RET(s32Ret, "CVI_SENSOR_SetSnsI2c(%d) failed!\n", sns_id);
        // s32Ret = CVI_SENSOR_SetSnsGpioInit(sns_id, pstViInfo->stSnsInfo.u8RstGpioInx,
        //     pstViInfo->stSnsInfo.u8RstGpioPin, pstViInfo->stSnsInfo.u8RstGpioPol);
        // CHECK_MAPI_VCAP_RET(s32Ret, "CVI_SENSOR_SetSnsGpioInit(%d) failed!\n", sns_id);

        // CVI_LOGD("sns_cfg I2cAddr %d I2cDev %d idx %u pin %u pol %u", pstViInfo->stSnsInfo.s32SnsI2cAddr,
        //         pstViInfo->stSnsInfo.s32BusId, pstViInfo->stSnsInfo.u8RstGpioInx,
        //         pstViInfo->stSnsInfo.u8RstGpioPin, pstViInfo->stSnsInfo.u8RstGpioPol);
    // }

    if (CVI_ISP_SnsInit(sns_id, &isp_sns_cfg) != CVI_SUCCESS) {
        CVI_LOGE("CVI_ISP_SnsInit failed\n");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static int _MAPI_VCAP_Init(CVI_MAPI_VCAP_HANDLE_T *vcap_hdl, CVI_MAPI_VCAP_ATTR_T *vcap_attr)
{
    int sns_num = 0;

    sns_num = vcap_attr->u8DevNum;
    if ((sns_num == 0) || (sns_num > VI_MAX_DEV_NUM)) {
        CVI_LOGE("sensor dev number (%d) error\n", sns_num);
        return CVI_MAPI_ERR_INVALID;
    }

    CVI_MAPI_VCAP_CTX_T *vt = NULL;
    vt = (CVI_MAPI_VCAP_CTX_T *)malloc(sizeof(CVI_MAPI_VCAP_CTX_T));
    if (!vt) {
        CVI_LOGE("malloc failed\n");
        return CVI_MAPI_ERR_NOMEM;
    }

    memset(vt, 0, sizeof(CVI_MAPI_VCAP_CTX_T));
    vt->ViConfig.s32WorkingViNum = sns_num;

    for (int i = 0; i < sns_num; i++) {
        vt->ViConfig.astViInfo[i].stSnsInfo.s32SnsId        = i;
        vt->ViConfig.astViInfo[i].stSnsInfo.enSnsType       = enSnsType[i];
        printf("enter _MAPI_VCAP_Init, enSnsType[i] = %d\n", enSnsType[i]);
        vt->ViConfig.astViInfo[i].stSnsInfo.MipiDev         = vcap_attr->attr_sns[i].u8MipiDev;
        vt->ViConfig.astViInfo[i].stSnsInfo.s32BusId        = vcap_attr->attr_sns[i].u8I2cBusId;
        vt->ViConfig.astViInfo[i].stSnsInfo.s32SnsI2cAddr   = vcap_attr->attr_sns[i].u8I2cSlaveAddr;
        vt->ViConfig.astViInfo[i].stSnsInfo.u8HwSync        = vcap_attr->attr_sns[i].u8HwSync;

        vt->ViConfig.astViInfo[i].stSnsInfo.as16LaneId[0]   = vcap_attr->attr_sns[i].as8LaneId[0];
        vt->ViConfig.astViInfo[i].stSnsInfo.as16LaneId[1]   = vcap_attr->attr_sns[i].as8LaneId[1];
        vt->ViConfig.astViInfo[i].stSnsInfo.as16LaneId[2]   = vcap_attr->attr_sns[i].as8LaneId[2];
        vt->ViConfig.astViInfo[i].stSnsInfo.as16LaneId[3]   = vcap_attr->attr_sns[i].as8LaneId[3];
        vt->ViConfig.astViInfo[i].stSnsInfo.as16LaneId[4]   = vcap_attr->attr_sns[i].as8LaneId[4];

        vt->ViConfig.astViInfo[i].stSnsInfo.as8PNSwap[0]    = vcap_attr->attr_sns[i].as8PNSwap[0];
        vt->ViConfig.astViInfo[i].stSnsInfo.as8PNSwap[1]    = vcap_attr->attr_sns[i].as8PNSwap[1];
        vt->ViConfig.astViInfo[i].stSnsInfo.as8PNSwap[2]    = vcap_attr->attr_sns[i].as8PNSwap[2];
        vt->ViConfig.astViInfo[i].stSnsInfo.as8PNSwap[3]    = vcap_attr->attr_sns[i].as8PNSwap[3];
        vt->ViConfig.astViInfo[i].stSnsInfo.as8PNSwap[4]    = vcap_attr->attr_sns[i].as8PNSwap[4];

        vt->ViConfig.astViInfo[i].stSnsInfo.u8CamClkId      = vcap_attr->attr_sns[i].u8CamClkId;
        vt->ViConfig.astViInfo[i].stSnsInfo.u8RstGpioInx    = vcap_attr->attr_sns[i].u8RstGpioInx;
        vt->ViConfig.astViInfo[i].stSnsInfo.u8RstGpioPin    = vcap_attr->attr_sns[i].u8RstGpioPin;
        vt->ViConfig.astViInfo[i].stSnsInfo.u8RstGpioPol    = vcap_attr->attr_sns[i].u8RstGpioPol;

        vt->ViConfig.astViInfo[i].stChnInfo.s32ChnId        = i;
        vt->ViConfig.astViInfo[i].stChnInfo.u32Width        = vcap_attr->attr_chn[i].u32Width;
        vt->ViConfig.astViInfo[i].stChnInfo.u32Height       = vcap_attr->attr_chn[i].u32Height;
        vt->ViConfig.astViInfo[i].stChnInfo.f32Fps          = (vcap_attr->attr_chn[i].f32Fps==0)?25.0f:vcap_attr->attr_chn[i].f32Fps;
        vt->ViConfig.astViInfo[i].stChnInfo.enPixFormat     = vcap_attr->attr_chn[i].enPixelFmt;
        vt->ViConfig.astViInfo[i].stChnInfo.enWDRMode       = vcap_attr->attr_sns[i].u8WdrMode;
        vt->ViConfig.astViInfo[i].stChnInfo.enCompressMode  = vcap_attr->attr_chn[i].enCompressMode;
        vt->ViConfig.astViInfo[i].stChnInfo.fbmEnable       = vcap_attr->attr_chn[i].fbmEnable;
        vt->ViConfig.astViInfo[i].stChnInfo.vbcnt       = vcap_attr->attr_chn[i].vbcnt;

        CVI_LOGI("Sensor[%d]: type %d, mipi %d, bus %d, s32SnsI2cAddr %d\n",
                i,
                vt->ViConfig.astViInfo[i].stSnsInfo.enSnsType,
                vt->ViConfig.astViInfo[i].stSnsInfo.MipiDev,
                vt->ViConfig.astViInfo[i].stSnsInfo.s32BusId,
                vt->ViConfig.astViInfo[i].stSnsInfo.s32SnsI2cAddr);
    }

    // if (CVI_SYS_VI_Open() != CVI_SUCCESS) {
    //     CVI_LOGE("CVI_SYS_VI_Open failed\n");
    //     goto error;
    // }

    for (int i = 0; i < sns_num; i++) {
        if (_MAPI_VCAP_StartSensor(&vt->ViConfig.astViInfo[i], i) != CVI_SUCCESS) {
            CVI_LOGE("Start Sensor failed\n");
            goto error;
        }
    }

    *vcap_hdl = (CVI_MAPI_VCAP_HANDLE_T *)vt;
    return CVI_MAPI_SUCCESS;
error:
    if (vt != NULL) {
        free(vt);
        vt = NULL;
    }
    return CVI_MAPI_ERR_FAILURE;
}

static int _MAPI_VCAP_Deinit(CVI_MAPI_VCAP_HANDLE_T vcap_hdl)
{
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)vcap_hdl;

    if (vt == NULL) {
        return CVI_MAPI_SUCCESS;
    }

    if (vt != NULL) {
        free(vt);
        vt = NULL;
    }

    // if (CVI_SYS_VI_Close() != CVI_SUCCESS) {
    //     CVI_LOGE("CVI_SYS_VI_Close failed\n");
    //     return CVI_MAPI_ERR_FAILURE;
    // }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_InitSensor(CVI_MAPI_VCAP_SENSOR_HANDLE_T *sns_hdl, int sns_id, CVI_MAPI_VCAP_ATTR_T *vcap_attr)
{
    CVI_LOGI("%s(%d)\n", __FUNCTION__, sns_id);
    CHECK_MAPI_VCAP_NULL_PTR_RET(vcap_attr);
    CVI_LOGI("[###VCAP ATTR######][SENSOR0] u8SnsId : %d, u8WdrMode : %d, u8I2cBusId : %d, u8I2cSlaveAddr : %d, u8HwSync : %d, u8MipiDev : %d, u8CamClkId : %d, u8RstGpioInx : %d, u8RstGpioPin : %d, u8RstGpioPol : %d, as8LaneId : %d, %d, %d, %d, %d, as8PNSwap : %d, %d, %d, %d, %d \n",
        vcap_attr->attr_sns[0].u8SnsId,
        vcap_attr->attr_sns[0].u8WdrMode,
        vcap_attr->attr_sns[0].u8I2cBusId,
        vcap_attr->attr_sns[0].u8I2cSlaveAddr,
        vcap_attr->attr_sns[0].u8HwSync,
        vcap_attr->attr_sns[0].u8MipiDev,
        vcap_attr->attr_sns[0].u8CamClkId,
        vcap_attr->attr_sns[0].u8RstGpioInx,
        vcap_attr->attr_sns[0].u8RstGpioPin,
        vcap_attr->attr_sns[0].u8RstGpioPol,
        vcap_attr->attr_sns[0].as8LaneId[0],
        vcap_attr->attr_sns[0].as8LaneId[1],
        vcap_attr->attr_sns[0].as8LaneId[2],
        vcap_attr->attr_sns[0].as8LaneId[3],
        vcap_attr->attr_sns[0].as8LaneId[4],
        vcap_attr->attr_sns[0].as8PNSwap[0],
        vcap_attr->attr_sns[0].as8PNSwap[1],
        vcap_attr->attr_sns[0].as8PNSwap[2],
        vcap_attr->attr_sns[0].as8PNSwap[3],
        vcap_attr->attr_sns[0].as8PNSwap[4]);
    CVI_LOGI("[###VCAP ATTR######][SENSOR1] u8SnsId : %d, u8WdrMode : %d, u8I2cBusId : %d, u8I2cSlaveAddr : %d, u8HwSync : %d, u8MipiDev : %d, u8CamClkId : %d, u8RstGpioInx : %d, u8RstGpioPin : %d, u8RstGpioPol : %d, as8LaneId : %d, %d, %d, %d, %d, as8PNSwap : %d, %d, %d, %d, %d \n",
        vcap_attr->attr_sns[1].u8SnsId,
        vcap_attr->attr_sns[1].u8WdrMode,
        vcap_attr->attr_sns[1].u8I2cBusId,
        vcap_attr->attr_sns[1].u8I2cSlaveAddr,
        vcap_attr->attr_sns[1].u8HwSync,
        vcap_attr->attr_sns[1].u8MipiDev,
        vcap_attr->attr_sns[1].u8CamClkId,
        vcap_attr->attr_sns[1].u8RstGpioInx,
        vcap_attr->attr_sns[1].u8RstGpioPin,
        vcap_attr->attr_sns[1].u8RstGpioPol,
        vcap_attr->attr_sns[1].as8LaneId[0],
        vcap_attr->attr_sns[1].as8LaneId[1],
        vcap_attr->attr_sns[1].as8LaneId[2],
        vcap_attr->attr_sns[1].as8LaneId[3],
        vcap_attr->attr_sns[1].as8LaneId[4],
        vcap_attr->attr_sns[1].as8PNSwap[0],
        vcap_attr->attr_sns[1].as8PNSwap[1],
        vcap_attr->attr_sns[1].as8PNSwap[2],
        vcap_attr->attr_sns[1].as8PNSwap[3],
        vcap_attr->attr_sns[1].as8PNSwap[4]);

    pthread_mutex_lock(&vcap_mutex);

    if (vcap_ctx == NULL) {
        if (_MAPI_VCAP_Init(&vcap_ctx, vcap_attr) != CVI_MAPI_SUCCESS) {
            CVI_LOGE("_MAPI_VCAP_Init fail\n");
            pthread_mutex_unlock(&vcap_mutex);
            return CVI_MAPI_ERR_FAILURE;
        }
    }

    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)vcap_ctx;

    CVI_MAPI_VCAP_SENSOR_T *st = NULL;
    st = (CVI_MAPI_VCAP_SENSOR_T *)malloc(sizeof(CVI_MAPI_VCAP_SENSOR_T));
    if (!st) {
        CVI_LOGE("malloc failed\n");
        _MAPI_VCAP_Deinit(vcap_ctx);
        pthread_mutex_unlock(&vcap_mutex);
        return CVI_MAPI_ERR_NOMEM;
    }
    memset(st, 0, sizeof(CVI_MAPI_VCAP_SENSOR_T));
    st->sns_id = sns_id;
    st->attr = *vcap_attr;
    st->vcap_hdl = vcap_ctx;

    vt->ref_count ++;
    pthread_mutex_unlock(&vcap_mutex);

    *sns_hdl = (CVI_MAPI_VCAP_SENSOR_HANDLE_T)st;
    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_DeinitSensor(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_LOGI("%s(%d)\n", __FUNCTION__, st->sns_id);

    pthread_mutex_lock(&vcap_mutex);

    if (vcap_ctx == NULL) {
        CVI_LOGE("VCAP not initialized\n");
        pthread_mutex_unlock(&vcap_mutex);
        return CVI_MAPI_ERR_INVALID;
    }
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)vcap_ctx;
    vt->ref_count --;
    CVI_LOG_ASSERT(vt->ref_count >= 0, "VCAP CTX ref_count < 0\n");

    if (vt->ref_count == 0) {
        _MAPI_VCAP_Deinit(vcap_ctx);
        vcap_ctx = NULL;
    }

    pthread_mutex_unlock(&vcap_mutex);

    if (st != NULL) {
        free(st);
        st = NULL;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_StartDev(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;
    CVI_LOGI("%s(%d)\n", __FUNCTION__, sns_id);

    VI_DEV         ViDev;
    VI_DEV_ATTR_S  stViDevAttr;
    ISP_PUB_ATTR_S stPubAttr;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViDev = pstViInfo->stChnInfo.s32ChnId;

    memset(&stViDevAttr, 0x0, sizeof(stViDevAttr));
    stViDevAttr.stWDRAttr.enWDRMode = pstViInfo->stChnInfo.enWDRMode;
    s32Ret = getDevAttr(pstViInfo->stSnsInfo.enSnsType, &stViDevAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getDevAttr(%d) failed!\n", ViDev);

    memset(&stPubAttr, 0x0, sizeof(stPubAttr));
    s32Ret = getIspPubAttr(pstViInfo->stSnsInfo.enSnsType, &stPubAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getIspPubAttr(%d) failed!\n", ViDev);

    stViDevAttr.stSize.u32Width     = pstViInfo->stChnInfo.u32Width;
    stViDevAttr.stSize.u32Height    = pstViInfo->stChnInfo.u32Height;
    stViDevAttr.enBayerFormat = stPubAttr.enBayer;
    stViDevAttr.snrFps = pstViInfo->stChnInfo.f32Fps;
    // stViDevAttr.fbmEnable = pstViInfo->stChnInfo.fbmEnable;
    stViDevAttr.phy_addr = 0;
    stViDevAttr.phy_size = 0;
    // CVI_LOGD("in cvi_mapi_vcap.c: enWDRMode = %d\n", stViDevAttr.stWDRAttr.enWDRMode);

    s32Ret = CVI_VI_SetDevAttr(ViDev, &stViDevAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_VI_SetDevAttr(%d) failed!\n", ViDev);

    s32Ret = CVI_VI_EnableDev(ViDev);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_VI_EnableDev(%d) failed!\n", ViDev);

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_StopDev(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;
    CVI_LOGI("%s(%d)\n", __FUNCTION__, sns_id);

    VI_DEV         ViDev;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViDev = pstViInfo->stChnInfo.s32ChnId;

    if (CVI_VI_DisableDev(ViDev) != CVI_SUCCESS) {
        CVI_LOGE("CVI_VI_DisableDev failed with %d\n", ViDev);
        return CVI_MAPI_ERR_FAILURE;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_StartChn(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;
    CVI_LOGI("%s(%d)\n", __FUNCTION__, sns_id);

    VI_PIPE        ViPipe;
    VI_CHN         ViChn;
    VI_DEV_ATTR_S  stViDevAttr;
    VI_CHN_ATTR_S  stViChnAttr;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;
    ViChn  = pstViInfo->stChnInfo.s32ChnId;

    memset(&stViDevAttr, 0x0, sizeof(stViDevAttr));
    s32Ret = getDevAttr(pstViInfo->stSnsInfo.enSnsType, &stViDevAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getDevAttr(%d) failed!\n", ViPipe);

    s32Ret = getChnAttr(pstViInfo->stSnsInfo.enSnsType, &stViChnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getChnAttr(%d) failed!\n", ViPipe);

    stViChnAttr.stSize.u32Width  = pstViInfo->stChnInfo.u32Width;
    stViChnAttr.stSize.u32Height = pstViInfo->stChnInfo.u32Height;
    stViChnAttr.u32Depth         = 0; // depth
    stViChnAttr.enPixelFormat = pstViInfo->stChnInfo.enPixFormat;
    stViChnAttr.enCompressMode = pstViInfo->stChnInfo.enCompressMode;
    s32Ret = CVI_VI_SetChnAttr(ViPipe, ViChn, &stViChnAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_VI_SetChnAttr(%d) failed!\n", ViPipe);

    s32Ret = CVI_SENSOR_SetVIFlipMirrorCB(ViPipe, ViChn);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_SENSOR_SetVIFlipMirrorCB(%d, %d) failed!\n", ViPipe, ViChn);

    if(pstViInfo->stChnInfo.vbcnt > 0) {
        VI_VPSS_MODE_S stVIVPSSMode;
        CVI_SYS_GetVIVPSSMode(&stVIVPSSMode);

        if(stVIVPSSMode.aenMode[sns_id] == VI_OFFLINE_VPSS_OFFLINE) {
            VB_POOL_CONFIG_S stVbPoolCfg;
            VB_POOL chnVbPool;
            CVI_U32 u32BlkSize = 0;

            u32BlkSize = COMMON_GetPicBufferSize(pstViInfo->stChnInfo.u32Width, pstViInfo->stChnInfo.u32Height ,pstViInfo->stChnInfo.enPixFormat ,
                DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

            memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
            stVbPoolCfg.u32BlkSize	= u32BlkSize;
            stVbPoolCfg.u32BlkCnt	= pstViInfo->stChnInfo.vbcnt;
            stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
            chnVbPool = CVI_VB_CreatePool(&stVbPoolCfg);
            if (chnVbPool == VB_INVALID_POOLID) {
                CVI_LOGE("CVI_VB_CreatePool failed.\n");
            } else {
                CVI_VI_AttachVbPool(ViPipe,ViChn,chnVbPool);
            }
        }
    }

    s32Ret = CVI_VI_EnableChn(ViPipe, ViChn);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_VI_EnableChn(%d) failed!\n", ViPipe);

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_StopChn(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;
    CVI_LOGI("%s(%d)\n", __FUNCTION__, sns_id);

    VI_PIPE        ViPipe;
    VI_CHN         ViChn;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;
    ViChn  = pstViInfo->stChnInfo.s32ChnId;

    CVI_VI_UnRegChnFlipMirrorCallBack(ViPipe, ViChn);

    s32Ret = CVI_VI_DisableChn(ViPipe, ViChn);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_VI_DisableChn(%d) failed!\n", ViPipe);

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_StartPipe(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;
    CVI_LOGI("%s(%d)\n", __FUNCTION__, sns_id);

    VI_PIPE        ViPipe;
    VI_PIPE_ATTR_S stViPipeAttr;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;

    s32Ret = getPipeAttr(pstViInfo->stSnsInfo.enSnsType, &stViPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getPipeAttr(%d) failed!\n", ViPipe);

    stViPipeAttr.u32MaxW = pstViInfo->stChnInfo.u32Width;
    stViPipeAttr.u32MaxH = pstViInfo->stChnInfo.u32Height;
    stViPipeAttr.enCompressMode = pstViInfo->stChnInfo.enCompressMode;
    s32Ret = CVI_VI_CreatePipe(ViPipe, &stViPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_VI_CreatePipe(%d) failed!\n", ViPipe);

    s32Ret = CVI_VI_StartPipe(ViPipe);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_VI_StartPipe(%d) failed!\n", ViPipe);

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_StopPipe(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;
    CVI_LOGI("%s(%d)\n", __FUNCTION__, sns_id);

    VI_PIPE        ViPipe;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;

    s32Ret = CVI_VI_DestroyPipe(ViPipe);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_VI_DestroyPipe(%d) failed!\n", ViPipe);

    s32Ret = CVI_VI_StopPipe(ViPipe);
    CHECK_MAPI_VCAP_RET(s32Ret, "CVI_VI_StopPipe(%d) failed!\n", ViPipe);

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_InitISP(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;
    CVI_LOGI("%s(%d)\n", __FUNCTION__, sns_id);

    VI_PIPE              ViPipe;
    MAPI_VI_INFO_S       *pstViInfo = CVI_NULL;
    VI_PIPE_ATTR_S stViPipeAttr;
    ISP_PUB_ATTR_S stPubAttr;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;

    s32Ret = getPipeAttr(pstViInfo->stSnsInfo.enSnsType, &stViPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getPipeAttr(%d) failed!\n", ViPipe);

    if (stViPipeAttr.bYuvBypassPath == CVI_TRUE) {
        CVI_LOGW("yuv sensor skip isp init\n");
        return CVI_SUCCESS;
    }

    s32Ret = CVI_ISP_SetBypassFrm(ViPipe, CVI_ISP_BYPASS_CNT);
    CHECK_MAPI_VCAP_RET(s32Ret, "ISP Set bypass fail, ViPipe[%d]\n", ViPipe);

    s32Ret = CVI_ISP_Init(ViPipe);
    CHECK_MAPI_VCAP_RET(s32Ret, "ISP Init fail, ViPipe[%d]\n", ViPipe);

    s32Ret = CVI_ISP_GetPubAttr(ViPipe, &stPubAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "ISP GetPubAttr fail, ViPipe[%d]\n", ViPipe);
    stPubAttr.f32FrameRate = pstViInfo->stChnInfo.f32Fps;
    s32Ret = CVI_ISP_SetPubAttr(ViPipe, &stPubAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "ISP SetPubAttr fail, ViPipe[%d]\n", ViPipe);

    CVI_MAPI_VCAP_BIN_ReadParaFrombin(s_BinId[sns_id]);

#ifdef ENABLE_ISP_PQ_TOOL
    if (!g_ISPDaemon) {
        isp_daemon2_init(ISPD_CONNECT_PORT);
        g_ISPDaemon = CVI_TRUE;
    }
#endif

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_DeInitISP(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;
    CVI_LOGI("%s(%d)\n", __FUNCTION__, sns_id);
    VI_PIPE        ViPipe;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;
    VI_PIPE_ATTR_S stViPipeAttr;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];

    s32Ret = getPipeAttr(pstViInfo->stSnsInfo.enSnsType, &stViPipeAttr);
    if (stViPipeAttr.bYuvBypassPath == CVI_TRUE) {
        return CVI_MAPI_SUCCESS;
    }
    ViPipe = pstViInfo->stChnInfo.s32ChnId;
    s32Ret = CVI_ISP_Exit(ViPipe);

    return s32Ret;
}

int CVI_MAPI_VCAP_SetAttrEx(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, CVI_MAPI_VCAP_CMD_E enCMD,
                            void *pAttr, uint32_t u32Len)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(sns_hdl);
    CHECK_MAPI_VCAP_NULL_PTR_RET(pAttr);
    CHECK_MAPI_VCAP_MAX_VAL_RET("enCMD", enCMD, (CVI_MAPI_VCAP_CMD_BUTT - 1));
    CHECK_MAPI_VCAP_ZERO_VAL_RET("u32Len", u32Len);

    CVI_S32 s32Ret = CVI_MAPI_SUCCESS;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;

    VI_PIPE        ViPipe;
    VI_CHN         ViChn;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;
    ViChn  = pstViInfo->stChnInfo.s32ChnId;
    VI_PIPE_ATTR_S stViPipeAttr;

    s32Ret = getPipeAttr(pstViInfo->stSnsInfo.enSnsType, &stViPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getPipeAttr(%d) failed!\n", ViPipe);

    switch (enCMD) {
    case CVI_MAPI_VCAP_CMD_Fps: {
        ISP_PUB_ATTR_S pubAttr;
        CVI_FLOAT *f32FrameRate;
        if (stViPipeAttr.bYuvBypassPath == CVI_TRUE) {
            CVI_LOGW("yuv sensor skip isp ops\n");
            return CVI_MAPI_SUCCESS;
        }
        f32FrameRate = (CVI_FLOAT *)pAttr;
        s32Ret = CVI_ISP_GetPubAttr(ViPipe, &pubAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call CVI_ISP_GetPubAttr fail, ViPipe[%d]\n", ViPipe);

        if (*f32FrameRate == pubAttr.f32FrameRate) {
            return CVI_MAPI_SUCCESS;
        }

        pubAttr.f32FrameRate = *f32FrameRate;
        s32Ret = CVI_ISP_SetPubAttr(ViPipe, &pubAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call CVI_ISP_SetPubAttr fail, ViPipe[%d]\n", ViPipe);
        break;
    }
    case CVI_MAPI_VCAP_CMD_Rotate: {
        ROTATION_E enRotation;
        ROTATION_E *penRotationTemp;

        penRotationTemp = (ROTATION_E *)pAttr;
        s32Ret = CVI_VI_GetChnRotation(ViPipe, ViChn, &enRotation);
        CHECK_MAPI_VCAP_RET(s32Ret, "call CVI_VI_GetChnRotation fail, ViChn[%d]\n", ViChn);

        if (*penRotationTemp == enRotation) {
            return CVI_MAPI_SUCCESS;
        }

        s32Ret = CVI_VI_SetChnRotation(ViPipe, ViChn, *penRotationTemp);
        CHECK_MAPI_VCAP_RET(s32Ret, "call CVI_VI_SetChnRotation fail, ViChn[%d]\n", ViChn);
        break;
    }
    case CVI_MAPI_VCAP_CMD_MirrorFlip: {
        CVI_BOOL tmpFlip, tmpMirror;
        CVI_MAPI_VCAP_MIRRORFLIP_ATTR_S *pstMirrorFlip;

        pstMirrorFlip = (CVI_MAPI_VCAP_MIRRORFLIP_ATTR_S *)pAttr;
        s32Ret = CVI_VI_GetChnFlipMirror(ViPipe, ViChn, &tmpFlip, &tmpMirror);
        CHECK_MAPI_VCAP_RET(s32Ret, "call CVI_VI_GetChnFlipMirror fail, ViChn[%d]\n", ViChn);

        if ((pstMirrorFlip->bFlip == tmpFlip) && (pstMirrorFlip->bMirror == tmpMirror)) {
            return CVI_MAPI_SUCCESS;
        }

        s32Ret = CVI_VI_SetChnFlipMirror(ViPipe, ViChn, pstMirrorFlip->bFlip, pstMirrorFlip->bMirror);
        CHECK_MAPI_VCAP_RET(s32Ret, "call CVI_VI_SetChnFlipMirror fail, ViChn[%d]\n", ViChn);
        break;
    }
    default:
        break;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_GetAttrEx(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, CVI_MAPI_VCAP_CMD_E enCMD,
                            void *pAttr, uint32_t u32Len)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(sns_hdl);
    CHECK_MAPI_VCAP_NULL_PTR_RET(pAttr);
    CHECK_MAPI_VCAP_MAX_VAL_RET("enCMD", enCMD, (CVI_MAPI_VCAP_CMD_BUTT - 1));
    CHECK_MAPI_VCAP_ZERO_VAL_RET("u32Len", u32Len);

    CVI_S32 s32Ret = CVI_MAPI_SUCCESS;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;

    VI_PIPE        ViPipe;
    VI_CHN         ViChn;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;
    ViChn  = pstViInfo->stChnInfo.s32ChnId;
    VI_PIPE_ATTR_S stViPipeAttr;
    s32Ret = getPipeAttr(pstViInfo->stSnsInfo.enSnsType, &stViPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getPipeAttr(%d) failed!\n", ViPipe);

    switch (enCMD) {
    case CVI_MAPI_VCAP_CMD_Fps: {
        ISP_PUB_ATTR_S pubAttr;
        if (stViPipeAttr.bYuvBypassPath == CVI_TRUE) {
            CVI_LOGW("yuv sensor skip isp ops\n");
            return CVI_SUCCESS;
        }
        s32Ret = CVI_ISP_GetPubAttr(ViPipe, &pubAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call CVI_ISP_GetPubAttr fail, ViPipe[%d]\n", ViPipe);

        *((CVI_FLOAT *)pAttr) = pubAttr.f32FrameRate;
        break;
    }
    case CVI_MAPI_VCAP_CMD_Rotate: {
        s32Ret = CVI_VI_GetChnRotation(ViPipe, ViChn, (ROTATION_E *)pAttr);
        CHECK_MAPI_VCAP_RET(s32Ret, "call CVI_VI_GetChnRotation fail, ViChn[%d]\n", ViChn);
        break;
    }
    case CVI_MAPI_VCAP_CMD_MirrorFlip: {
        CVI_BOOL tmpFlip, tmpMirror;
        s32Ret = CVI_VI_GetChnFlipMirror(ViPipe, ViChn, &tmpFlip, &tmpMirror);
        CHECK_MAPI_VCAP_RET(s32Ret, "call CVI_VI_GetChnFlipMirror fail, ViChn[%d]\n", ViChn);

        ((CVI_MAPI_VCAP_MIRRORFLIP_ATTR_S *)pAttr)->bMirror = tmpMirror;
        ((CVI_MAPI_VCAP_MIRRORFLIP_ATTR_S *)pAttr)->bFlip = tmpFlip;
        break;
    }
    default:
        break;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_SetChnCropAttr(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, VI_CROP_INFO_S *pstCropInfo)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(pstCropInfo);
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;

    VI_PIPE        ViPipe;
    VI_CHN         ViChn;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;
    ViChn  = pstViInfo->stChnInfo.s32ChnId;

    if (CVI_VI_SetChnCrop(ViPipe, ViChn, pstCropInfo) != CVI_SUCCESS) {
        CVI_LOGE("CVI_VI_SetChnCrop failed\n");
        return CVI_MAPI_ERR_FAILURE;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_GetChnCropAttr(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, VI_CROP_INFO_S *pstCropInfo)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(pstCropInfo);
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;

    VI_PIPE        ViPipe;
    VI_CHN         ViChn;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;
    ViChn  = pstViInfo->stChnInfo.s32ChnId;

    if (CVI_VI_GetChnCrop(ViPipe, ViChn, pstCropInfo) != CVI_SUCCESS) {
        CVI_LOGE("CVI_VI_GetChnCrop failed\n");
        return CVI_MAPI_ERR_FAILURE;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_SetDumpRawAttr(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, VI_DUMP_ATTR_S *pstDumpAttr)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(pstDumpAttr);
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;

    VI_PIPE        ViPipe;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;

    if (CVI_VI_SetPipeDumpAttr(ViPipe, pstDumpAttr) != CVI_SUCCESS) {
        CVI_LOGE("CVI_VI_SetPipeDumpAttr failed\n");
        return CVI_MAPI_ERR_FAILURE;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_GetDumpRawAttr(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, VI_DUMP_ATTR_S *pstDumpAttr)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(pstDumpAttr);
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;

    VI_PIPE        ViPipe;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;

    if (CVI_VI_GetPipeDumpAttr(ViPipe, pstDumpAttr) != CVI_SUCCESS) {
        CVI_LOGE("CVI_VI_GetPipeDumpAttr failed\n");
        return CVI_MAPI_ERR_FAILURE;
    }

    return CVI_MAPI_SUCCESS;
}

void *VcapDumpRAWthread(void *pArg)
{
    CVI_S32 s32Ret;
    uint32_t i;
    VIDEO_FRAME_INFO_S stVideoFrame[2];
    VCAP_DUMP_RAW_CTX_T *pstArg = NULL;
    CVI_MAPI_VCAP_RAW_DATA_T *pstCallbackFun = NULL;

    memset(stVideoFrame, 0, sizeof(stVideoFrame));

    stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

    pstArg = (VCAP_DUMP_RAW_CTX_T *)pArg;
    pstCallbackFun = (CVI_MAPI_VCAP_RAW_DATA_T *)&pstArg->stCallbackFun;

    for (i = 0; i < pstArg->u32Count; i++) {
        s32Ret = CVI_VI_GetPipeFrame(pstArg->ViPipe, stVideoFrame, 1000);
        if (s32Ret != CVI_SUCCESS) {
            CVI_LOGE("Get %dth vcap frame timeout.\n", i);
            continue;
        }

        pstCallbackFun->pfn_VCAP_RawDataProc(pstArg->ViPipe, stVideoFrame, i,
                                             pstCallbackFun->pPrivateData);

        s32Ret = CVI_VI_ReleasePipeFrame(pstArg->ViPipe, stVideoFrame);
        if (s32Ret != CVI_SUCCESS) {
            CVI_LOGE("Release %dth vcap frame error.\n", i);
        }
    }
    return NULL;
}

int CVI_MAPI_VCAP_StartDumpRaw(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, uint32_t u32Count,
                               CVI_MAPI_VCAP_RAW_DATA_T *pstVCapRawData)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(sns_hdl);
    CHECK_MAPI_VCAP_NULL_PTR_RET(pstVCapRawData);
    CHECK_MAPI_VCAP_NULL_PTR_RET(pstVCapRawData->pfn_VCAP_RawDataProc);

    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;

    VI_PIPE        ViPipe;
    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;
    VI_DUMP_ATTR_S stDumpAttr;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    ViPipe = pstViInfo->stChnInfo.s32ChnId;

    if (CVI_VI_GetPipeDumpAttr(ViPipe, &stDumpAttr) != CVI_SUCCESS) {
        CVI_LOGE("CVI_VI_GetPipeDumpAttr failed\n");
        return CVI_MAPI_ERR_FAILURE;
    }

    if (stDumpAttr.bEnable != CVI_TRUE) {
        CVI_LOGE("Vcap dump raw is not been enabled.\n");
        return CVI_MAPI_ERR_FAILURE;
    }

    if (stDumpAttr.enDumpType != VI_DUMP_TYPE_RAW) {
        CVI_LOGE("Vcap dump type is not raw.\n");
        return CVI_MAPI_ERR_FAILURE;
    }

    if (u32Count == 0) {
        CVI_LOGE("Vcap dump raw number is zero.\n");
        return CVI_MAPI_ERR_FAILURE;
    }

    st->stVcapDumpRawCtx.bStart = CVI_TRUE;
    st->stVcapDumpRawCtx.ViPipe = ViPipe;
    st->stVcapDumpRawCtx.u32Count = u32Count;
    st->stVcapDumpRawCtx.stCallbackFun = *pstVCapRawData;

    s32Ret = pthread_create(&st->stVcapDumpRawCtx.pthreadDumpRaw, CVI_NULL,
                            VcapDumpRAWthread, (void *)&st->stVcapDumpRawCtx);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("Create vcap dump raw thread error.\n");
        st->stVcapDumpRawCtx.pthreadDumpRaw = 0;
        return CVI_MAPI_ERR_FAILURE;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_StopDumpRaw(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(sns_hdl);

    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;

    if ((long)st->stVcapDumpRawCtx.pthreadDumpRaw != -1) {
        s32Ret = pthread_join(st->stVcapDumpRawCtx.pthreadDumpRaw, CVI_NULL);
        if (s32Ret != CVI_SUCCESS) {
            CVI_LOGE("pthread_join failed.\n");
        }
    }
    st->stVcapDumpRawCtx.bStart = CVI_FALSE;
    st->stVcapDumpRawCtx.pthreadDumpRaw = 0;
    st->stVcapDumpRawCtx.stCallbackFun.pfn_VCAP_RawDataProc = CVI_NULL;
    st->stVcapDumpRawCtx.stCallbackFun.pPrivateData = CVI_NULL;
    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_GetSensorPipeAttr(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, int *status)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(sns_hdl);

    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;
    // CVI_LOGD("%s(%d)\n", __FUNCTION__, sns_id);

    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;
    VI_PIPE_ATTR_S stViPipeAttr;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    s32Ret = getPipeAttr(pstViInfo->stSnsInfo.enSnsType, &stViPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getPipeAttr failed!\n");

    if (stViPipeAttr.bYuvBypassPath == CVI_FALSE) {
        *status = 1;
    }

    return s32Ret;
}

int CVI_MAPI_VCAP_GetSensorPipe(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;

    //
    // TODO: FIXME
    //
    // When initializing VI, in astViInfo[id], astViInfo[i].stPipeInfo.aPipe[0]
    // is 0 and 1 for sensor 0 and sensor 1, and astViInfo[i].stChnInfo.s32ChnId is
    // always 0. However, when bind, or calling CVI_VI_GetChnFrame(), we always
    // pass pipeId as 0, and pass chnId as 0 or 1 respectively.
    // This is very confusing becasue it mixed up the definication or pipe and
    // chn.
    //
    // WR: always return 0 for GetSensorPipe
    //
#if 0
    return (int)vt->ViConfig.astViInfo[sns_id].stPipeInfo.ViPipe;
#else
    CVI_LOG_ASSERT(sns_id == (int)vt->ViConfig.astViInfo[sns_id].stChnInfo.s32ChnId,
            "sns_id %d not match with pipe Id %d\n",
            sns_id,
            (int)vt->ViConfig.astViInfo[sns_id].stChnInfo.s32ChnId);
    return 0;
#endif
}

int CVI_MAPI_VCAP_GetSensorChn(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;

    //
    // TODO: FIXME
    //
    // When initializing VI, in astViInfo[id], astViInfo[i].stPipeInfo.aPipe[0]
    // is 0 and 1 for sensor 0 and sensor 1, and astViInfo[i].stChnInfo.s32ChnId is
    // always 0. However, when bind, or calling CVI_VI_GetChnFrame(), we always
    // pass pipeId as 0, and pass chnId as 0 or 1 respectively.
    // This is very confusing becasue it mixed up the definication or pipe and
    // chn.
    //
    // WR: return sns_id for GetSensorChn
    //
#if 1
    return (int)vt->ViConfig.astViInfo[sns_id].stChnInfo.s32ChnId;
#else
    CVI_LOG_ASSERT(0 == (int)vt->ViConfig.astViInfo[sns_id].stChnInfo.s32ChnId,
            "sensor %d, chn_id %d is not zero\n",
            sns_id,
            (int)vt->ViConfig.astViInfo[sns_id].stChnInfo.s32ChnId);
    return sns_id;
#endif
}

int CVI_MAPI_VCAP_GetFrame(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl,
        VIDEO_FRAME_INFO_S *frame)
{
    VI_PIPE ViPipe = CVI_MAPI_VCAP_GetSensorPipe(sns_hdl);
    VI_CHN ViChn = CVI_MAPI_VCAP_GetSensorChn(sns_hdl);

    if (CVI_VI_GetChnFrame(ViPipe, ViChn, frame, 1000) != CVI_SUCCESS) {
        CVI_LOGE("CVI_VI_GetChnFrame ViPipe(%d) ViChn(%d) failed\n",
                    ViPipe, ViChn);
        return CVI_MAPI_ERR_FAILURE;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_ReleaseFrame(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl,
        VIDEO_FRAME_INFO_S *frame)
{
    VI_PIPE ViPipe = CVI_MAPI_VCAP_GetSensorPipe(sns_hdl);
    VI_CHN ViChn = CVI_MAPI_VCAP_GetSensorChn(sns_hdl);

    if (CVI_VI_ReleaseChnFrame(ViPipe, ViChn, frame) != CVI_SUCCESS) {
        CVI_LOGE("CVI_VI_ReleaseChnFrame ViPipe(%d) ViChn(%d) failed\n",
                    ViPipe, ViChn);
        return CVI_MAPI_ERR_FAILURE;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_GetGeneralVcapAttr(CVI_MAPI_VCAP_ATTR_T *vcap_attr)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(vcap_attr);

    getVcapAttr(vcap_attr);

    if ((vcap_attr->u8DevNum == 0) || (vcap_attr->u8DevNum > VI_MAX_DEV_NUM)) {
        CVI_LOGE("sensor dev number (%d) error\n", vcap_attr->u8DevNum);
        return CVI_MAPI_ERR_INVALID;
    }

    return CVI_MAPI_SUCCESS;
}

int CVI_MAPI_VCAP_SetPqBinPath(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl)
{
    CHECK_MAPI_VCAP_NULL_PTR_RET(sns_hdl);

    CVI_S32 s32Ret;
    CVI_MAPI_VCAP_SENSOR_T *st = (CVI_MAPI_VCAP_SENSOR_T *)sns_hdl;
    CVI_MAPI_VCAP_CTX_T *vt = (CVI_MAPI_VCAP_CTX_T *)st->vcap_hdl;
    int sns_id = st->sns_id;
    uint8_t u8WdrMode = 0;
    CVI_LOGI("%s(%d)\n", __FUNCTION__, sns_id);

    MAPI_VI_INFO_S *pstViInfo = CVI_NULL;
    VI_PIPE_ATTR_S stViPipeAttr;

    pstViInfo = &vt->ViConfig.astViInfo[sns_id];
    u8WdrMode = st->attr.attr_sns[sns_id].u8WdrMode;
    s32Ret = getPipeAttr(pstViInfo->stSnsInfo.enSnsType, &stViPipeAttr);
    CHECK_MAPI_VCAP_RET(s32Ret, "getPipeAttr failed!\n");

    if (stViPipeAttr.bYuvBypassPath == CVI_FALSE) {
        s32Ret = CVI_BIN_SetBinName(u8WdrMode, PqBinName[u8WdrMode]);
    }

    return s32Ret;
}
