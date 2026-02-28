#include "cvi_player/stream/base_stream.hpp"
#include <assert.h>
#include "cvi_log.h"
#include "cvi_mem.h"
#include "cvi_player/utils/frame.hpp"
#include "cvi_player/stream/audio_stream.hpp"

extern "C"
{
    #include <libavutil/time.h>
    #include <libswresample/swresample.h>
    #include <libavutil/samplefmt.h>
}

namespace cvi_player {

using std::condition_variable;
using std::shared_ptr;

constexpr float MAX_DURATION_SEC_OF_PACKETS = 0.4;
constexpr int32_t MAX_NUMBER_OF_PACKETS = 10;
struct SwrContext* resample_ctx;
bool Aacflag = false;
uint8_t* out_buffer;
double MediaRate = 0;
extern int32_t AudioResamplingRate;
extern int32_t AudioResamplingChannel;

BaseStream::BaseStream(int32_t index, AVStream *stream) :
stream(stream),
index(index)
{
    assert(stream);
    stream->discard = AVDISCARD_DEFAULT;
}

BaseStream::~BaseStream()
{
    close();
    MediaRate = 0;
}

void BaseStream::open()
{
    if (running) {
        close();
    }

    prepare();
    running = true;
    if (decoder) {
        decoder->start();
    }
    refresh_thread = std::thread(&BaseStream::refreshLoop, this);
}

void BaseStream::close()
{
    running = false;
    if (decoder) {
        decoder->stop();
    }
    if (refresh_thread.joinable()) {
        refresh_thread.join();
    }
    index = -1;
    stream->discard = AVDISCARD_ALL;
}

CVI_ERROR BaseStream::getFrame(CviPlayerFrame *player_frame)
{
    if ((player_frame == nullptr) || (!decoder)) {
        return CVI_ERROR::NULL_PTR;
    }

    Frame *frame = decoder->getFrameQueue().peekLast();
    utils::shallowCopyFrame(frame->frame, player_frame);

    return CVI_ERROR::NONE;
}

int32_t BaseStream::getIndex() const
{
    return index;
}

double BaseStream::getDuration() const
{
    return stream->duration*av_q2d(stream->time_base);
}

void BaseStream::flush()
{
    if (decoder) {
        decoder->flush();
    }
}

void BaseStream::setEmptyCV(shared_ptr<condition_variable> cv)
{
    if (decoder) {
        decoder->setEmptyCV(std::move(cv));
    }
}

bool BaseStream::isEmpty() const
{
    if (decoder) {
        const FrameQueue &frame_queue = decoder->getFrameQueue();
        const PacketQueue &packet_queue = decoder->getPacketQueue();
        return (frame_queue.getSize() == 0) && (packet_queue.getSize() == 0);
    }

    return true;
}

bool BaseStream::hasAttachedPicture() const
{
    return static_cast<bool>(stream->disposition & AV_DISPOSITION_ATTACHED_PIC);
}

AVPacket &BaseStream::getAttachedPacket() const
{
    return stream->attached_pic;
}

bool BaseStream::hasEnoughPackets() const
{
    bool enough = (index < 0) || hasAttachedPicture();
    if (decoder) {
        const PacketQueue &packet_queue = decoder->getPacketQueue();
        enough |= packet_queue.isAbort() ||
                    ((packet_queue.getSize() > MAX_NUMBER_OF_PACKETS) &&
                     ((packet_queue.getTotalDuration() == 0) ||
                      ((av_q2d(stream->time_base)*packet_queue.getTotalDuration()) > MAX_DURATION_SEC_OF_PACKETS)));
    }

    return enough;
}

PacketQueue &BaseStream::getPacketQueue()
{
    assert(decoder);
    return decoder->getPacketQueue();
}

void BaseStream::putNullPacket()
{
    assert(decoder);
    PacketQueue &packet_queue = decoder->getPacketQueue();
    packet_queue.putNullPacket(index);
}

bool BaseStream::isValidPacket(const AVPacket& packet) const
{
    return (packet.stream_index == index);
}

void BaseStream::prepare()
{
    setRefreshRate(1/av_q2d(stream->r_frame_rate));
    AVCodecParameters *codecParams = stream->codecpar;
    if (codecParams->codec_type == AVMEDIA_TYPE_VIDEO) {
        int width = codecParams->width;
        int height = codecParams->height;
        AVRational avgFrameRate = stream->avg_frame_rate;
        double frameRate = static_cast<double>(avgFrameRate.num) / avgFrameRate.den;

        if ((width >= 2560) && (height >= 1440) &&
            (frameRate == 30.0)) {
            if (decoder) {
                decoder->setspecialmediasign(true);
            }
        } else {
            if (decoder) {
                decoder->setspecialmediasign(false);
            }
        }
    }
}

double BaseStream::getRatetime()
{
    double Ratetime = 0;
    Ratetime= (1/av_q2d(stream->r_frame_rate));
    return Ratetime;
}

void BaseStream::refreshLoop()
{
    thread_local double remaining_time = refresh_rate;
    while (running) {
        if (remaining_time > 0.0) {
            av_usleep(static_cast<int64_t>(remaining_time * 1000000.0));
        }

        if (remaining_time != refresh_rate) {
            remaining_time = refresh_rate;
        }

        if (needRefresh()) {
            refresh(remaining_time);
        }
    }
}

bool BaseStream::needRefresh() const
{
    return (running) && (decoder);
}

void BaseStream::refresh(double &remaining_time)
{
    assert(decoder);
    FrameQueue &frame_queue = decoder->getFrameQueue();
    if (frame_queue.getSize() > 0) {
        frame_queue.next();
    }
}

AVCodecContext *BaseStream::createCodecContext()
{
    avcodec_register_all();
    AVCodecContext *avctx = avcodec_alloc_context3(nullptr);
    if (avctx == nullptr) {
        CVI_LOGE("Can't alloc memory");
        return nullptr;
    }

    if (avcodec_parameters_to_context(avctx, stream->codecpar) < 0) {
        avcodec_free_context(&avctx);
        return nullptr;
    }

    avctx->pkt_timebase = stream->time_base;
    if ((1/av_q2d(stream->avg_frame_rate)) > 0) {
        MediaRate = 1/av_q2d(stream->avg_frame_rate);
    }

    CVI_LOGI("codec id is (%d)", avctx->codec_id);
    Aacflag = false;
    if (dynamic_cast<AudioStream*>(this)) {
        if (AV_CODEC_ID_PCM_S16LE != avctx->codec_id/* &&
            AV_CODEC_ID_ADPCM_IMA_WAV != avctx->codec_id &&
            AV_CODEC_ID_PCM_S24LE != avctx->codec_id*/) {
            Aacflag = true;
        } else {
            if (AV_CODEC_ID_PCM_S16LE == avctx->codec_id) {
                if ((avctx->channels != 2) || (avctx->sample_rate != 16000) ||
                    ((avctx->sample_fmt != AV_SAMPLE_FMT_S16) && (avctx->sample_fmt != AV_SAMPLE_FMT_NONE))) {
                    Aacflag = true;
                }
            } else {
                Aacflag = false;
            }
        }
    }

    AVCodec *codec = avcodec_find_decoder(avctx->codec_id);
    if (codec == nullptr) {
        CVI_LOGD("No ffmpeg decoder could be found for codec %s", avcodec_get_name(avctx->codec_id));
        avcodec_free_context(&avctx);
        return nullptr;
    }
    avctx->codec_id = codec->id;

    int32_t stream_lowres = 0;
    if (stream_lowres > codec->max_lowres) {
        CVI_LOGW("The maximum value for lowres supported by the decoder is %d",
               codec->max_lowres);
        stream_lowres = codec->max_lowres;
    }
    avctx->lowres = stream_lowres;

    if (avcodec_open2(avctx, codec, nullptr) < 0) {
        CVI_LOGE("Context codec open failed");
		avcodec_free_context(&avctx);
        return nullptr;
	}

    if (true == Aacflag) {
    // init resample
        int32_t output_channels = AudioResamplingChannel;
        int32_t output_rate = AudioResamplingRate;
        int32_t input_channels = avctx->channels;
        int32_t input_rate = avctx->sample_rate;
        AVSampleFormat input_sample_fmt = avctx->sample_fmt;
        AVSampleFormat output_sample_fmt = AV_SAMPLE_FMT_S16;
        CVI_LOGI("channels[%d=>%d],rate[%d=>%d],sample_fmt[%d=>%d]",
        input_channels,output_channels,input_rate,output_rate,input_sample_fmt,output_sample_fmt);
        // out_buffer = (uint8_t*)malloc(192000);
        out_buffer = (uint8_t*)CVI_MEM_AllocateVb(192000);

        resample_ctx = swr_alloc_set_opts(resample_ctx, av_get_default_channel_layout(output_channels),
                                          output_sample_fmt, output_rate,
                                          av_get_default_channel_layout(input_channels),
                                          input_sample_fmt, input_rate,
                                          0, NULL);
        if (!resample_ctx) {
            CVI_LOGI("av_audio_resample_init fail!!!");
        }
        if (swr_init(resample_ctx) < 0) {
            CVI_LOGI("swr_init fail");
            swr_free(&resample_ctx);
        }
    }

    return avctx;
}

void BaseStream::setRefreshRate(double rate)
{
    if (!isnan(rate) && (rate != 0)) {
        refresh_rate = rate;
    } else {
        if (Aacflag == false) {
            refresh_rate = refresh_rate / stream->codecpar->channels;
        } else {
            refresh_rate = refresh_rate / AudioResamplingChannel;
        }
    }
}

void BaseStream::packetflush()
{
    if (decoder) {
        decoder->packetflush();
    }
}

} // namespace cvi_player
