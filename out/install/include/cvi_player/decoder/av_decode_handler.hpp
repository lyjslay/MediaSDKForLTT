#pragma once

#include <functional>

extern "C"
{
    #include <libavformat/avformat.h>
}

namespace cvi_player {

struct AvDecodeHandler
{
    template <typename T>
    using handle_function = std::function<int32_t(T)>;

    explicit operator bool() const {
        return get_frame || decode_packet;
    }

    handle_function<AVFrame *> get_frame{nullptr};
    handle_function<AVPacket *> decode_packet{nullptr};
};

} // namespace cvi_player
