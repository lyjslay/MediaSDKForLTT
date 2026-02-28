#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "cvi_demuxer/packet.h"
#include "cvi_demuxer/media_info.h"
#include "event/event.h"
#include "stream/audio_parameters.h"
#include "stream/video_parameters.h"
#include "player/play_info.h"
#include "frame/frame.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef CVI_DEMUXER_PACKET_S CVI_PLAYER_PACKET_S;
typedef CVI_DEMUXER_MEDIA_INFO_S CVI_PLAYER_MEDIA_INFO_S;
typedef void* CVI_PLAYER_HANDLE_T;
// handlers signature
typedef void (*CVI_PLAYER_OUTPUT_HANDLER)(CVI_PLAYER_FRAME_S *);
typedef void (*CVI_PLAYER_CUSTOM_ARG_OUTPUT_HANDLER)(void *,
        CVI_PLAYER_FRAME_S *);
typedef void (*CVI_PLAYER_EVENT_HANDLER)(CVI_PLAYER_EVENT_S *);
typedef void (*CVI_PLAYER_CUSTOM_ARG_EVENT_HANDLER)(void *,
        CVI_PLAYER_EVENT_S *);
typedef struct {
    int32_t (*get_frame)(CVI_PLAYER_FRAME_S *);
    int32_t (*decode_packet)(CVI_PLAYER_PACKET_S *);
} CVI_PLAYER_DECODE_HANDLER_S;
typedef struct {
    int32_t (*get_frame)(void *, CVI_PLAYER_FRAME_S *);
    int32_t (*decode_packet)(void *, CVI_PLAYER_PACKET_S *);
} CVI_PLAYER_CUSTOM_ARG_DECODE_HANDLER_S;

int32_t CVI_PLAYER_Init();
int32_t CVI_PLAYER_Deinit();
int32_t CVI_PLAYER_Create(CVI_PLAYER_HANDLE_T *handle);
int32_t CVI_PLAYER_Destroy(CVI_PLAYER_HANDLE_T *handle);
int32_t CVI_PLAYER_SetDataSource(CVI_PLAYER_HANDLE_T handle,
        const char *data_source);
int32_t CVI_PLAYER_GetDataSource(CVI_PLAYER_HANDLE_T handle,
        char *data_source);
int32_t CVI_PLAYER_LightOpen(CVI_PLAYER_HANDLE_T handle);
int32_t CVI_PLAYER_Play(CVI_PLAYER_HANDLE_T handle);
int32_t CVI_PLAYER_Stop(CVI_PLAYER_HANDLE_T handle);
int32_t CVI_PLAYER_Pause(CVI_PLAYER_HANDLE_T handle);
int32_t CVI_PLAYER_Resume(CVI_PLAYER_HANDLE_T handle);
int32_t CVI_PLAYER_Seek(CVI_PLAYER_HANDLE_T handle, int64_t time_in_ms);
int32_t CVI_PLAYER_TPlay(CVI_PLAYER_HANDLE_T handle, double speed);
int32_t CVI_PLAYER_SetAudioParameters(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_AUDIO_PARAMETERS parameters);
int32_t CVI_PLAYER_SetVideoParameters(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_VIDEO_PARAMETERS parameters);
int32_t CVI_PLAYER_SetAOHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_OUTPUT_HANDLER handler);
int32_t CVI_PLAYER_SetCustomArgAOHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_CUSTOM_ARG_OUTPUT_HANDLER handler, void *custom_arg);
int32_t CVI_PLAYER_SetVOHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_OUTPUT_HANDLER handler);
int32_t CVI_PLAYER_SetCustomArgVOHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_CUSTOM_ARG_OUTPUT_HANDLER handler, void *custom_arg);
int32_t CVI_PLAYER_SetEventHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_EVENT_HANDLER handler);
int32_t CVI_PLAYER_SetCustomArgEventHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_CUSTOM_ARG_EVENT_HANDLER handler, void *custom_arg);
int32_t CVI_PLAYER_SaveImage(CVI_PLAYER_HANDLE_T handle,
        const char *file_path);
int32_t CVI_PLAYER_GetMediaInfo(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_MEDIA_INFO_S *info);
int32_t CVI_PLAYER_GetPlayInfo(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_PLAY_INFO *info);
int32_t CVI_PLAYER_GetVideoFrame(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_FRAME_S *frame);
int32_t CVI_PLAYER_GetVideoPacket(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_PACKET_S *packet);
int32_t CVI_PLAYER_GetVideoExtraPacket(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_PACKET_S *packet);
int32_t CVI_PLAYER_SetVideoDecodeHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_DECODE_HANDLER_S handler);
int32_t CVI_PLAYER_SetVideoCustomArgDecodeHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_CUSTOM_ARG_DECODE_HANDLER_S handler, void *custom_arg);
int32_t CVI_PLAYER_SetAudioDecodeHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_DECODE_HANDLER_S handler);
int32_t CVI_PLAYER_SetAudioCustomArgDecodeHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_CUSTOM_ARG_DECODE_HANDLER_S handler, void *custom_arg);
bool CVI_PLAYER_PacketContainSps(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_PACKET_S *packet);
int32_t CVI_PLAYER_SeekPause(CVI_PLAYER_HANDLE_T handle, int64_t time_in_ms);
int32_t CVI_PLAYER_SeekFlage();
int32_t CVI_PLAYER_SeekTime();
int32_t CVI_PLAYER_PlayerSeep(CVI_PLAYER_HANDLE_T handle, int32_t speed, int32_t backforward);
int32_t CVI_PLAYER_GetForWardBackWardStatus(CVI_PLAYER_HANDLE_T handle);
int32_t CVI_PLAYER_GetMediumVideoExtraPacket(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_PACKET_S *packet);
void CVI_PLAYER_SetPlaySubStreamFlag(CVI_PLAYER_HANDLE_T handle, bool subflag);
#ifdef __cplusplus
}
#endif
