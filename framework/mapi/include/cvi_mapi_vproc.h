#ifndef __CVI_MAPI_VPROC_H__
#define __CVI_MAPI_VPROC_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "string.h"
#include "cvi_mapi_define.h"
#include "cvi_mapi_vcap.h"

#include "cvi_comm_vpss.h"
#include "cvi_vpss.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef CVI_MAPI_HANDLE_T CVI_MAPI_VPROC_HANDLE_T;

#define CVI_MAPI_VPROC_MAX_CHN_NUM    (4)
#define CVI_MAPI_VPROC_TIMEOUT_MS    (1000)


typedef struct CVI_MAPI_VPROC_ATTR_S {
    VPSS_GRP_ATTR_S   attr_inp;
    int               chn_num;
    VPSS_CHN_ATTR_S   attr_chn[CVI_MAPI_VPROC_MAX_CHN_NUM];
    uint32_t          chn_vbcnt[CVI_MAPI_VPROC_MAX_CHN_NUM];
    uint32_t          lowdelay_cnt[CVI_MAPI_VPROC_MAX_CHN_NUM];
    bool              bFbOnVpss[CVI_MAPI_VPROC_MAX_CHN_NUM];
} CVI_MAPI_VPROC_ATTR_T;

typedef struct CVI_MAPI_EXTCHN_ATTR_S {
    uint32_t        ChnId;
    uint32_t        BindVprocChnId;
    VPSS_CHN_ATTR_S VpssChnAttr;
} CVI_MAPI_EXTCHN_ATTR_T;

typedef enum CVI_MAPI_VPROC_CMD_E
{
    CVI_VPROC_CMD_CHN_ROTATE,
    CVI_VPROC_CMD_CHN_CROP,
    CVI_VPROC_CMD_BUTT
}CVI_MAPI_VPROC_CMD_T;

typedef int (*PFN_VPROC_FrameDataProc)(uint32_t Grp,
    uint32_t Chn, VIDEO_FRAME_INFO_S *pFrame, void *pPrivateData);

typedef struct CVI_DUMP_FRAME_CALLBACK_FUNC_S {
    PFN_VPROC_FrameDataProc pfunFrameProc;
    void *pPrivateData;
} CVI_DUMP_FRAME_CALLBACK_FUNC_T;


