#pragma once

extern "C"
{
    #include <libavcodec/avcodec.h>
}

namespace cvi_player {

class FlushPacket {
private:
    FlushPacket();

    FlushPacket(const FlushPacket &packet) = delete;
    void operator=(const FlushPacket &packet) = delete;

public:
    static AVPacket &getInstance();

private:
    static AVPacket packet;
};

} // namespace cvi_player
