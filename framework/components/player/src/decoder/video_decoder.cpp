#include "cvi_player/decoder/video_decoder.hpp"
#include "cvi_log.h"
#include "cvi_mem.h"

namespace cvi_player {

constexpr int32_t DEFAULT_FRAME_FORMAT = AV_PIX_FMT_YUV420P;

VideoDecoder::VideoDecoder(AVCodecContext *avctx)
: AvDecoder(avctx)
{
    frame_queue.init(VIDEO_QUEUE_SIZE, true);
}

void VideoDecoder::setTimebase(AVRational time)
{
    timebase = time;
}

void VideoDecoder::setFramerate(AVRational framerate)
{
    this->framerate = framerate;
}

void VideoDecoder::setOutputSize(uint32_t width, uint32_t height)
{
    output_width = width;
    output_height = height;
}

uint8_t *VideoDecoder::getExtraData(int32_t &data_size) const
{
    if (avctx == nullptr) {
        data_size = 0;
        return nullptr;
    }

    data_size = avctx->extradata_size;
    return avctx->extradata;
}

uint8_t *VideoDecoder::getVideoExtraData(int &data_size) const
{
    if (codec_params == nullptr) {
        data_size = 0;
        return nullptr;
    }

    data_size = codec_params->extradata_size;

    return codec_params->extradata;
}

int32_t VideoDecoder::getFrame(AVFrame *frame)
{
    int32_t ret = AvDecoder::getFrame(frame);
    if (ret >= 0) {
        frame->pts = frame->best_effort_timestamp;
    }

    return ret;
}

CVI_ERROR VideoDecoder::enqueueFrame(AVFrame *src_frame)
{
    Frame *frame = frame_queue.peekWritable();
    if (frame_queue.isAbort()) {
        return CVI_ERROR::NONE;
    }
    if (frame == nullptr) {
        return CVI_ERROR::FAILURE;
    }

    double pts = (src_frame->pts == AV_NOPTS_VALUE) ? NAN : src_frame->pts * av_q2d(this->timebase);
    double duration = ((framerate.num != 0) && (framerate.den != 0)) ?
        av_q2d(AVRational{framerate.den, framerate.num}) : 0;
    frame->width = src_frame->width;
    frame->height = src_frame->height;
    frame->format = src_frame->format;
    frame->pos = src_frame->pkt_pos;
    frame->pts = pts;
    frame->duration = duration;
    frame->serial = this->packet_serial;
    av_frame_move_ref(frame->frame, src_frame);
    frame_queue.commit();

    return CVI_ERROR::NONE;
}

void VideoDecoder::createFrameBufferIfNull(AVFrame *frame) const
{
    if (codec_params == nullptr) {
        return;
    }

    if ((frame != nullptr) && (frame->data[0] == nullptr)) {
        frame->format = (codec_params->format < 0) ? DEFAULT_FRAME_FORMAT : codec_params->format;
        frame->width = (output_width == 0) ? codec_params->width : output_width;
        frame->height = (output_height == 0) ? codec_params->height : output_height;

        if (frame->format == AV_PIX_FMT_YUV420P) {
            const int32_t size = frame->width * frame->height;
            frame->data[0] = (uint8_t *)CVI_MEM_AllocateVb(size); // CVI_MEM_Allocate(size, "player");
            frame->data[1] = (uint8_t *)CVI_MEM_AllocateVb(size / 2); // CVI_MEM_Allocate(size / 2, "player");
        } else {
            av_frame_get_buffer(frame, 0);
        }
    }
}

bool VideoDecoder::needDrop(int64_t pts) const
{
    if (pts == AV_NOPTS_VALUE) {
        return true;
    }

    if (this->sync_context && (sync_context->getType() != AV_SYNC_TYPE::VIDEO_MASTER)) {
        double real_pts = av_q2d(timebase) * pts;
        double diff = real_pts - sync_context->getClockTime();
        double duration = ((framerate.num != 0) && (framerate.den != 0)) ?
            av_q2d(AVRational{framerate.den, framerate.num}) : 0;
        // check pts is outdated or not
        if (!isnan(diff) && (fabs(diff) < Clock::NOSYNC_THRESHOLD) &&
            ((diff + duration) < 0)) {
            return true;
        }
    }

    return false;
}

} // namespace cvi_player
