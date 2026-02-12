#ifndef __CVI_MAPI_VENC_H__
#define __CVI_MAPI_VENC_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "cvi_mapi_define.h"

#include "cvi_comm_video.h"
#include "cvi_comm_venc.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef CVI_MAPI_HANDLE_T CVI_MAPI_VENC_HANDLE_T;

typedef enum _CVI_MAPI_VCODEC_E {
    CVI_MAPI_VCODEC_H264 = 0,
    CVI_MAPI_VCODEC_H265,
    CVI_MAPI_VCODEC_JPG,
    CVI_MAPI_VCODEC_MJP,
    CVI_MAPI_VCODEC_MULTI,
    CVI_MAPI_VCODEC_MAX
} CVI_MAPI_VCODEC_E;

static const PAYLOAD_TYPE_E CVI_VCODEC_TO_PAYLOAD_TYPE[CVI_MAPI_VCODEC_MAX] = {
    PT_H264,
    PT_H265,
    PT_JPEG,
    PT_MJPEG,
    PT_BUTT    // non supported
};

typedef int (*CVI_MAPI_VENC_STREAM_CALLBACK_FN_PTR)(CVI_MAPI_VENC_HANDLE_T venc_hdl,
    VENC_STREAM_S *stream, void *private_data);

typedef struct CVI_MAPI_VENC_CHN_CB_S {
    CVI_MAPI_VENC_STREAM_CALLBACK_FN_PTR   stream_cb_func;
    void                                  *stream_cb_data;
} CVI_MAPI_VENC_CHN_CB_T;

typedef struct CVI_MAPI_VENC_CHN_PARAM_S {
    CVI_MAPI_VCODEC_E codec;
    uint32_t          width;
    uint32_t          height;
    PIXEL_FORMAT_E    pixel_format;
    int               gop;
    int               profile;
    // RCMODE: 0 - CBR, 1 - VBR, 2 - AVBR, 3 - QVBR, 4 - FIXQP, 5 - QPMAP
    int               rate_ctrl_mode;
    int               bitrate_kbps;
    int               iqp;
    int               pqp;
    int               minIqp;
    int               maxIqp;
    int               minQp;
    int               maxQp;
    int               changePos;
    int               jpeg_quality;
    int               single_EsBuf;
    uint32_t          bufSize;
    uint32_t          datafifoLen;
    uint32_t          src_framerate;
    uint32_t          dst_framerate;
    int               initialDelay;
    uint32_t          thrdLv;
    uint32_t          statTime;
    int firstFrameStartQp;
    int maxBitRate;
    int gop_mode;
    int maxIprop;
    int minIprop;
    int minStillPercent;
    int maxStillQP;
    int avbrPureStillThr;
    int motionSensitivity;
    int bgDeltaQp;
    int rowQpDelta;

    uint8_t aspectRatioInfoPresentFlag;
    uint8_t aspectRatioIdc;
    uint8_t overscanInfoPresentFlag;
    uint8_t overscanAppropriateFlag;
    uint16_t sarWidth;
    uint16_t sarHeight;

    uint8_t timingInfoPresentFlag;
    uint8_t fixedFrameRateFlag;
    uint32_t numUnitsInTick;
    uint32_t timeScale;
    uint32_t num_ticks_poc_diff_one_minus1;

    uint8_t videoSignalTypePresentFlag;
    uint8_t videoFormat;
    uint8_t videoFullRangeFlag;
    uint8_t colourDescriptionPresentFlag;
    uint8_t colourPrimaries;
    uint8_t transferCharacteristics;
    uint8_t matrixCoefficients;
    uint8_t ipqpDelta;
} CVI_MAPI_VENC_CHN_PARAM_T;

typedef struct CVI_MAPI_VENC_CHN_ATTR_S {
    CVI_MAPI_VENC_CHN_PARAM_T venc_param;
    CVI_MAPI_VENC_CHN_CB_T cb;
} CVI_MAPI_VENC_CHN_ATTR_T;

int CVI_MAPI_VENC_InitChn(CVI_MAPI_VENC_HANDLE_T *venc_hdl,
        CVI_MAPI_VENC_CHN_ATTR_T *attr);
int CVI_MAPI_VENC_DeinitChn(CVI_MAPI_VENC_HANDLE_T venc_hdl);

int CVI_MAPI_VENC_GetChn(CVI_MAPI_VENC_HANDLE_T venc_hdl);

int CVI_MAPI_VENC_GetChnFd(CVI_MAPI_VENC_HANDLE_T venc_hdl);

int CVI_MAPI_VENC_SendFrame(CVI_MAPI_VENC_HANDLE_T venc_hdl,
        VIDEO_FRAME_INFO_S *frame);
int CVI_MAPI_VENC_GetStream(CVI_MAPI_VENC_HANDLE_T venc_hdl,
        VENC_STREAM_S *stream);

int CVI_MAPI_VENC_GetStreamTimeWait(CVI_MAPI_VENC_HANDLE_T venc_hdl,
        VENC_STREAM_S *stream, CVI_S32 S32MilliSec);

int CVI_MAPI_VENC_ReleaseStream(CVI_MAPI_VENC_HANDLE_T venc_hdl,
        VENC_STREAM_S *stream);

int CVI_MAPI_VENC_GetStreamStatus(CVI_MAPI_VENC_HANDLE_T venc_hdl,
        VENC_PACK_S *stream, bool *is_I_frame);

// combine CVI_MAPI_VENC_SendFrame() and CVI_MAPI_VENC_GetStream()
int CVI_MAPI_VENC_EncodeFrame(CVI_MAPI_VENC_HANDLE_T venc_hdl,
        VIDEO_FRAME_INFO_S *frame, VENC_STREAM_S *stream);

int CVI_MAPI_VENC_RequestIDR(CVI_MAPI_VENC_HANDLE_T venc_hdl);
int CVI_MAPI_VENC_SetAttr(CVI_MAPI_VENC_HANDLE_T *venc_hdl, CVI_MAPI_VENC_CHN_ATTR_T *attr);
int CVI_MAPI_VENC_GetAttr(CVI_MAPI_VENC_HANDLE_T venc_hdl, CVI_MAPI_VENC_CHN_ATTR_T *attr);
int CVI_MAPI_VENC_BindVproc(CVI_MAPI_VENC_HANDLE_T venc_hdl, int VpssGrp, int VpssChn);
int CVI_MAPI_VENC_UnBindVproc(CVI_MAPI_VENC_HANDLE_T venc_hdl, int VpssGrp, int VpssChn);
int CVI_MAPI_VENC_StartRecvFrame(CVI_MAPI_VENC_HANDLE_T venc_hdl, CVI_S32 frameCnt);
int CVI_MAPI_VENC_StopRecvFrame(CVI_MAPI_VENC_HANDLE_T venc_hdl);
int CVI_MAPI_VENC_SetBitrate(CVI_MAPI_VENC_HANDLE_T venc_hdl, CVI_U32 bitRate);
int CVI_MAPI_VENC_SetDataFifoLen(CVI_MAPI_VENC_HANDLE_T venc_hdl, CVI_U32 len);
int CVI_MAPI_VENC_GetDataFifoLen(CVI_MAPI_VENC_HANDLE_T venc_hdl, CVI_U32 *len);
#ifdef __cplusplus
}
#endif

#endif
