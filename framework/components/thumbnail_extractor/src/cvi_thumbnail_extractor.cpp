#include "cvi_thumbnail_extractor/cvi_thumbnail_extractor.h"
#include "cvi_demuxer/utils/check.hpp"
#include "cvi_demuxer/utils/file.hpp"
#include "cvi_log.h"
#include "thumbnail_extractor.hpp"
#ifdef COMPONENTS_FILE_RECOVER_ON
#include "cvi_file_recover/cvi_file_recover.h"
#endif

using namespace cvi_thumbnail_extractor;

int32_t CVI_THUMBNAIL_EXTRACTOR_Create(CVI_THUMBNAIL_EXTRACTOR_HANDLE_T *handle)
{
    if (!cvi_demuxer::utils::hasNullptr(*handle)) {
        CVI_LOGE("Extractor is not null");
        return -1;
    }

    *handle = new ThumbnailExtractor();

    return 0;
}

int32_t CVI_THUMBNAIL_EXTRACTOR_Destroy(CVI_THUMBNAIL_EXTRACTOR_HANDLE_T *handle)
{
    if (cvi_demuxer::utils::hasNullptr(handle, *handle)) {
        CVI_LOGF("Extractor is null");
        return -1;
    }

    ThumbnailExtractor *extractor = static_cast<ThumbnailExtractor *>(*handle);
    delete extractor;
    *handle = nullptr;

    return 0;
}

static int32_t CVI_THUMBNAIL_EXTRACTOR_File_Recoder(const char* file_path)
{
    int32_t ret = 0;
#if defined(COMPONENTS_FILE_RECOVER_ON) && !defined(FLUSH_MOOV_STREAM_ON)
    CVI_FILE_RECOVER_HANDLE_T handle = NULL;

    ret = CVI_FILE_RECOVER_Create(&handle);
    if (ret != 0) {
        CVI_LOGE("Thumbnail:CVI_FILE_RECOVER_Create failed");
        goto FUNC_OUT;
    }

    ret = CVI_FILE_RECOVER_Open(handle, file_path);
    if (ret != 0) {
        CVI_LOGE("Thumbnail:CVI_FILE_RECOVER_Open failed");
        goto FUNC_OUT;
    }

    ret = CVI_FILE_RECOVER_Recover(handle, file_path, "", false);
    if (ret != 0) {
        CVI_LOGE("Thumbnail:CVI_FILE_RECOVER_Recover failed");
    }

FUNC_OUT:
    CVI_FILE_RECOVER_Destroy(&handle);
#endif

    return ret;
}

int32_t CVI_THUMBNAIL_EXTRACTOR_GetThumbnail(CVI_THUMBNAIL_EXTRACTOR_HANDLE_T handle,
    const char *input, CVI_THUMBNAIL_PACKET_S *thumbnail)
{
    int32_t ret = 0;
    if (cvi_demuxer::utils::hasNullptr(handle, input, thumbnail)) {
        CVI_LOGF("Extractor or input or thumbnail is null");
        ret = -1;
        return ret;
    }

    ThumbnailExtractor *extractor = static_cast<ThumbnailExtractor *>(handle);
    ret = extractor->getThumbnail(input, *thumbnail);
    if (ret != 0) {
        if (cvi_demuxer::utils::isJpegFile(input)) {
            return ret;
        } else {
            if (ret == 1) {
                CVI_LOGI("Failed to get thumbnail file. repair the file");
                ret = CVI_THUMBNAIL_EXTRACTOR_File_Recoder(input);
                extractor->getThumbnail(input, *thumbnail);
            } else {
                return ret;
            }
        }
    }

    return 0;
}

int32_t CVI_THUMBNAIL_EXTRACTOR_ClearPacket(CVI_THUMBNAIL_PACKET_S *packet)
{
     if (cvi_demuxer::utils::hasNullptr(packet)) {
        CVI_LOGF("Packet is null");
        return -1;
    }

    if (packet->data != nullptr) {
        delete [] packet->data;
        packet->data = nullptr;
    }
    packet->pts = 0;
    packet->size = 0;

    return 0;
}
