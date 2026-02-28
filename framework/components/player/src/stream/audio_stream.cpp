#include "cvi_player/stream/audio_stream.hpp"
#include "cvi_player/decoder/audio_decoder.hpp"
#include "cvi_player/utils/frame.hpp"
#include "cvi_mem.h"
#include "cvi_log.h"

extern "C"
{
    #include <libavutil/time.h>
    #include <libswresample/swresample.h>
}

namespace cvi_player {

constexpr int32_t AUDIO_FRAME_PLANAR_SIZE = 1;
constexpr int32_t AUDIO_MIX_MAXVOLUME = 128;
extern bool Aacflag;
int Aac_size = 0;
extern struct SwrContext* resample_ctx;
extern uint8_t* out_buffer;
int32_t AudioResamplingRate = 16000;
int32_t AudioResamplingChannel = 1;

AudioStream::AudioStream(int32_t index, AVStream *stream)
: AvStream(index, stream)
{
    AVCodecContext *avctx = createCodecContext();
    if (avctx == nullptr) {
        CVI_LOGI("AudioStream::AudioStream is null");
    }
    decoder = std::make_unique<AudioDecoder>(avctx);
    clock->setQueueSharedSerial(decoder->getPacketQueue().getSharedSerial());
    dynamic_cast<AudioDecoder*>(decoder.get())->setCodecParameters(stream->codecpar);
}

AudioStream::~AudioStream()
{
    destroyFrameBuffer();

    if (resample_ctx) {
        swr_free(&resample_ctx);
    }

    if (out_buffer) {
        CVI_MEM_VbFree(out_buffer);
        out_buffer = nullptr;
    }
}

void AudioStream::setMuted(bool muted)
{
    this->muted = muted;
}

void AudioStream::updateVolume(int32_t sign, double step)
{
    double &&volume_level = (audio_volume != 0) ? (20 * log(audio_volume / (double)AUDIO_MIX_MAXVOLUME) / log(10)) : -1000.0;
    int32_t &&new_volume = lrint(AUDIO_MIX_MAXVOLUME * pow(10.0, (volume_level + sign * step) / 20.0));
    audio_volume = av_clip(audio_volume == new_volume ? (audio_volume + sign) : new_volume, 0, AUDIO_MIX_MAXVOLUME);
}

void AudioStream::setParameters(const CviPlayerAudioParameters &parameter)
{
    // if (!isnan(parameter.sample_rate) && (parameter.sample_rate != 0)) {
    //     stream->codecpar->sample_rate = parameter.sample_rate;
    //     stream->codecpar->bit_rate = parameter.sample_rate*(stream->codecpar->bits_per_coded_sample/8);
    //     stream->time_base = AVRational{1, static_cast<int32_t>(parameter.sample_rate)};
    // }
    AudioResamplingRate = parameter.sample_rate;
    AudioResamplingChannel = parameter.channel;
}

void AudioStream::prepare()
{
    BaseStream::prepare();

    buffer_total_size = stream->codecpar->sample_rate*refresh_rate;
    buffer_total_size *= (stream->codecpar->bits_per_coded_sample/8) * stream->codecpar->channels;
    buffer_total_size = std::max(buffer_total_size, BUFFER_MIN_SIZE);
    if (Aacflag == true) {
        int bits_per_coded_sample = 16;
        buffer_total_size = AudioResamplingRate * (bits_per_coded_sample / 8) * AudioResamplingChannel * refresh_rate;
        Aac_size = 0;
    }
    if (frame_buffer.data != nullptr) {
        destroyFrameBuffer();
    }
    createFrameBuffer();
}

void AudioStream::refresh(double &remaining_time)
{
    double duration;
    FrameQueue &frame_queue = decoder->getFrameQueue();
    if (frame_queue.getSize() == 0) {
        // nothing to do, no frame in the queue
    } else {
        Frame *frame = frame_queue.peekLast();
        int time_multipler = 1;
        thread_local CviPlayerFrame player_frame = {};
        if (output_handler) {
            utils::shallowCopyFrame(frame->frame, &player_frame);
            // Aac_size = player_frame.packet_size;
            if ((player_frame.packet_size == buffer_total_size) &&
                (this->buffer_current_size == 0)) {
                output_handler(&player_frame);
            } else {
                // continuous send if frame data is enough
                writeContinuouslyWithBuffer(player_frame, time_multipler);
            }
            // calculate remaining time
            remaining_time = (remaining_time*time_multipler);
        } else {
            // calculate remaining time
            double time_multipler = static_cast<double>(frame->frame->pkt_size)/buffer_total_size;
            remaining_time = std::max(remaining_time*time_multipler, remaining_time);
        }
        // update clock
        if (!isnan(frame->pts)) {
            clock->setClock(frame->pts, frame->serial);
        }
        // update remaining time
        if (last_write_time != 0) {
            duration = (av_gettime_relative() - last_write_time)/1000000.0;
            remaining_time -= (duration - last_remaining_time);
        }
        last_write_time = av_gettime_relative();
        last_remaining_time = std::max(remaining_time, 0.0);

        if (Aacflag == true) {
            if (Aac_size > 0) {
                if (Aac_size + this->buffer_current_size < this->buffer_total_size) {
				    if (frame_queue.getSize() > 0) {
						frame_queue.next();
						Frame *last_frame = frame_queue.peekLast();
						utils::shallowCopyFrame(last_frame->frame, &player_frame);
						writeContinuouslyWithBuffer(player_frame, time_multipler);

						if (!isnan(frame->pts)) {
							clock->setClock(frame->pts, frame->serial);
						}
					}
                }
            }
        }

        if (frame_queue.getSize() > 0) {
            frame_queue.next();
        }
    }
}

void AudioStream::createFrameBuffer()
{
    std::lock_guard<std::mutex> lock(buffer_mutex);
    frame_buffer.data = new uint8_t*[AUDIO_FRAME_PLANAR_SIZE];
    for (int32_t i = 0; i < AUDIO_FRAME_PLANAR_SIZE; ++i) {
        frame_buffer.data[i] = (uint8_t *)CVI_MEM_AllocateVb(buffer_total_size); // (uint8_t *)CVI_MEM_Allocate(buffer_total_size, "player");
    }
    frame_buffer.packet_size = buffer_total_size;
}

void AudioStream::destroyFrameBuffer()
{
    std::lock_guard<std::mutex> lock(buffer_mutex);
    if (frame_buffer.data != nullptr) {
        for (int32_t i = 0; i < AUDIO_FRAME_PLANAR_SIZE; ++i) {
            // CVI_MEM_Free(frame_buffer.data[i]);
            CVI_MEM_VbFree(frame_buffer.data[i]);
            frame_buffer.data[i] = nullptr;
        }
        delete[] frame_buffer.data;
        frame_buffer.data = nullptr;
    }
}

void AudioStream::writeContinuouslyWithBuffer(const CviPlayerFrame &frame, int32_t &write_count)
{
    std::lock_guard<std::mutex> lock(buffer_mutex);
    if (frame_buffer.data == nullptr) {
        return;
    }

    int32_t total_size = this->buffer_current_size + frame.packet_size;
    int32_t remaining_size = frame.packet_size;
    int32_t total_copy_size = 0;
    write_count = 0;
    // write buffer data to output with buffer total size
    while (total_size > this->buffer_total_size) {
        int32_t copy_size = (this->buffer_total_size - this->buffer_current_size);
        memcpy(frame_buffer.data[0] + this->buffer_current_size,
            frame.data[0] + total_copy_size, copy_size);
        total_copy_size += copy_size;
        // write output
        output_handler(&frame_buffer);
        total_size -= this->buffer_total_size;
        remaining_size -= copy_size;
        this->buffer_current_size = 0;
        ++write_count;
    }
    // copy remaining data to buffer
    if (remaining_size > 0) {
        if ((this->buffer_current_size + remaining_size) > this->buffer_total_size) {
            return;
        }
        memcpy(frame_buffer.data[0] + this->buffer_current_size,
            frame.data[0] + (frame.packet_size - remaining_size), remaining_size);
        this->buffer_current_size += remaining_size;
    }
}

} // namespace cvi_player
