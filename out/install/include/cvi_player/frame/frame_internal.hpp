#pragma once

#include <cstdint>

extern "C"
{
    #include <libavformat/avformat.h>
}

namespace cvi_player {

struct Frame
{
    AVFrame *frame{nullptr};
    AVSubtitle subtitle;
    int32_t serial{-1};
    double pts{0};           /* presentation timestamp for the frame */
    double duration{0};      /* estimated duration of the frame */
    int64_t pos{0};          /* byte position of the frame in the input file */
    int32_t width{0};
    int32_t height{0};
    int32_t format;
};

} // namespace cvi_player
