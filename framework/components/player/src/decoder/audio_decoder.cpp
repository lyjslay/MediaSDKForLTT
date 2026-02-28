#include "cvi_player/decoder/audio_decoder.hpp"
#include <cmath>
#include "cvi_player/stream/base_stream.hpp"
#include "cvi_demuxer/ffmpeg_demuxer.hpp"
#include "cvi_log.h"

extern "C"
{
    #include <libavutil/time.h>
    #include <libswresample/swresample.h>
}

namespace cvi_player {

using std::isnan;
//#define MAX_AUDIO_FRAME_SIZE 192000 // 48khz 16bit audio 2 channels
extern struct SwrContext* resample_ctx;
extern bool Aacflag;
extern uint8_t* out_buffer;

constexpr int32_t DEFAULT_FRAME_FORMAT = AV_SAMPLE_FMT_S16;
extern int32_t AudioResamplingRate;
extern int32_t AudioResamplingChannel;

AudioDecoder::AudioDecoder(AVCodecContext *avctx)
: AvDecoder(avctx)
{
    frame_queue.init(AUDIO_QUEUE_SIZE, true);
}

int32_t AudioDecoder::getFrame(AVFrame *frame)
{
    int32_t size = 0;

    // Add variables, and directly test fit when the number of channels needs to be changed in the future
    int32_t outchannels = AudioResamplingChannel;

    int32_t ret = AvDecoder::getFrame(frame);
    if (ret >= 0) {
        AVRational timebase = (AVRational){1, frame->sample_rate};
        if (frame->pts != AV_NOPTS_VALUE) {
            frame->pts = av_rescale_q(frame->pts, avctx->pkt_timebase, timebase);
        } else if (next_pts != AV_NOPTS_VALUE) {
            frame->pts = av_rescale_q(next_pts, next_pts_timebase, timebase);
        }
        if (frame->pts != AV_NOPTS_VALUE) {
            next_pts = frame->pts + frame->nb_samples;
            next_pts_timebase = timebase;
        }

        if (true == Aacflag) {
            int64_t original_pts = frame->pts;
            int64_t original_best_effort_timestamp = frame->best_effort_timestamp;
            int original_flags = frame->flags;
            int original_sample_rate = frame->sample_rate;

            int original_key_frame = frame->key_frame;
            int64_t original_pkt_pts = frame->pkt_pts;
            int64_t original_pkt_dts = frame->pkt_dts;
            int64_t original_pkt_pos = frame->pkt_pos;
            int64_t original_pkt_duration = frame->pkt_duration;

            if (NULL == resample_ctx || NULL == out_buffer) {
                CVI_LOGI("resample_ctx is null");
            } else {
                int out_nb_samples = 0;
                out_nb_samples = av_rescale_rnd(frame->nb_samples, AudioResamplingRate, avctx->sample_rate, AV_ROUND_ZERO);

                size = frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
                memset(out_buffer, 0, 192000);
                int32_t out_samples = swr_convert(resample_ctx, &out_buffer, out_nb_samples, (const uint8_t **)frame->data, frame->nb_samples);

                if(out_samples > 0){
                    size = av_samples_get_buffer_size(NULL, outchannels, out_samples, AV_SAMPLE_FMT_S16, 1);// out_samples*output_channels*av_get_bytes_per_sample(output_sample_fmt);
                    av_frame_unref(frame);
                    frame->pkt_size = size;
                    frame->nb_samples = out_samples;
                    frame->format = AV_SAMPLE_FMT_S16;
                    frame->channel_layout = av_get_default_channel_layout(outchannels);
                    frame->channels = outchannels;

                    av_frame_get_buffer(frame, 0);

                    memcpy(frame->data[0], out_buffer, size);

                    frame->pts = original_pts;
                    frame->best_effort_timestamp = original_best_effort_timestamp;
                    frame->flags = original_flags;
                    frame->key_frame = original_key_frame;
                    frame->pkt_pts = original_pkt_pts;
                    frame->pkt_dts = original_pkt_dts;
                    frame->pkt_pos = original_pkt_pos;
                    frame->pkt_duration = original_pkt_duration;
                    frame->sample_rate = original_sample_rate;
                }
            }
            //av_free(out_buffer);

            // Resampled data without gain, if gain is required, open here
            // for(int32_t i = 0, j = 0; i < 640 * 2; i+= 2, j++) {
            //     *(short*)(&(frame->data[0][i])) *= 1.2;
            // }
        }
    }

    return ret;
}

CVI_ERROR AudioDecoder::enqueueFrame(AVFrame *src_frame)
{
    Frame *frame = frame_queue.peekWritable();
    if (frame_queue.isAbort()) {
        return CVI_ERROR::NONE;
    }
    if (frame == nullptr) {
        return CVI_ERROR::FAILURE;
    }

    AVRational timebase = {1, src_frame->sample_rate};
    double pts = (src_frame->pts == AV_NOPTS_VALUE) ? NAN : src_frame->pts * av_q2d(timebase);
    frame->pos = src_frame->pkt_pos;
    frame->pts = pts;
    frame->format = src_frame->format;
    frame->duration = av_q2d(AVRational{src_frame->nb_samples, src_frame->sample_rate});
    frame->serial = this->packet_serial;

    av_frame_move_ref(frame->frame, src_frame);
    frame_queue.commit();

    return CVI_ERROR::NONE;
}

void AudioDecoder::createFrameBufferIfNull(AVFrame *frame) const
{
    if (codec_params == nullptr) {
        return;
    }

    if ((frame != nullptr) && (frame->data[0] == nullptr)) {
        frame->format = (codec_params->format < 0) ? DEFAULT_FRAME_FORMAT : codec_params->format;
        frame->nb_samples = codec_params->sample_rate * av_q2d(AVRational{1, DEFAULT_FRAMERATE});
        frame->channel_layout = codec_params->channel_layout;
        av_frame_get_buffer(frame, 0);
    }
}

} // namespace cvi_player
