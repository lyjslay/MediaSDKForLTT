#pragma once

extern "C"
{
    #include <libavcodec/avcodec.h>
}

namespace cvi_demuxer {
namespace utils {

inline bool isH26X(AVCodecID codec)
{
    return (codec == AV_CODEC_ID_H264) || (codec == AV_CODEC_ID_HEVC);
}

} // namespace utils
} // namespace cvi_demuxer
