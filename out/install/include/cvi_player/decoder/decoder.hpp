#pragma once

#include <atomic>
#include <condition_variable>
#include <thread>
#include <memory>
#include "cvi_player/queue/frame_queue.hpp"
#include "cvi_player/queue/packet_queue.hpp"

extern "C"
{
    #include <libavcodec/avcodec.h>
}

namespace cvi_player {

class Decoder {
public:
    explicit Decoder(AVCodecContext *avctx);
    virtual ~Decoder();

    void start();
    void stop();
    bool isFinished() const;
    void setStartPts(int64_t start_pts);
    void setStartPtsTimebase(AVRational start_pts_timebase);
    void setEmptyCV(std::shared_ptr<std::condition_variable> cv);
    void flush();
    PacketQueue &getPacketQueue();
    FrameQueue &getFrameQueue();
    void packetflush();
    void setspecialmediasign(bool sign);

protected:
    virtual void decode() = 0;
    void flushBuffers() const;
    int32_t getPacket(AVPacket &packet);
    virtual void decodespecial() = 0;
    virtual void getframe() = 0;

protected:
    AVCodecContext *avctx{nullptr};
    std::atomic<bool> running{false};
    int32_t finished_serial{-1};
    int32_t packet_serial{-1};
    int64_t start_pts{AV_NOPTS_VALUE};
    int64_t next_pts{AV_NOPTS_VALUE};
    bool packet_pending{false};
    AVRational next_pts_timebase{0, 0};
    AVRational start_pts_timebase{0, 0};
    AVPacket pending_packet;
    std::shared_ptr<std::condition_variable> empty_cv;
    FrameQueue frame_queue;
    PacketQueue packet_queue;

private:
    std::thread decoder_thread;
    std::thread getframe_thread;
};

} // namespace cvi_player
