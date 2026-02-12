#ifndef __CVI_MAPI_VCAP_H__
#define __CVI_MAPI_VCAP_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "cvi_mapi_define.h"
#include "cvi_comm_vi.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef CVI_MAPI_HANDLE_T CVI_MAPI_VCAP_SENSOR_HANDLE_T;

typedef struct cviMAPI_VCAP_SENSOR_ATTR_S {
    uint8_t     u8SnsId;
    uint8_t     u8WdrMode; //0:linear, 3:wdr 2to1
    uint8_t     u8I2cBusId;
    uint8_t     u8I2cSlaveAddr; //0xFF means used default slave addr.
    uint8_t     u8HwSync;
    uint8_t     u8MipiDev; //0xFF means used default LaneId&PNSwap.
    uint8_t     u8CamClkId;
    uint8_t     u8RstGpioInx;
    uint8_t     u8RstGpioPin;
    uint8_t     u8RstGpioPol;
    int8_t      as8LaneId[5];
    int8_t      as8PNSwap[5];
} CVI_MAPI_VCAP_SENSOR_ATTR_T;

typedef struct cviMAPI_VCAP_CHN_ATTR_S {
    uint32_t        u32Width;
    uint32_t        u32Height;
    PIXEL_FORMAT_E  enPixelFmt;
    COMPRESS_MODE_E enCompressMode;
    bool            bMirror;
    bool            bFlip;
    float           f32Fps;
    bool            fbmEnable;
    uint32_t            vbcnt;
} CVI_MAPI_VCAP_CHN_ATTR_T;

typedef struct cviMAPI_VCAP_ATTR_S {
    uint8_t                     u8DevNum;
    CVI_MAPI_VCAP_SENSOR_ATTR_T attr_sns[VI_MAX_DEV_NUM];
    CVI_MAPI_VCAP_CHN_ATTR_T    attr_chn[VI_MAX_DEV_NUM];
} CVI_MAPI_VCAP_ATTR_T;

typedef struct cviMAPI_VCAP_RAW_DATA_S {
    void *pPrivateData;
    int32_t (*pfn_VCAP_RawDataProc)(uint32_t ViPipe, VIDEO_FRAME_INFO_S *pVCapRawData,
                                    uint32_t u32DataNum, void *pPrivateData);
} CVI_MAPI_VCAP_RAW_DATA_T;

typedef enum cviMAPI_VCAP_CMD_E
{
    CVI_MAPI_VCAP_CMD_Fps,
    CVI_MAPI_VCAP_CMD_Rotate,
    CVI_MAPI_VCAP_CMD_MirrorFlip,
    CVI_MAPI_VCAP_CMD_BUTT
}CVI_MAPI_VCAP_CMD_E;

typedef struct cviMAPI_VCAP_MIRRORFLIP_ATTR_S {
    bool bMirror;
    bool bFlip;
} CVI_MAPI_VCAP_MIRRORFLIP_ATTR_S;

int CVI_MAPI_VCAP_InitSensorDetect(int sns_id, void *cb);
int CVI_MAPI_VCAP_GetAhdMode(int sns_id, int *mode, int *status);

int CVI_MAPI_VCAP_InitSensor(CVI_MAPI_VCAP_SENSOR_HANDLE_T *sns_hdl,
        int sns_id, CVI_MAPI_VCAP_ATTR_T *vcap_attr);
int CVI_MAPI_VCAP_DeinitSensor(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_StartDev(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_StopDev(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_StartChn(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_StopChn(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_StartPipe(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_StopPipe(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_InitISP(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_DeInitISP(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_SetAttrEx(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, CVI_MAPI_VCAP_CMD_E enCMD,
                            void *pAttr, uint32_t u32Len);
int CVI_MAPI_VCAP_GetAttrEx(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, CVI_MAPI_VCAP_CMD_E enCMD,
                            void *pAttr, uint32_t u32Len);
int CVI_MAPI_VCAP_SetChnCropAttr(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, VI_CROP_INFO_S *pstCropInfo);
int CVI_MAPI_VCAP_GetChnCropAttr(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, VI_CROP_INFO_S *pstCropInfo);
int CVI_MAPI_VCAP_SetDumpRawAttr(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, VI_DUMP_ATTR_S *pstDumpAttr);
int CVI_MAPI_VCAP_GetDumpRawAttr(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, VI_DUMP_ATTR_S *pstDumpAttr);
int CVI_MAPI_VCAP_StartDumpRaw(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, uint32_t u32Count,
                               CVI_MAPI_VCAP_RAW_DATA_T *pstVCapRawData);
int CVI_MAPI_VCAP_StopDumpRaw(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_GetSensorPipeAttr(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, int *status);
int CVI_MAPI_VCAP_GetSensorPipe(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_GetSensorChn(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_GetFrame(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, VIDEO_FRAME_INFO_S *frame);
// Deprecated, use CVI_MAPI_ReleaseFrame instead
int CVI_MAPI_VCAP_ReleaseFrame(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, VIDEO_FRAME_INFO_S *frame);
int CVI_MAPI_VCAP_GetGeneralVcapAttr(CVI_MAPI_VCAP_ATTR_T *vcap_attr);
int CVI_MAPI_VCAP_GetSnsAttrFromFile(bool *dual_sns, CVI_MAPI_VCAP_ATTR_T *attr);
int CVI_MAPI_VCAP_InitSensorFromFile(CVI_MAPI_VCAP_SENSOR_HANDLE_T *sns_hdl, int sns_id, CVI_MAPI_VCAP_ATTR_T *attr);
int CVI_MAPI_VCAP_SetPqBinPath(CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl);
int CVI_MAPI_VCAP_SetAhdMode(int sns_id, int mode);
#ifdef __cplusplus
}
#endif

#endif
