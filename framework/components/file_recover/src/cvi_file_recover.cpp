#include "cvi_file_recover/cvi_file_recover.h"
#include "file_recover.hpp"
#include "cvi_demuxer/utils/check.hpp"
#include "cvi_log.h"

using namespace cvi_file_recover;
using cvi_demuxer::utils::hasNullptr;
#define FILE_RECOVERY_MAJOR_VERSION 1
#define FILE_RECOVERY_MINOR_VERSION 4
int32_t CVI_FILE_RECOVER_Create(CVI_FILE_RECOVER_HANDLE_T *handle)
{
    if (!hasNullptr(*handle)) {
        CVI_LOGE("Handle is not null");
        return -1;
    }
    //CVI_LOGI("CVI File Recovery version : %d.%d",FILE_RECOVERY_MAJOR_VERSION, FILE_RECOVERY_MINOR_VERSION);
    *handle = new FileRecover();

    return 0;
}

int32_t CVI_FILE_RECOVER_Destroy(CVI_FILE_RECOVER_HANDLE_T *handle)
{
    if (hasNullptr(handle, *handle)) {
        CVI_LOGE("Handle is null");
        return -1;
    }

    FileRecover *file_recover = static_cast<FileRecover*>(*handle);
    delete file_recover;
    *handle = nullptr;

    return 0;
}

int32_t CVI_FILE_RECOVER_Open(CVI_FILE_RECOVER_HANDLE_T handle, const char* input_file_path)
{
    if (hasNullptr(handle, input_file_path)) {
        CVI_LOGE("Handle or file path is null");
        return -1;
    }

    FileRecover *file_recover = static_cast<FileRecover*>(handle);
    return file_recover->open(input_file_path);
}

int32_t CVI_FILE_RECOVER_Check(CVI_FILE_RECOVER_HANDLE_T handle)
{
    if (hasNullptr(handle)) {
        CVI_LOGE("Handle is null");
        return -1;
    }

    FileRecover *file_recover = static_cast<FileRecover *>(handle);
    return file_recover->check();
}

int32_t CVI_FILE_RECOVER_Dump(CVI_FILE_RECOVER_HANDLE_T handle)
{
    if (hasNullptr(handle)) {
        CVI_LOGE("Handle is null");
        return -1;
    }

    FileRecover *file_recover = static_cast<FileRecover*>(handle);
    return file_recover->dump();
}

void CVI_FILE_RECOVER_PreallocateState(CVI_FILE_RECOVER_HANDLE_T handle, bool PreallocFlage)
{
    if (hasNullptr(handle)) {
        CVI_LOGE("Handle or output file PreallocFlage is null");
        return;
    }

    FileRecover *file_recover = static_cast<FileRecover*>(handle);
    file_recover->preallocatestate(PreallocFlage);
}

int32_t CVI_FILE_RECOVER_Recover(CVI_FILE_RECOVER_HANDLE_T handle, const char* output_file_path, const char* device_model, bool has_create_time)
{
    if (hasNullptr(handle, output_file_path)) {
        CVI_LOGE("Handle or output file path is null");
        return -1;
    }

    FileRecover *file_recover = static_cast<FileRecover*>(handle);
    return file_recover->recover(output_file_path, device_model, has_create_time);
}

int32_t CVI_FILE_RECOVER_RecoverAsync(CVI_FILE_RECOVER_HANDLE_T handle, const char* output_file_path, const char* device_model, bool has_create_time)
{
    if (hasNullptr(handle, output_file_path)) {
        CVI_LOGE("Handle or output file path is null");
        return -1;
    }

    FileRecover *file_recover = static_cast<FileRecover*>(handle);
    return file_recover->recoverAsync(output_file_path, device_model, has_create_time);
}

int32_t CVI_FILE_RECOVER_RecoverJoin(CVI_FILE_RECOVER_HANDLE_T handle)
{
    if (hasNullptr(handle)) {
        CVI_LOGE("Handle is null");
        return -1;
    }

    FileRecover *file_recover = static_cast<FileRecover*>(handle);
    file_recover->join();

    return 0;
}

int32_t CVI_FILE_RECOVER_Close(CVI_FILE_RECOVER_HANDLE_T handle)
{
    if (hasNullptr(handle)) {
        CVI_LOGE("Handle is null");
        return -1;
    }

    FileRecover *file_recover = static_cast<FileRecover*>(handle);
    file_recover->close();

    return 0;
}

int32_t CVI_FILE_RECOVER_SetEventHandler(CVI_FILE_RECOVER_HANDLE_T handle,
        CVI_FILE_RECOVER_EVENT_HANDLER handler)
{
    if (hasNullptr(handle)) {
        CVI_LOGE("Handle is null");
        return -1;
    }

    FileRecover* file_recover = static_cast<FileRecover*>(handle);
    file_recover->setEventHandler(handler);

    return 0;
}

int32_t CVI_FILE_RECOVER_SetCustomArgEventHandler(CVI_FILE_RECOVER_HANDLE_T handle,
        CVI_FILE_RECOVER_CUSTOM_ARG_EVENT_HANDLER handler, void* custom_arg)
{
    if (hasNullptr(handle)) {
        CVI_LOGE("Handle is null");
        return -1;
    }

    FileRecover* file_recover = static_cast<FileRecover*>(handle);
    auto&& wrapper_handler = [custom_arg, handler](CVI_FILE_RECOVER_EVENT_S* event) {
        handler(custom_arg, event);
    };
    file_recover->setEventHandler(wrapper_handler);

    return 0;
}
