#include "cvi_demuxer/cvi_demuxer.h"
#include "cvi_demuxer/ffmpeg_demuxer.hpp"
#include "cvi_demuxer/utils/check.hpp"
#include "cvi_demuxer/utils/packet.hpp"
#include "cvi_log.h"

using namespace cvi_demuxer;

static AVPacket buffer_packet;

int32_t CVI_DEMUXER_Create(CVI_DEMUXER_HANDLE_T *handle)
{
    if (!utils::hasNullptr(*handle)) {
        CVI_LOGE("Demuxer is not null");
        return -1;
    }

    *handle = new FFmpegDemuxer();

    return 0;
}

int32_t CVI_DEMUXER_Destroy(CVI_DEMUXER_HANDLE_T *handle)
{
    if (utils::hasNullptr(handle, *handle)) {
        CVI_LOGE("Demuxer is null");
        return -1;
    }

    if (buffer_packet.buf != nullptr) {
        av_packet_unref(&buffer_packet);
    }

    FFmpegDemuxer *demuxer = static_cast<FFmpegDemuxer *>(*handle);
    delete demuxer;
    *handle = nullptr;

    return 0;
}

int32_t CVI_DEMUXER_Open(CVI_DEMUXER_HANDLE_T handle)
{
    if (utils::hasNullptr(handle)) {
        CVI_LOGE("Demuxer is null");
        return -1;
    }

    FFmpegDemuxer *demuxer = static_cast<FFmpegDemuxer *>(handle);
    return demuxer->open();
}

int32_t CVI_DEMUXER_Close(CVI_DEMUXER_HANDLE_T handle)
{
    if (utils::hasNullptr(handle)) {
        CVI_LOGE("Demuxer is null");
        return -1;
    }

    FFmpegDemuxer *demuxer = static_cast<FFmpegDemuxer *>(handle);
    demuxer->close();

    return 0;
}

int32_t CVI_DEMUXER_Pause(CVI_DEMUXER_HANDLE_T handle)
{
    if (utils::hasNullptr(handle)) {
        CVI_LOGE("Demuxer is null");
        return -1;
    }

    FFmpegDemuxer *demuxer = static_cast<FFmpegDemuxer *>(handle);
    demuxer->pause();

    return 0;
}

int32_t CVI_DEMUXER_Resume(CVI_DEMUXER_HANDLE_T handle)
{
    if (utils::hasNullptr(handle)) {
        CVI_LOGE("Demuxer is null");
        return -1;
    }

    FFmpegDemuxer *demuxer = static_cast<FFmpegDemuxer *>(handle);
    demuxer->resume();

    return 0;
}

int32_t CVI_DEMUXER_SetInput(CVI_DEMUXER_HANDLE_T handle, const char *input)
{
    if (utils::hasNullptr(handle)) {
        CVI_LOGE("Demuxer is null");
        return -1;
    }

    FFmpegDemuxer *demuxer = static_cast<FFmpegDemuxer *>(handle);
    demuxer->setInput(input);

    return 0;
}

int32_t CVI_DEMUXER_Read(CVI_DEMUXER_HANDLE_T handle, CVI_DEMUXER_PACKET_S *packet)
{
    if (utils::hasNullptr(handle)) {
        CVI_LOGE("Demuxer is null");
        return -1;
    }

    FFmpegDemuxer *demuxer = static_cast<FFmpegDemuxer *>(handle);
    if (demuxer->isPaused()) {
        utils::shallowCopyPacket(&buffer_packet, packet);
        return 0;
    }

    if (buffer_packet.buf != nullptr) {
        av_packet_unref(&buffer_packet);
    }

    int32_t ret = demuxer->read(&buffer_packet);
    if (ret != 0) {
        CVI_LOGE("Read failed :%d", ret);
        return ret;
    }

    utils::shallowCopyPacket(&buffer_packet, packet);

    return 0;
}

int32_t CVI_DEMUXER_Seek(CVI_DEMUXER_HANDLE_T handle, const int64_t time_in_ms)
{
    if (utils::hasNullptr(handle)) {
        CVI_LOGE("Demuxer is null");
        return -1;
    }

    FFmpegDemuxer *demuxer = static_cast<FFmpegDemuxer *>(handle);
    return demuxer->seek(time_in_ms*1000);
}

int32_t CVI_DEMUXER_GetMediaInfo(CVI_DEMUXER_HANDLE_T handle, CVI_DEMUXER_MEDIA_INFO_S *info)
{
     if (utils::hasNullptr(handle, info)) {
        CVI_LOGE("Demuxer or info is null");
        return -1;
    }

    FFmpegDemuxer *demuxer = static_cast<FFmpegDemuxer *>(handle);
    return demuxer->getMediaInfo(*info);
}