static inline CVI_MAPI_VPROC_ATTR_T CVI_MAPI_VPROC_DefaultAttr_OneChn(
    uint32_t          width_in,
    uint32_t          height_in,
    PIXEL_FORMAT_E    pixel_format_in,
    uint32_t          width_out,
    uint32_t          height_out,
    PIXEL_FORMAT_E    pixel_format_out)
{
    CVI_MAPI_VPROC_ATTR_T attr;
    memset((void*)&attr, 0, sizeof(attr));

    attr.attr_inp.stFrameRate.s32SrcFrameRate    = -1;
    attr.attr_inp.stFrameRate.s32DstFrameRate    = -1;
    attr.attr_inp.enPixelFormat                  = pixel_format_in;
    attr.attr_inp.u32MaxW                        = width_in;
    attr.attr_inp.u32MaxH                        = height_in;
    attr.attr_inp.u8VpssDev                   = 0;

    attr.chn_num = 1;

    attr.attr_chn[0].u32Width                    = width_out;
    attr.attr_chn[0].u32Height                   = height_out;
    attr.attr_chn[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[0].enPixelFormat               = pixel_format_out;
    attr.attr_chn[0].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[0].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[0].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[0].bMirror                     = CVI_FALSE;
    attr.attr_chn[0].bFlip                       = CVI_FALSE;
    attr.attr_chn[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[0].stNormalize.bEnable         = CVI_FALSE;

    return attr;
}

static inline CVI_MAPI_VPROC_ATTR_T CVI_MAPI_VPROC_DefaultAttr_TwoChn(
    uint32_t          width_in,
    uint32_t          height_in,
    PIXEL_FORMAT_E    pixel_format_in,
    uint32_t          width_out0,
    uint32_t          height_out0,
    PIXEL_FORMAT_E    pixel_format_out0,
    uint32_t          width_out1,
    uint32_t          height_out1,
    PIXEL_FORMAT_E    pixel_format_out1)
{
    CVI_MAPI_VPROC_ATTR_T attr;
    memset((void*)&attr, 0, sizeof(attr));

    attr.attr_inp.stFrameRate.s32SrcFrameRate    = -1;
    attr.attr_inp.stFrameRate.s32DstFrameRate    = -1;
    attr.attr_inp.enPixelFormat                  = pixel_format_in;
    attr.attr_inp.u32MaxW                        = width_in;
    attr.attr_inp.u32MaxH                        = height_in;
    attr.attr_inp.u8VpssDev                   = 0;

    attr.chn_num = 2;

    attr.attr_chn[0].u32Width                    = width_out0;
    attr.attr_chn[0].u32Height                   = height_out0;
    attr.attr_chn[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[0].enPixelFormat               = pixel_format_out0;
    attr.attr_chn[0].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[0].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[0].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[0].bMirror                     = CVI_FALSE;
    attr.attr_chn[0].bFlip                       = CVI_FALSE;
    attr.attr_chn[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[0].stNormalize.bEnable         = CVI_FALSE;

    attr.attr_chn[1].u32Width                    = width_out1;
    attr.attr_chn[1].u32Height                   = height_out1;
    attr.attr_chn[1].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[1].enPixelFormat               = pixel_format_out1;
    attr.attr_chn[1].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[1].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[1].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[1].bMirror                     = CVI_FALSE;
    attr.attr_chn[1].bFlip                       = CVI_FALSE;
    attr.attr_chn[1].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[1].stNormalize.bEnable         = CVI_FALSE;

    return attr;
}

static inline CVI_MAPI_VPROC_ATTR_T CVI_MAPI_VPROC_DefaultAttr_ThreeChn(
    uint32_t          width_in,
    uint32_t          height_in,
    PIXEL_FORMAT_E    pixel_format_in,
    uint32_t          width_out0,
    uint32_t          height_out0,
    PIXEL_FORMAT_E    pixel_format_out0,
    uint32_t          width_out1,
    uint32_t          height_out1,
    PIXEL_FORMAT_E    pixel_format_out1,
    uint32_t          width_out2,
    uint32_t          height_out2,
    PIXEL_FORMAT_E    pixel_format_out2)
{
    CVI_MAPI_VPROC_ATTR_T attr;
    memset((void*)&attr, 0, sizeof(attr));

    attr.attr_inp.stFrameRate.s32SrcFrameRate    = -1;
    attr.attr_inp.stFrameRate.s32DstFrameRate    = -1;
    attr.attr_inp.enPixelFormat                  = pixel_format_in;
    attr.attr_inp.u32MaxW                        = width_in;
    attr.attr_inp.u32MaxH                        = height_in;
    attr.attr_inp.u8VpssDev                   = 0;

    attr.chn_num = 3;

    attr.attr_chn[0].u32Width                    = width_out0;
    attr.attr_chn[0].u32Height                   = height_out0;
    attr.attr_chn[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[0].enPixelFormat               = pixel_format_out0;
    attr.attr_chn[0].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[0].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[0].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[0].bMirror                     = CVI_FALSE;
    attr.attr_chn[0].bFlip                       = CVI_FALSE;
    attr.attr_chn[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[0].stNormalize.bEnable         = CVI_FALSE;

    attr.attr_chn[1].u32Width                    = width_out1;
    attr.attr_chn[1].u32Height                   = height_out1;
    attr.attr_chn[1].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[1].enPixelFormat               = pixel_format_out1;
    attr.attr_chn[1].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[1].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[1].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[1].bMirror                     = CVI_FALSE;
    attr.attr_chn[1].bFlip                       = CVI_FALSE;
    attr.attr_chn[1].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[1].stNormalize.bEnable         = CVI_FALSE;

// channel[2]for venc
    attr.attr_chn[2].u32Width                    = width_out2;
    attr.attr_chn[2].u32Height                   = height_out2;
    attr.attr_chn[2].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[2].enPixelFormat               = pixel_format_out2;
    attr.attr_chn[2].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[2].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[2].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[2].bMirror                     = CVI_FALSE;
    attr.attr_chn[2].bFlip                       = CVI_FALSE;
    attr.attr_chn[2].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[2].stNormalize.bEnable         = CVI_FALSE;

    return attr;
}

// grp_id : 0 ~ 15, use -1 for auto select
int CVI_MAPI_VPROC_Init(CVI_MAPI_VPROC_HANDLE_T *vproc_hdl,
        int grp_id, CVI_MAPI_VPROC_ATTR_T *attr);
int CVI_MAPI_VPROC_Deinit(CVI_MAPI_VPROC_HANDLE_T vproc_hdl);

int CVI_MAPI_VPROC_ExtChnStop(CVI_MAPI_VPROC_HANDLE_T vproc_hdl);

int CVI_MAPI_VPROC_GetGrp(CVI_MAPI_VPROC_HANDLE_T vproc_hdl);

int CVI_MAPI_VPROC_GetGrpAttr(CVI_MAPI_VPROC_HANDLE_T vproc_hdl, VPSS_GRP_ATTR_S *stGrpAttr);

int CVI_MAPI_VPROC_SetGrpAttr(CVI_MAPI_VPROC_HANDLE_T vproc_hdl, VPSS_GRP_ATTR_S *stGrpAttr);

int CVI_MAPI_VPROC_BindVcap(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
        CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, uint32_t vichn_idx);
int CVI_MAPI_VPROC_UnBindVcap(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
        CVI_MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, uint32_t vichn_idx);

int CVI_MAPI_VPROC_BindVproc(CVI_MAPI_VPROC_HANDLE_T vproc_src_hdl,
        uint32_t chn_idx, CVI_MAPI_VPROC_HANDLE_T vproc_dest_hdl);

int CVI_MAPI_VPROC_UnBindVproc(CVI_MAPI_VPROC_HANDLE_T vproc_src_hdl,
        uint32_t chn_idx, CVI_MAPI_VPROC_HANDLE_T vproc_dest_hdl);

int CVI_MAPI_VPROC_GetChnAttr(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
                uint32_t chn_idx, VPSS_CHN_ATTR_S *pstChnAttr);

int CVI_MAPI_VPROC_SetChnAttr(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
                uint32_t chn_idx, VPSS_CHN_ATTR_S *pstChnAttr);

int CVI_MAPI_VPROC_GetChnAttrEx(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,uint32_t chn_idx,
            CVI_MAPI_VPROC_CMD_T enCMD, void *pAttr, uint32_t u32Len);

int CVI_MAPI_VPROC_SetChnAttrEx(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,uint32_t chn_idx,
            CVI_MAPI_VPROC_CMD_T enCMD, void *pAttr, uint32_t u32Len);

int CVI_MAPI_VPROC_SendFrame(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
        VIDEO_FRAME_INFO_S *frame);
int CVI_MAPI_VPROC_GetChnFrame(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, VIDEO_FRAME_INFO_S *frame);
int CVI_MAPI_VPROC_SendChnFrame(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, VIDEO_FRAME_INFO_S *frame);

// Deprecated, use CVI_MAPI_ReleaseFrame instead
int CVI_MAPI_VPROC_ReleaseFrame(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, VIDEO_FRAME_INFO_S *frame);

int CVI_MAPI_VPROC_StartChnDump(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, int s32Count, CVI_DUMP_FRAME_CALLBACK_FUNC_T *pstCallbackFun);

int CVI_MAPI_VPROC_StopChnDump(CVI_MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx);

int CVI_MAPI_VPROC_SetExtChnAttr(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
        CVI_MAPI_EXTCHN_ATTR_T *pstExtChnAttr);
int CVI_MAPI_VPROC_GetExtChnAttr(CVI_MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, CVI_MAPI_EXTCHN_ATTR_T *pstExtChnAttr);

int CVI_MAPI_VPROC_GetExtChnGrp(CVI_MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx);
int CVI_MAPI_VPROC_IsExtChn(CVI_MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx);
int CVI_MAPI_VPROC_EnableTileMode(void);
int CVI_MAPI_VPROC_DisableTileMode(void);
#ifdef __cplusplus
}
#endif

#endif
