#pragma once

#include <stdint.h>
#include "packet.h"
#include "media_info.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* CVI_DEMUXER_HANDLE_T;

int32_t CVI_DEMUXER_Create(CVI_DEMUXER_HANDLE_T *handle);
int32_t CVI_DEMUXER_Destroy(CVI_DEMUXER_HANDLE_T *handle);
int32_t CVI_DEMUXER_Open(CVI_DEMUXER_HANDLE_T handle);
int32_t CVI_DEMUXER_Close(CVI_DEMUXER_HANDLE_T handle);
int32_t CVI_DEMUXER_Pause(CVI_DEMUXER_HANDLE_T handle);
int32_t CVI_DEMUXER_Resume(CVI_DEMUXER_HANDLE_T handle);
int32_t CVI_DEMUXER_SetInput(CVI_DEMUXER_HANDLE_T handle, const char *input);
int32_t CVI_DEMUXER_Read(CVI_DEMUXER_HANDLE_T handle, CVI_DEMUXER_PACKET_S *packet);
int32_t CVI_DEMUXER_Seek(CVI_DEMUXER_HANDLE_T handle, const int64_t time_in_ms);
int32_t CVI_DEMUXER_GetMediaInfo(CVI_DEMUXER_HANDLE_T handle, CVI_DEMUXER_MEDIA_INFO_S *info);

#ifdef __cplusplus
}
#endif
