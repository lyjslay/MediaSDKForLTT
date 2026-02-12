#pragma once

#include "cvi_mapi.h"
#include "cvi_comm_video.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* CVI_IMAGE_VIEWER_HANDLE_T;
typedef void (*CVI_IMAGE_VIEWER_OUTPUT_HANDLER)(VIDEO_FRAME_INFO_S *);
typedef void (*CVI_IMAGE_VIEWER_CUSTOM_ARG_OUTPUT_HANDLER)(void *,
    VIDEO_FRAME_INFO_S *);

int32_t CVI_IMAGE_VIEWER_Create(CVI_IMAGE_VIEWER_HANDLE_T *handle);
int32_t CVI_IMAGE_VIEWER_Destroy(CVI_IMAGE_VIEWER_HANDLE_T *handle);
int32_t CVI_IMAGE_VIEWER_SetDecodeHandle(CVI_IMAGE_VIEWER_HANDLE_T handle,
        CVI_MAPI_VDEC_HANDLE_T decode_handle);
int32_t CVI_IMAGE_VIEWER_SetDisplayHandle(CVI_IMAGE_VIEWER_HANDLE_T handle,
        CVI_MAPI_DISP_HANDLE_T display_handle);
int32_t CVI_IMAGE_VIEWER_DisplayThumbnail(CVI_IMAGE_VIEWER_HANDLE_T handle, const char *input,
        POINT_S pos, SIZE_S size);
int32_t CVI_IMAGE_VIEWER_SetOutputHandler(CVI_IMAGE_VIEWER_HANDLE_T handle,
        CVI_IMAGE_VIEWER_OUTPUT_HANDLER handler);
int32_t CVI_IMAGE_VIEWER_SetCustomArgOutputHandler(CVI_IMAGE_VIEWER_HANDLE_T handle,
        CVI_IMAGE_VIEWER_CUSTOM_ARG_OUTPUT_HANDLER handler, void *custom_arg);

#ifdef __cplusplus
}
#endif
