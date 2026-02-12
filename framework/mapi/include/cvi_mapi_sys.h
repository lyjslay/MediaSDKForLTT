#ifndef __CVI_MAPI_SYS_H__
#define __CVI_MAPI_SYS_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

#include "cvi_comm_video.h"
#include "cvi_comm_sys.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct CVI_MAPI_MEDIA_SYS_VB_POOL_S {
    union cvi_vb_blk_size {
        uint32_t                   size;
        struct cvi_vb_blk_frame_s {
            uint32_t             width;
            uint32_t             height;
            PIXEL_FORMAT_E       fmt;
        } frame;
    } vb_blk_size;
    bool                         is_frame;
    uint32_t                     vb_blk_num;
} CVI_MAPI_MEDIA_SYS_VB_POOL_T;

#define CVI_MAPI_VB_POOL_MAX_NUM (16)
typedef struct CVI_MAPI_MEDIA_SYS_ATTR_S {
    CVI_MAPI_MEDIA_SYS_VB_POOL_T vb_pool[CVI_MAPI_VB_POOL_MAX_NUM];
    uint32_t                     vb_pool_num;
    VI_VPSS_MODE_S stVIVPSSMode;
    VPSS_MODE_S stVPSSMode;
} CVI_MAPI_MEDIA_SYS_ATTR_T;

void CVI_MAPI_Media_PrintVersion(void);
int CVI_MAPI_Media_Init(CVI_MAPI_MEDIA_SYS_ATTR_T *attr);
int CVI_MAPI_Media_Deinit(void);

//
// VB Frame helper functions
//
int CVI_MAPI_ReleaseFrame(VIDEO_FRAME_INFO_S *frm);
int CVI_MAPI_AllocateFrame(VIDEO_FRAME_INFO_S *frm,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt);
int CVI_MAPI_AllocateFrame_ByPoolID(VB_POOL pool,VIDEO_FRAME_INFO_S *frm,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt);
int CVI_MAPI_GetFrameFromMemory_YUV(VIDEO_FRAME_INFO_S *frm,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt, void *data);
int CVI_MAPI_GetFrameFromFile_YUV(VIDEO_FRAME_INFO_S *frame,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt,
        const char *filaneme, uint32_t frame_no);
int CVI_MAPI_SaveFramePixelData(VIDEO_FRAME_INFO_S *frm, const char *name);

int CVI_MAPI_FrameMmap(VIDEO_FRAME_INFO_S *frm, bool enable_cache);
int CVI_MAPI_FrameMunmap(VIDEO_FRAME_INFO_S *frm);
int CVI_MAPI_FrameFlushCache(VIDEO_FRAME_INFO_S *frm);
int CVI_MAPI_FrameInvalidateCache(VIDEO_FRAME_INFO_S *frm);

int CVI_MAPI_CreateVbPool(PIXEL_FORMAT_E encode_type, int input_width, int input_height);
int CVI_MAPI_DestroyVbPool();

#ifdef __cplusplus
}
#endif

#endif
