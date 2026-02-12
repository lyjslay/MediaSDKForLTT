#ifndef __CVI_MAPI_VDEC_H__
#define __CVI_MAPI_VDEC_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "cvi_mapi_define.h"
#include "cvi_mapi_venc.h"  // CVI_MAPI_VCODEC_E
// #include "cvi_common.h"
#include "cvi_comm_vb.h"

#include "cvi_comm_video.h"
#include "cvi_comm_vdec.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef CVI_MAPI_HANDLE_T CVI_MAPI_VDEC_HANDLE_T;

typedef struct CVI_MAPI_VDEC_CHN_ATTR_S {
    CVI_MAPI_VCODEC_E codec;
    uint32_t          max_width;
    uint32_t          max_height;
    PIXEL_FORMAT_E    pixel_format;
} CVI_MAPI_VDEC_CHN_ATTR_T;

int CVI_MAPI_VDEC_InitChn(CVI_MAPI_VDEC_HANDLE_T *vdec_hdl,
        CVI_MAPI_VDEC_CHN_ATTR_T *attr);
int CVI_MAPI_VDEC_DeinitChn(CVI_MAPI_VDEC_HANDLE_T vdec_hdl);

int CVI_MAPI_VDEC_GetChn(CVI_MAPI_VDEC_HANDLE_T vdec_hdl);

int CVI_MAPI_VDEC_SendStream(CVI_MAPI_VDEC_HANDLE_T vdec_hdl,
        VDEC_STREAM_S *stream);
int CVI_MAPI_VDEC_GetFrame(CVI_MAPI_VDEC_HANDLE_T vdec_hdl,
        VIDEO_FRAME_INFO_S *frame);

int CVI_MAPI_VDEC_ReleaseFrame(CVI_MAPI_VDEC_HANDLE_T vdec_hdl,
        VIDEO_FRAME_INFO_S *frame);

void CVI_MAPI_GetMaxSizeByEncodeType(PAYLOAD_TYPE_E enType,
         uint32_t *max_width, uint32_t *max_height);

int CVI_MAPI_VDEC_SetVBMode(VB_SOURCE_E vbMode, CVI_U32 frameBufCnt);


#ifdef __cplusplus
}
#endif

#endif
