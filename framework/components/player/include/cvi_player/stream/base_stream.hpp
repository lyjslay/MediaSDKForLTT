#pragma once

#include <atomic>
#include <condition_variable>
#include <thread>
#include <memory>
#include "cvi_player/decoder/decoder.hpp"
#include "cvi_player/frame/frame.h"
#include "cvi_player/queue/frame_queue.hpp"
#include "cvi_player/queue/packet_queue.hpp"
#include "cvi_player/types.hpp"

extern "C"
{
    #include <libavformat/avformat.h>
}

namespace cvi_player {

constexpr int32_t DEFAULT_FRAMERATE = 25;

class BaseStream
{
public:
    BaseStream(int32_t index, AVStream *stream);
    virtual ~BaseStream();

    void open();
    void close();
    CVI_ERROR getFrame(CviPlayerFrame *player_frame);
    int32_t getIndex() const;
    double getDuration() const;
    void flush();
    void setEmptyCV(std::shared_ptr<std::condition_variable> cv);
    bool isEmpty() const;
    bool hasAttachedPicture() const;
    AVPacket &getAttachedPacket() const;
    bool hasEnoughPackets() const;
    PacketQueue &getPacketQueue();
    void putNullPacket();
    virtual bool isValidPacket(const AVPacket& packet) const;
    double getRatetime();
    void packetflush();

protected:
    virtual void prepare();
    void refreshLoop();
    virtual bool needRefresh() const;
    virtual void refresh(double &remaining_time);
    AVCodecContext *createCodecContext();

private:
    void setRefreshRate(double rate);

protected:
    AVStream *stream{nullptr};
    std::atomic<bool> running{false};
    std::unique_ptr<Decoder> decoder;
    std::thread refresh_thread;
    double refresh_rate{av_q2d(AVRational{1, DEFAULT_FRAMERATE})};

private:
    int32_t index{-1};
};

} // namespace cvi_player
