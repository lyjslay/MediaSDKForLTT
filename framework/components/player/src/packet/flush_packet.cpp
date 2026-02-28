#include "cvi_player/packet/flush_packet.hpp"

namespace cvi_player {

AVPacket FlushPacket::packet;

FlushPacket::FlushPacket()
{
    av_init_packet(&packet);
    packet.data = (uint8_t *)&packet;
}

AVPacket &FlushPacket::getInstance()
{
    static FlushPacket instance;

    return instance.packet;
}

} // namespace cvi_player
