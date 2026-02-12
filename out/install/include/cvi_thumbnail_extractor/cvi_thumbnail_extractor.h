#pragma once

#include <stdint.h>
#include "cvi_demuxer/packet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* CVI_THUMBNAIL_EXTRACTOR_HANDLE_T;
typedef CVI_DEMUXER_PACKET_S CVI_THUMBNAIL_PACKET_S;

int32_t CVI_THUMBNAIL_EXTRACTOR_Create(CVI_THUMBNAIL_EXTRACTOR_HANDLE_T *handle);
int32_t CVI_THUMBNAIL_EXTRACTOR_Destroy(CVI_THUMBNAIL_EXTRACTOR_HANDLE_T *handle);
int32_t CVI_THUMBNAIL_EXTRACTOR_GetThumbnail(CVI_THUMBNAIL_EXTRACTOR_HANDLE_T handle,
    const char *input, CVI_THUMBNAIL_PACKET_S *thumbnail);
int32_t CVI_THUMBNAIL_EXTRACTOR_ClearPacket(CVI_THUMBNAIL_PACKET_S *packet);

#ifdef __cplusplus
}
#endif
