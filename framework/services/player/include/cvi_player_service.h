#ifndef __CVI_PLAYER_SERVICE_H__
#define __CVI_PLAYER_SERVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cvi_mapi.h"
#include "cvi_mapi_ao.h"
#include "cvi_player/cvi_player.h"
#include "cvi_signal_slot/cvi_signal_slot.h"

typedef struct {
    int32_t chn_id;
    bool repeat;
    // handle
    CVI_MAPI_DISP_HANDLE_T disp;
    CVI_MAPI_AO_HANDLE_T ao;
    // display
    int32_t disp_id;
    PIXEL_FORMAT_E disp_fmt;
    ROTATION_E disp_rotate;
    ASPECT_RATIO_E disp_aspect_ratio;
    uint32_t x;
    uint32_t y;
    uint32_t width; // 0 for screen width
    uint32_t height; // 0 for screen height
    uint32_t SampleRate;
    uint32_t AudioChannel;
} CVI_PLAYER_SERVICE_PARAM_S;

typedef struct {
    CVI_SIGNAL_S play;
    CVI_SIGNAL_S pause;
    CVI_SIGNAL_S resume;
    CVI_SIGNAL_S finish;
} CVI_PLAYER_SERVICE_SIGNALS_S;

typedef struct {
    CVI_SLOT_S set_input;
    CVI_SLOT_S play;
    CVI_SLOT_S stop;
    CVI_SLOT_S pause;
    CVI_SLOT_S seek;
    CVI_SLOT_S resize;
    CVI_SLOT_S toggle_fullscreen;
    CVI_SLOT_S get_play_info;
} CVI_PLAYER_SERVICE_SLOTS_S;

typedef enum
{
    CVI_PLAYER_SERVICE_EVENT_UNKNOWN,
    CVI_PLAYER_SERVICE_EVENT_OPEN_FAILED,
    CVI_PLAYER_SERVICE_EVENT_PLAY,
    CVI_PLAYER_SERVICE_EVENT_PLAY_FINISHED,
    CVI_PLAYER_SERVICE_EVENT_PLAY_PROGRESS,
    CVI_PLAYER_SERVICE_EVENT_PAUSE,
    CVI_PLAYER_SERVICE_EVENT_RESUME,
    CVI_PLAYER_SERVICE_EVENT_RECOVER_START,
    CVI_PLAYER_SERVICE_EVENT_RECOVER_PROGRESS,
    CVI_PLAYER_SERVICE_EVENT_RECOVER_FAILED,
    CVI_PLAYER_SERVICE_EVENT_RECOVER_FINISHED,
} CVI_PLAYER_SERVICE_EVENT_TYPE_E;

typedef struct
{
    CVI_PLAYER_SERVICE_EVENT_TYPE_E type;
    double value;
} CVI_PLAYER_SERVICE_EVENT_S;

typedef void* CVI_PLAYER_SERVICE_HANDLE_T;

typedef void (*CVI_PLAYER_SERVICE_EVENT_HANDLER)(CVI_PLAYER_SERVICE_HANDLE_T,
    CVI_PLAYER_SERVICE_EVENT_S *);

#ifdef __cplusplus
extern "C" {
#endif

int32_t CVI_PLAYER_SERVICE_GetDefaultParam(CVI_PLAYER_SERVICE_PARAM_S *param);
int32_t CVI_PLAYER_SERVICE_Create(CVI_PLAYER_SERVICE_HANDLE_T *handle,
        CVI_PLAYER_SERVICE_PARAM_S *param);
int32_t CVI_PLAYER_SERVICE_Destroy(CVI_PLAYER_SERVICE_HANDLE_T *handle);
int32_t CVI_PLAYER_SERVICE_SetInput(CVI_PLAYER_SERVICE_HANDLE_T handle, const char *input);
int32_t CVI_PLAYER_SERVICE_GetMediaInfo(CVI_PLAYER_SERVICE_HANDLE_T handle, CVI_PLAYER_MEDIA_INFO_S *info);
int32_t CVI_PLAYER_SERVICE_GetPlayInfo(CVI_PLAYER_SERVICE_HANDLE_T handle, CVI_PLAYER_PLAY_INFO *info);
int32_t CVI_PLAYER_SERVICE_Play(CVI_PLAYER_SERVICE_HANDLE_T handle);
int32_t CVI_PLAYER_SERVICE_PlayerAndSeek(CVI_PLAYER_SERVICE_HANDLE_T handle, int64_t seektime);
int32_t CVI_PLAYER_SERVICE_Stop(CVI_PLAYER_SERVICE_HANDLE_T handle);
int32_t CVI_PLAYER_SERVICE_Pause(CVI_PLAYER_SERVICE_HANDLE_T handle);
int32_t CVI_PLAYER_SERVICE_Seek(CVI_PLAYER_SERVICE_HANDLE_T handle, int64_t time_in_ms);
int32_t CVI_PLAYER_SERVICE_SetEventHandler(CVI_PLAYER_SERVICE_HANDLE_T handle,
    CVI_PLAYER_SERVICE_EVENT_HANDLER handler);
int32_t CVI_PLAYER_SERVICE_Resize(CVI_PLAYER_SERVICE_HANDLE_T handle, uint32_t width, uint32_t height);
int32_t CVI_PLAYER_SERVICE_MoveTo(CVI_PLAYER_SERVICE_HANDLE_T handle, uint32_t x, uint32_t y);
int32_t CVI_PLAYER_SERVICE_ToggleFullscreen(CVI_PLAYER_SERVICE_HANDLE_T handle);
int32_t CVI_PLAYER_SERVICE_GetSignals(CVI_PLAYER_SERVICE_HANDLE_T handle, CVI_PLAYER_SERVICE_SIGNALS_S **signals);
int32_t CVI_PLAYER_SERVICE_GetSlots(CVI_PLAYER_SERVICE_HANDLE_T handle, CVI_PLAYER_SERVICE_SLOTS_S **slots);
int32_t CVI_PLAYER_SERVICE_SeekFlage();
int32_t CVI_PLAYER_SERVICE_SeekTime();
int32_t CVI_PLAYER_SERVICE_SeekPause(CVI_PLAYER_SERVICE_HANDLE_T handle, int64_t time_in_ms);
int32_t CVI_PLAYER_SERVICE_TouchSeekPause(CVI_PLAYER_SERVICE_HANDLE_T handle, int64_t time_in_ms);
int32_t CVI_PLAYER_SERVICE_PlayerSeep(CVI_PLAYER_SERVICE_HANDLE_T handle, int32_t speeds);
int32_t CVI_PLAYER_SERVICE_PlayerSeepBack(CVI_PLAYER_SERVICE_HANDLE_T handle, int32_t speeds);
int32_t CVI_PLAYER_SERVICE_GetFileMediaInfo(char *filepatch);
void CVI_PLAYER_SERVICE_SetPlaySubStreamFlag(CVI_PLAYER_SERVICE_HANDLE_T handle, bool subflag);

#ifdef __cplusplus
}
#endif

#endif // __CVI_PLAYER_SERVICE_H__
