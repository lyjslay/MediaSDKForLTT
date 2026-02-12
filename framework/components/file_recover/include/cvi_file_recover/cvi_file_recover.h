#pragma once

#include <stdint.h>
#include "cvi_file_recover/event.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* CVI_FILE_RECOVER_HANDLE_T;
typedef void (*CVI_FILE_RECOVER_EVENT_HANDLER)(CVI_FILE_RECOVER_EVENT_S*);
typedef void (*CVI_FILE_RECOVER_CUSTOM_ARG_EVENT_HANDLER)(void*, CVI_FILE_RECOVER_EVENT_S*);

int32_t CVI_FILE_RECOVER_Create(CVI_FILE_RECOVER_HANDLE_T *handle);
int32_t CVI_FILE_RECOVER_Destroy(CVI_FILE_RECOVER_HANDLE_T *handle);
int32_t CVI_FILE_RECOVER_Open(CVI_FILE_RECOVER_HANDLE_T handle, const char* input_file_path);
int32_t CVI_FILE_RECOVER_Check(CVI_FILE_RECOVER_HANDLE_T handle);
int32_t CVI_FILE_RECOVER_Dump(CVI_FILE_RECOVER_HANDLE_T handle);
int32_t CVI_FILE_RECOVER_Recover(CVI_FILE_RECOVER_HANDLE_T handle, const char* output_file_path, const char* device_model, bool has_create_time);
int32_t CVI_FILE_RECOVER_RecoverAsync(CVI_FILE_RECOVER_HANDLE_T handle, const char* output_file_path, const char* device_model, bool has_create_time);
int32_t CVI_FILE_RECOVER_RecoverJoin(CVI_FILE_RECOVER_HANDLE_T handle);
int32_t CVI_FILE_RECOVER_Close(CVI_FILE_RECOVER_HANDLE_T handle);
int32_t CVI_FILE_RECOVER_SetEventHandler(CVI_FILE_RECOVER_HANDLE_T handle,
        CVI_FILE_RECOVER_EVENT_HANDLER handler);
int32_t CVI_FILE_RECOVER_SetCustomArgEventHandler(CVI_FILE_RECOVER_HANDLE_T handle,
        CVI_FILE_RECOVER_CUSTOM_ARG_EVENT_HANDLER handler, void* custom_arg);
void CVI_FILE_RECOVER_PreallocateState(CVI_FILE_RECOVER_HANDLE_T handle, bool PreallocFlage);

#ifdef __cplusplus
}
#endif
