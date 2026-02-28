#include "cvi_player/decoder/subtitle_decoder.hpp"
#include "cvi_player/packet/flush_packet.hpp"
#include "cvi_log.h"

namespace cvi_player {

SubtitleDecoder::SubtitleDecoder(AVCodecContext *avctx)
: Decoder(avctx)
{
    frame_queue.init(SUBTITLE_QUEUE_SIZE, false);
}

void SubtitleDecoder::decode()
{
    Frame *frame = nullptr;
    while (running) {
        frame = frame_queue.peekWritable();
        if (frame == nullptr) {
            return;
        }

        int32_t got_frame = decodeFrame(frame->subtitle);
        if (got_frame < 0) {
            CVI_LOGE("decode frame failed %d", got_frame);
            avsubtitle_free(&frame->subtitle);
            break;
        } else if (got_frame > 0) {
            enqueueFrame(*frame);
        }
    }
}

int32_t SubtitleDecoder::decodeFrame(AVSubtitle &subtitle)
{
    while (running) {
        thread_local AVPacket packet = {};
        if (getPacket(packet) != 0) {
            av_packet_unref(&packet);
            return 0;
        }

        int32_t ret = decodePacket(packet, subtitle);
        av_packet_unref(&packet);
        if (ret == AVERROR_EOF) {
            finished_serial = packet_serial;
            flushBuffers();
            return 0;
        } else if (ret == 0) {
            return 1;
        }
    }

    // return > 0 means success, = 0 means continue, < 0 means failed
    return 0;
}

int32_t SubtitleDecoder::decodePacket(AVPacket &packet, AVSubtitle &subtitle)
{
    if (!avctx) {
        return AVERROR_EOF;
    }

    int32_t got_subtitle = 0;
    int32_t ret = avcodec_decode_subtitle2(avctx, &subtitle, &got_subtitle, &packet);
    if (ret < 0) {
        ret = AVERROR(EAGAIN);
    } else {
        if ((got_subtitle != 0) && (packet.data == nullptr)) {
            packet_pending = true;
            av_packet_move_ref(&pending_packet, &packet);
        }
        ret = (got_subtitle != 0) ? 0 : ((packet.data != nullptr) ? AVERROR(EAGAIN) : AVERROR_EOF);
    }

    return ret;
}

void SubtitleDecoder::enqueueFrame(Frame &frame)
{
    double pts = 0;
    if (frame.subtitle.pts != AV_NOPTS_VALUE) {
        pts = frame.subtitle.pts/static_cast<double>(AV_TIME_BASE);
    }
    frame.pts = pts;
    frame.serial = packet_serial;
    frame.width = avctx->width;
    frame.height = avctx->height;
    frame_queue.commit();
}

void SubtitleDecoder::decodespecial()
{
    return;
}
void SubtitleDecoder::getframe()
{
    return;
}

} // namespace cvi_player
