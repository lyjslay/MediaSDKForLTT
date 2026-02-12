/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: sensor_cfg.h
 * Description:
 */

#ifndef __SENSOR_CFG_H__
#define __SENSOR_CFG_H__

#include "cvi_comm_vi.h"
#include "cvi_sns_ctrl.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct _SNS_SIZE_S {
	CVI_U32 u32Width;
	CVI_U32 u32Height;
} SNS_SIZE_S;

typedef enum _SENSOR_TYPE_E {
    SENSOR_NONE = 0,
    SONY_IMX327_MIPI_2M_30FPS_12BIT,
    SONY_IMX335_MIPI_5M_30FPS_12BIT,
    SONY_IMX662_MIPI_2M_30FPS_12BIT,
    SONY_IMX307_MIPI_2M_30FPS_12BIT,
    SONY_IMX307_2L_MIPI_2M_30FPS_12BIT,
    SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT,
    GCORE_GC1054_MIPI_1M_30FPS_10BIT,
    GCORE_GC1084_MIPI_1M_30FPS_10BIT,
    GCORE_GC2053_MIPI_2M_30FPS_10BIT,
    GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT,
    GCORE_GC2083_MIPI_2M_30FPS_10BIT,
    GCORE_GC2093_MIPI_2M_30FPS_10BIT,
    GCORE_GC02M1_MIPI_2M_30FPS_10BIT,
    GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT,
    GCORE_GC4653_MIPI_4M_30FPS_10BIT,
    PIXELPLUS_PR2020_2M_25FPS_8BIT,
    TECHPOINT_TP9950_2M_25FPS_8BIT,
    TECHPOINT_TP9963_2M_25FPS_8BIT,
    TECHPOINT_TP9963_2M_2CH_25FPS_8BIT,
    TECHPOINT_TP9963_2M_30FPS_8BIT,
    TECHPOINT_TP9963_2M_2CH_30FPS_8BIT,
    TECHPOINT_TP9963_1M_25FPS_8BIT,
    TECHPOINT_TP9963_1M_2CH_25FPS_8BIT,
    TECHPOINT_TP9963_1M_30FPS_8BIT,
    TECHPOINT_TP9963_1M_2CH_30FPS_8BIT,
    CISTA_C4390_MIPI_4M_30FPS_10BIT,
    GCORE_GC4023_MIPI_4M_30FPS_10BIT,
    CISTA_C2395_MIPI_2M_30FPS_10BIT,
    CISTA_C4395_MIPI_4M_30FPS_10BIT,
    NEXTCHIP_N5_2M_25FPS_8BIT,
    PIXELPLUS_XS9950_2M_25FPS_8BIT,
    PIXELPLUS_XS9951_2M_25FPS_8BIT,
    OV_OS5648_4M_15FPS_8BIT,
    SENSOR_TYPE_BUTT
} SENSOR_TYPE_E;

extern ISP_SNS_OBJ_S stSnsGc1054_Obj;
extern ISP_SNS_OBJ_S stSnsGc1084_Obj;
extern ISP_SNS_OBJ_S stSnsGc2053_Obj;
extern ISP_SNS_OBJ_S stSnsGc2053_1l_Obj;
extern ISP_SNS_OBJ_S stSnsGc2083_Obj;
extern ISP_SNS_OBJ_S stSnsGc2093_Obj;
extern ISP_SNS_OBJ_S stSnsGc02m1_Obj;
extern ISP_SNS_OBJ_S stSnsGc02m1_Slave_Obj;
extern ISP_SNS_OBJ_S stSnsGc4653_Obj;
extern ISP_SNS_OBJ_S stSnsPR2020_Obj;
extern SNS_AHD_OBJ_S stAhdPr2020Obj;
extern ISP_SNS_OBJ_S stSnsImx307_Obj;
extern ISP_SNS_OBJ_S stSnsImx307_2l_Obj;
extern ISP_SNS_OBJ_S stSnsImx307_Slave_Obj;
extern ISP_SNS_OBJ_S stSnsImx327_Obj;
extern ISP_SNS_OBJ_S stSnsImx335_Obj;
extern ISP_SNS_OBJ_S stSnsImx662_Obj;
extern ISP_SNS_OBJ_S stSnsTP9950_Obj;
extern ISP_SNS_OBJ_S stSnsTP9963_Obj;
extern SNS_AHD_OBJ_S stAhdTp9950Obj;
extern SNS_AHD_OBJ_S stAhdTp9963Obj;
extern ISP_SNS_OBJ_S stSnsC4390_Obj;
extern ISP_SNS_OBJ_S stSnsGc4023_Obj;
extern ISP_SNS_OBJ_S stSnsC2395_Obj;
extern ISP_SNS_OBJ_S stSnsC4395_Obj;
extern ISP_SNS_OBJ_S stSnsN5_Obj;
extern SNS_AHD_OBJ_S stAhdN5Obj;
extern ISP_SNS_OBJ_S stSnsXS9950_Obj;
extern SNS_AHD_OBJ_S stAhdXs9950Obj;
extern ISP_SNS_OBJ_S stSnsXS9951_Obj;
extern SNS_AHD_OBJ_S stAhdXs9951Obj;
extern ISP_SNS_OBJ_S stSnsOV5648_Obj;

#if defined(CONFIG_KERNEL_RHINO)
ISP_SNS_OBJ_S *getSnsObj(SENSOR_TYPE_E enSnsType);
SNS_AHD_OBJ_S *getAhdObj(CVI_S32 snsr_type);
#endif

CVI_S32 get_sensor_type(CVI_S32 dev_id);
CVI_S32 getDevAttr(SENSOR_TYPE_E enSnsType, VI_DEV_ATTR_S *pstViDevAttr);
CVI_S32 getPipeAttr(SENSOR_TYPE_E enSnsType, VI_PIPE_ATTR_S *pstViPipeAttr);
CVI_S32 getChnAttr(SENSOR_TYPE_E enSnsType, VI_CHN_ATTR_S *pstViChnAttr);
CVI_S32 getIspPubAttr(SENSOR_TYPE_E enSnsType, ISP_PUB_ATTR_S *pstIspPubAttr);
CVI_S32 getSnsTypeByName(SNS_TYPE_S *SnsInfo);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __APP_REGION_H__*/
