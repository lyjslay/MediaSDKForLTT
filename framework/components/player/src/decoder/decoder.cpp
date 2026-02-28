#include "cvi_player/decoder/decoder.hpp"
#include "cvi_player/packet/flush_packet.hpp"

namespace cvi_player {

using std::shared_ptr;
using std::condition_variable;

bool specialmedia = false;

Decoder::Decoder(AVCodecContext *avctx) :
avctx(avctx)
{}

Decoder::~Decoder()
{
    stop();
    flushBuffers();
    if (avctx != nullptr) {
        avcodec_free_context(&avctx);
    }
}

void Decoder::setspecialmediasign(bool sign)
{
    specialmedia = sign;
}

void Decoder::start()
{
    running = true;
    packet_queue.start();
    if (specialmedia) {
        getframe_thread = std::thread(&Decoder::getframe, this);
        decoder_thread = std::thread(&Decoder::decodespecial, this);
    } else {
        decoder_thread = std::thread(&Decoder::decode, this);
    }
}

void Decoder::stop()
{
    running = false;
    packet_queue.abort();
    frame_queue.abort();
    if (decoder_thread.joinable()) {
        decoder_thread.join();
    }

    if (specialmedia) {
        if (getframe_thread.joinable()) {
            getframe_thread.join();
        }
    }
}

bool Decoder::isFinished() const
{
    return (finished_serial == packet_queue.getSerial()) &&
           (frame_queue.getSize() == 0);
}

void Decoder::setStartPts(int64_t start_pts)
{
    this->start_pts = start_pts;
}

void Decoder::setStartPtsTimebase(AVRational start_pts_timebase)
{
    this->start_pts_timebase = std::move(start_pts_timebase);
}

void Decoder::setEmptyCV(shared_ptr<condition_variable> cv)
{
    empty_cv = std::move(cv);
}

void Decoder::flush()
{
    packet_queue.flush();
    packet_queue.put(&FlushPacket::getInstance());
}

PacketQueue &Decoder::getPacketQueue()
{
    return packet_queue;
}

FrameQueue &Decoder::getFrameQueue()
{
    return frame_queue;
}

void Decoder::flushBuffers() const
{
    if (avctx != nullptr) {
        avcodec_flush_buffers(avctx);
    }
}

int32_t Decoder::getPacket(AVPacket &packet)
{
    do {
        if (packet.data != nullptr) {
            av_packet_unref(&packet);
        }
        if (packet_queue.empty() && empty_cv) {
            // queue is empty, notify read more
            empty_cv->notify_one();
        }
        if (packet_pending) {
            av_packet_move_ref(&packet, &pending_packet);
            packet_pending = false;
        } else {
            if (packet_queue.pop(&packet, &packet_serial) < 0) {
                return -1;
            }
        }

        if (packet.data == FlushPacket::getInstance().data) {
            flushBuffers();
            finished_serial = 0;
            next_pts = start_pts;
            next_pts_timebase = start_pts_timebase;
        }
        // check eof null packet
        if (packet.data == nullptr) {
            return -1;
        }
    } while (packet_queue.getSerial() != packet_serial);

    return 0;
}

void Decoder::packetflush()
{
    packet_queue.flush();
}

} // namespace cvi_player
