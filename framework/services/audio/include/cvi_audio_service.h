#pragma once
#include <cvi_mapi.h>
#include <cvi_mapi_aenc.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef void (CVI_AUDIO_SERVICR_ACAP_GET_FRAME_CALLBACK) (const AUDIO_FRAME_S *frame, const AEC_FRAME_S *aec_frame, void *arg);
typedef void (CVI_AUDIO_SERVICR_ACAP_GET_AAC_FRAME_CALLBACK) (const AUDIO_STREAM_S *stream, void *arg);

int32_t CVI_AUDIO_SERVICR_ACAP_CallbackSet(const char *name, CVI_AUDIO_SERVICR_ACAP_GET_FRAME_CALLBACK *cb, void *arg);
int32_t CVI_AUDIO_SERVICR_ACAP_CallbackUnset(const char *name);
int32_t CVI_AUDIO_SERVICR_ACAP_AacCallbackSet(const char *name, CVI_AUDIO_SERVICR_ACAP_GET_AAC_FRAME_CALLBACK *cb, void *arg);
int32_t CVI_AUDIO_SERVICR_ACAP_AacCallbackUnset(const char *name);
int32_t CVI_AUDIO_SERVICR_ACAP_TaskStart(CVI_MAPI_ACAP_HANDLE_T acap_handle, CVI_MAPI_AENC_HANDLE_T aenc_handle);
int32_t CVI_AUDIO_SERVICR_ACAP_TaskStop();

#ifdef __cplusplus
}
#endif