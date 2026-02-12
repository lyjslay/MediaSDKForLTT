#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <cstdint>

extern "C"
{
    #include <libavcodec/avcodec.h>
}

namespace cvi_player {

struct AVPacketNode {
    AVPacket pkt;
    struct AVPacketNode *next;
    int32_t serial;
};

class PacketQueue {
public:
    PacketQueue();
    ~PacketQueue();

    void start();
    void flush();
    void abort();
    bool isAbort() const;
    bool empty() const;
    int32_t put(AVPacket *pkt);
    int32_t putNullPacket(const int32_t stream_index);
    int32_t pop(AVPacket *pkt, int32_t *serial);
    int32_t getSerial() const;
    std::shared_ptr<int32_t> getSharedSerial() const;
    int32_t getSize() const;
    uint64_t getTotalBytes() const;
    int64_t getTotalDuration() const;
    AVPacketNode *getLastPacketNode() const;

private:
    int32_t putPacket(AVPacket *pkt);

private:
    std::atomic<bool> abort_request{true};
    AVPacketNode *first_pkt{nullptr};
    AVPacketNode *last_pkt{nullptr};
    int32_t size{0};
    uint64_t bytes{0};
    int64_t duration{0};
    std::shared_ptr<int32_t> serial;
    mutable std::mutex mutex;
    std::condition_variable cv;
};

} // namespace cvi_player
