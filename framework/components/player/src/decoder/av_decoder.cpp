#include <unistd.h>
#include "cvi_player/decoder/av_decoder.hpp"
#include "cvi_player/packet/flush_packet.hpp"
#include "cvi_player/utils/frame.hpp"
#include "cvi_log.h"

namespace cvi_player {

using std::shared_ptr;
extern bool Aacflag;

AvDecoder::AvDecoder(AVCodecContext *avctx)
: Decoder(avctx)
{}

void AvDecoder::decode()
{
    AVFrame *frame = av_frame_alloc();
    if (frame == nullptr) {
        return;
    }

    while (running) {
        int32_t got_frame = decodeFrame(frame);
        if (got_frame < 0) {
            CVI_LOGE("decode frame failed %d", got_frame);
            break;
        } else if (got_frame > 0) {
            if (enqueueFrame(frame) != CVI_ERROR::NONE) {
                CVI_LOGE("enqueue frame failed");
                break;
            }
        }
    }

    if (frame != nullptr) {
        utils::releaseFrameData(frame);
        av_frame_free(&frame);
        frame = nullptr;
    }
}

int32_t AvDecoder::decodeFrame(AVFrame *frame)
{
    thread_local int64_t decode_packet_pts = AV_NOPTS_VALUE;

    while (running) {
        if (packet_queue.getSerial() == this->packet_serial) {
            int32_t ret = AVERROR(EAGAIN);
            do {
                if (packet_queue.isAbort()) {
                    flushBuffers();
                    return 0;
                }

                if (handler.get_frame) {
                    // if not use avcodec_receive_frame, need create frame buffer by self
                    createFrameBufferIfNull(frame);
                    ret = handler.get_frame(frame);
                } else {
                    ret = getFrame(frame);
                }

                if (ret == AVERROR_EOF) {
                    this->finished_serial = this->packet_serial;
                    flushBuffers();
                    return 0;
                }

                // frame not fill pts, use saved packet pts
                if ((frame->pts == AV_NOPTS_VALUE) && (decode_packet_pts != AV_NOPTS_VALUE)) {
                    frame->pts = decode_packet_pts;
                }

                if (ret == 0) {
                    // drop outdated frame
                    if ((frame_queue.getSize() != 0) && needDrop(frame->pts)) {
                        utils::releaseFrameData(frame);
                        av_frame_unref(frame);
                        return 0;
                    }
                    return 1;
                }
            } while (ret != AVERROR(EAGAIN));
        }
        thread_local AVPacket packet = {};
        if (getPacket(packet) != 0) {
            av_packet_unref(&packet);

            return 0;
        }
        // drop outdated packet
        // if ((!packet_queue.empty()) && needDrop(packet.pts)) {
        //     av_packet_unref(&packet);
        //     return 0;
        // }
        int32_t ret = AVERROR(EAGAIN);
        if (handler.decode_packet) {
            ret = handler.decode_packet(&packet);
        } else {
            ret = decodePacket(&packet);
        }
        if (AVERROR(EAGAIN) == ret) {
            CVI_LOGI("Get frame and decode packet both returned EAGAIN, which is an API violation");
            this->packet_pending = true;
            av_packet_move_ref(&this->pending_packet, &packet);
        }
        // save decode packet pts
        decode_packet_pts = packet.pts;
        av_packet_unref(&packet);
    }

    // return > 0 means success, = 0 means continue, < 0 means failed
    return 0;
}

void AvDecoder::decodespecial()
{
    const double targetInterval = 33.16;
    auto start = std::chrono::steady_clock::time_point();
    auto end = std::chrono::steady_clock::time_point();
    while (running) {
        if (handler.decode_packet) {
            start = std::chrono::steady_clock::now();
        }

        int got_frame = decodeFramespecial();
        if (got_frame < 0) {
            CVI_LOGE("decode frame failed %d", got_frame);
            break;
        }

        if (handler.decode_packet) {
            end = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> elapsedTime = end - start;
            double sleepTime = targetInterval - elapsedTime.count();
            if (sleepTime > 0) {
                usleep(sleepTime * 1000);
            }
        } else {
            usleep(30 * 1000);
        }
    }
}

int AvDecoder::decodeFramespecial()
{
    thread_local AVPacket packet = {};
    if (getPacket(packet) != 0) {
        av_packet_unref(&packet);
        return 0;
    }

    int ret = AVERROR(EAGAIN);
    if (handler.decode_packet) {
        ret = handler.decode_packet(&packet);
    } else {
        ret = decodePacket(&packet);
    }
    if (AVERROR(EAGAIN) == ret) {
        //CVI_LOGI("Get frame and decode packet both returned EAGAIN, which is an API violation");
        this->packet_pending = true;
        av_packet_move_ref(&this->pending_packet, &packet);
    }
    // save decode packet pts
    av_packet_unref(&packet);

    // return > 0 means success, = 0 means continue, < 0 means failed
    return 0;
}

void AvDecoder::getframe()
{
    AVFrame *frame = av_frame_alloc();
    if (frame == nullptr) {
        return;
    }

    while (running) {
        int got_frame = getdecodeFrame(frame);
        if (got_frame < 0) {
            CVI_LOGE("decode frame failed %d", got_frame);
            break;
        } else if (got_frame > 0) {
            if (enqueueFrame(frame) != CVI_ERROR::NONE) {
                CVI_LOGE("enqueue frame failed");
                break;
            }
        }
        if (handler.get_frame) {
            usleep(8 * 1000);
        } else {
            usleep(16 * 1000);
        }
    }

    if (frame != nullptr) {
        utils::releaseFrameData(frame);
        av_frame_free(&frame);
        frame = nullptr;
    }
}


int AvDecoder::getdecodeFrame(AVFrame *frame)
{
    int ret = AVERROR(EAGAIN);
    do {
        if (packet_queue.isAbort()) {
            flushBuffers();
            return 0;
        }

        if (handler.get_frame) {
            // if not use avcodec_receive_frame, need create frame buffer by self
            createFrameBufferIfNull(frame);
            ret = handler.get_frame(frame);
        } else {
            ret = getFrame(frame);
        }

        if (ret == AVERROR_EOF) {
            this->finished_serial = this->packet_serial;
            flushBuffers();
            return 0;
        }

        if (ret == 0) {
            // drop outdated frame
            if ((frame_queue.getSize() != 0) && needDrop(frame->pts)) {
                utils::releaseFrameData(frame);
                av_frame_unref(frame);
                return 0;
            }
            return 1;
        }
    } while (ret != AVERROR(EAGAIN));

    // return > 0 means success, = 0 means continue, < 0 means failed
    return 0;
}

int32_t AvDecoder::decodePacket(AVPacket *packet) const
{
    return (avctx == nullptr) ? AVERROR_EOF : avcodec_send_packet(avctx, packet);
}

int32_t AvDecoder::getFrame(AVFrame *frame)
{
    return (avctx == nullptr) ? AVERROR(EAGAIN) : avcodec_receive_frame(avctx, frame);
}

void AvDecoder::setHandler(const AvDecodeHandler &handler)
{
    this->handler = handler;
}

void AvDecoder::setHandler(AvDecodeHandler &&handler)
{
    this->handler = std::move(handler);
}

void AvDecoder::setSharedSyncContext(shared_ptr<SyncContext> sync_context)
{
    this->sync_context = std::move(sync_context);
}

void AvDecoder::setCodecParameters(AVCodecParameters *params)
{
    this->codec_params = params;
}

SyncContext *AvDecoder::getSyncContext()
{
    return this->sync_context.get();
}

bool AvDecoder::needDrop(int64_t pts) const
{
    return false;
}

} // namespace cvi_player
