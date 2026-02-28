#include "cvi_player/queue/packet_queue.hpp"
#include "cvi_player/packet/flush_packet.hpp"

extern "C"
{
    #include <libavutil/mem.h>
}

namespace cvi_player {

using std::shared_ptr;
using std::lock_guard;
using std::unique_lock;

PacketQueue::PacketQueue()
{
    serial = std::make_shared<int32_t>(0);
}

PacketQueue::~PacketQueue()
{
    abort();
    flush();
}

void PacketQueue::start()
{
    lock_guard<std::mutex> lock(mutex);
    abort_request = false;
    putPacket(&FlushPacket::getInstance());
}

void PacketQueue::flush()
{
    AVPacketNode *packet, *next_packet;
    lock_guard<std::mutex> lock(mutex);
    for (packet = first_pkt; packet != nullptr; packet = next_packet) {
        next_packet = packet->next;
        av_packet_unref(&packet->pkt);
        av_freep(&packet);
    }

    last_pkt = nullptr;
    first_pkt = nullptr;
    size = 0;
    bytes = 0;
    duration = 0;
}

void PacketQueue::abort()
{
    lock_guard<std::mutex> lock(mutex);
    abort_request = true;
    cv.notify_all();
}

bool PacketQueue::isAbort() const
{
    return abort_request;
}

bool PacketQueue::empty() const
{
    return (size == 0);
}

int32_t PacketQueue::put(AVPacket *pkt)
{
    if (pkt == nullptr) {
        return -1;
    }

    lock_guard<std::mutex> lock(mutex);
    int32_t ret = putPacket(pkt);
    if ((pkt != &FlushPacket::getInstance()) && (ret < 0)) {
        av_packet_unref(pkt);
    }

    return ret;
}

int32_t PacketQueue::putNullPacket(const int32_t stream_index)
{
    AVPacket packet_node, *pkt = &packet_node;
    av_init_packet(pkt);
    pkt->data = nullptr;
    pkt->size = 0;
    pkt->stream_index = stream_index;
    return put(pkt);
}

int32_t PacketQueue::pop(AVPacket *pkt, int32_t *serial)
{
    std::unique_lock<std::mutex> lock(mutex);
    while((!abort_request) && empty()) {
        cv.wait(lock);
    }

    if (abort_request) {
        return -1;
    }

    AVPacketNode *packet_node = this->first_pkt;
    if (!packet_node) {
        return -1;
    }

    this->first_pkt = packet_node->next;
    if (this->first_pkt == nullptr) {
        this->last_pkt = nullptr;
    }
    this->size--;
    this->bytes -= packet_node->pkt.size + sizeof(*packet_node);
    this->duration -= packet_node->pkt.duration;
    av_packet_move_ref(pkt, &packet_node->pkt);
    if (serial != nullptr) {
        *serial = packet_node->serial;
    }
    av_freep(&packet_node);

    return 0;
}

int32_t PacketQueue::getSerial() const
{
    return *serial;
}

shared_ptr<int32_t> PacketQueue::getSharedSerial() const
{
    return serial;
}

int32_t PacketQueue::getSize() const
{
    return size;
}

uint64_t PacketQueue::getTotalBytes() const
{
    return bytes;
}

int64_t PacketQueue::getTotalDuration() const
{
    return duration;
}

AVPacketNode *PacketQueue::getLastPacketNode() const
{
    return last_pkt;
}

int32_t PacketQueue::putPacket(AVPacket *pkt)
{
    if (this->abort_request) {
        av_packet_unref(pkt);
        return -1;
    }

    AVPacketNode *packet_node = static_cast<AVPacketNode *>(av_malloc(sizeof(AVPacketNode)));
    if (packet_node == nullptr) {
        av_packet_unref(pkt);
        return -1;
    }
    av_packet_move_ref(&packet_node->pkt, pkt);
    packet_node->next = nullptr;
    packet_node->serial = (*this->serial);
    if (pkt == &FlushPacket::getInstance()) {
        (*this->serial)++;
    }

    if (this->last_pkt == nullptr) {
        this->first_pkt = packet_node;
    } else {
        this->last_pkt->next = packet_node;
    }
    this->last_pkt = packet_node;
    this->size++;
    this->bytes += (packet_node->pkt.size + sizeof(*packet_node));
    this->duration += packet_node->pkt.duration;

    this->cv.notify_one();

    return 0;
}

} // namespace cvi_player
