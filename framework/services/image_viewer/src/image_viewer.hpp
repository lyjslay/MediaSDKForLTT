#pragma once

#include <string>
#include <functional>
#include <memory>
#include "cvi_demuxer/packet.h"
#include "cvi_thumbnail_extractor/cvi_thumbnail_extractor.h"
#include "cvi_mapi.h"
#include "display_window.hpp"

namespace cvi_image_viewer {

class ImageViewer final
{
public:
    using OutputHandler = std::function<void(VIDEO_FRAME_INFO_S *)>;

    ~ImageViewer();

    void setDecodeHandle(CVI_MAPI_VDEC_HANDLE_T handle);
    void setDisplayHandle(CVI_MAPI_DISP_HANDLE_T handle);
    int32_t displayThumbnail(const std::string &input, const POINT_S &pos, const SIZE_S &size);
    void setOutputHandler(const OutputHandler &handler);
    void setOutputHandler(OutputHandler &&handler);

private:
    int32_t getThumbnailPacket(const std::string &input, CVI_THUMBNAIL_PACKET_S &packet);
    int32_t getJpegThumbnail(const std::string &input, CVI_THUMBNAIL_PACKET_S &thumbnail);
    int32_t decodePacketToYUVFrame(CviDemuxerPacket &src_packet, VIDEO_FRAME_INFO_S &dst_frame);
    int32_t resizeFrameToDisplaySize(VIDEO_FRAME_INFO_S &src_frame, VIDEO_FRAME_INFO_S &dst_frame);
    int32_t displayFrame(VIDEO_FRAME_INFO_S &frame, const POINT_S &pos, const SIZE_S &size);

private:
    CVI_THUMBNAIL_EXTRACTOR_HANDLE_T extractor_handle{nullptr};
    CVI_MAPI_VDEC_HANDLE_T vdec_handle{nullptr};
    CVI_MAPI_DISP_HANDLE_T display_handle{nullptr};
    std::unique_ptr<DisplayWindow> display_window;
    OutputHandler output_handler;
};

} // namespace cvi_image_viewer
