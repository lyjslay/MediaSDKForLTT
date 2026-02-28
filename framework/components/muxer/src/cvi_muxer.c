
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/time.h>
#include "cvi_muxer.h"
#include "cvi_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
#include <libavutil/opt.h>
#include <libavcodec/codec_id.h>
#include "libswresample/swresample.h"
#ifdef __cplusplus
}
#endif


typedef struct cviMUXER_CTX_S{
    AVFormatContext *stAVFormatCtx;
    AVCodec *aacCodec;
    AVCodecContext *aacCtx;
    uint8_t *aacOuts;
    SwrContext *swr;
    AVPacket packet;
    AVFrame *aacFrame;
    AVStream *stAVStream[CVI_MUXER_FRAME_TYPE_BUTT];
    char filename[100];
    CVI_MUXER_ATTR_S stattr;

}CVI_MUXER_CTX_S;

#if CVI_MUXER_EXT_DATA_LEN > 0
const unsigned char g_ext_audio_data[4]= {0xFF, 0xF1, 0x60, 0x40};
#endif

#ifdef MUXER_AUDIO_DEBUG
static int32_t audio_fd = -1;
#endif

static enum AVCodecID cvi_muxer_get_video_avcodec(CVI_MUXER_TRACK_VIDEO_CODEC_E video_codec) {
    enum AVCodecID codec = AV_CODEC_ID_NONE;

    switch (video_codec) {
        case CVI_MUXER_TRACK_VIDEO_CODEC_H264:
            codec = AV_CODEC_ID_H264;
            break;
        case CVI_MUXER_TRACK_VIDEO_CODEC_H265:
            codec = AV_CODEC_ID_HEVC;
            break;
        default:
            break;
    }

    return codec;
}

static enum AVCodecID cvi_muxer_get_audio_avcodec(CVI_MUXER_TRACK_AUDIO_CODEC_E audio_codec) {
    enum AVCodecID codec = AV_CODEC_ID_NONE;

    switch (audio_codec) {
        case CVI_MUXER_TRACK_AUDIO_CODEC_ADPCM:
            codec = AV_CODEC_ID_PCM_S16LE;
            break;
        case CVI_MUXER_TRACK_AUDIO_CODEC_AAC:
            codec = AV_CODEC_ID_AAC;
            break;
        default:
            break;
    }

    return codec;
}


static int32_t cvi_muxer_add_video_stream(AVFormatContext *ctx, AVStream **avstream, CVI_MUXER_CODEC_VIDEO_S codec) {
    if (0 == codec.en) {
        return 0;
    }

    AVStream *av = avformat_new_stream(ctx, NULL);
    if (!av) {
        CVI_LOGE("could not allocate video stream");
        return -1;
    }
    CVI_LOGD("samplerate %f,  w %u, codec %d", codec.framerate, codec.w, codec.codec);

    av->id = ctx->nb_streams - 1;
    av->time_base = av_d2q(1.0 / codec.framerate, INT_MAX);
    av->codecpar->codec_id = cvi_muxer_get_video_avcodec(codec.codec);
    av->codecpar->width = codec.w;
    av->codecpar->height = codec.h;
    av->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    av->codecpar->format = 0;
    *avstream = av;
    return 0;
}


static int32_t cvi_muxer_add_audio_stream(AVFormatContext *ctx, AVStream **avstream, CVI_MUXER_CODEC_AUDIO_S codec) {
    if (0 == codec.en) {
        return 0;
    }

    AVStream *av = avformat_new_stream(ctx, NULL);
    if (!av) {
        CVI_LOGE("could not allocate audio stream");
        return -1;
    }

    CVI_LOGD("samplerate %u, chns %u, codec %d", codec.samplerate, codec.chns, codec.codec);

    av->id = ctx->nb_streams - 1;
    av->time_base.num = 1;
    av->time_base.den = codec.samplerate;
    av->codecpar->codec_id = cvi_muxer_get_audio_avcodec(codec.codec);
    av->codecpar->channels = codec.chns;
    av->codecpar->channel_layout = AV_CH_LAYOUT_MONO;//av_get_default_channel_layout(av->codecpar->channels);
    av->codecpar->sample_rate = codec.samplerate;
    av->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    av->codecpar->format = AV_SAMPLE_FMT_S16;
    if(av->codecpar->codec_id == AV_CODEC_ID_PCM_S16LE){
        av->codecpar->format = AV_SAMPLE_FMT_S16;
        // av->codecpar->frame_size =
        //     codec.chns * (codec.samplerate / codec.framerate) * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    }
    else{
        av->codecpar->format = AV_SAMPLE_FMT_FLTP;
        av->codecpar->bits_per_raw_sample = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * 8;
        av->codecpar->frame_size = 1024;
    }

    *avstream = av;
    return 0;
}

static int32_t cvi_muxer_add_subtitle_stream(AVFormatContext *ctx, AVStream **avstream, CVI_MUXER_CODEC_SUBTITLE_S codec) {
    if (codec.en == 0) {
        return 0;
    }

    AVStream *av = avformat_new_stream(ctx, NULL);
    if (!av) {
        CVI_LOGE("could not allocate subtitle stream");
        return -1;
    }


    av->id = ctx->nb_streams - 1;
    av->time_base = av_d2q(1.0 / (codec.timebase * codec.framerate), INT_MAX);
    av->codecpar->codec_type = AVMEDIA_TYPE_DATA;
    av->codecpar->codec_id = AV_CODEC_ID_BIN_DATA;
    av->codecpar->codec_tag = MKTAG('g', 'p', 'm', 'd');
    *avstream = av;
    return 0;
}

static int32_t cvi_muxer_add_data_stream(AVFormatContext *ctx, AVStream **avstream, CVI_MUXER_CODEC_THUMBNAIL_S codec) {
    if (codec.en == 0) {
        CVI_LOGE("not support priv data");
        return 0;
    }

    AVStream *av = avformat_new_stream(ctx, NULL);
    if (!av) {
        CVI_LOGE("could not allocate picture stream");
        return -1;
    }


    av->id = ctx->nb_streams - 1;
    av->codecpar->codec_type = AVMEDIA_TYPE_DATA;
    av->codecpar->codec_id = AV_CODEC_ID_BIN_DATA;
    av->codecpar->codec_tag = MKTAG('g', 'p', 'm', 'd');
    *avstream = av;
    return 0;
}

#ifdef FLUSH_MOOV_STREAM_ON
static uint64_t _CalcReservedMoovSize(uint64_t file_ms, float framerate) {
    uint64_t moov_size = 0;
    moov_size = file_ms * framerate / 1000 * 20;
    moov_size = (moov_size + 4095) & ~4095;

    return moov_size;

}
#endif

static int32_t cvi_muxer_create_file(CVI_MUXER_CTX_S *muxer) {
    int32_t ret = avformat_alloc_output_context2(&muxer->stAVFormatCtx, NULL, NULL, muxer->filename);
    if (ret < 0) {
        CVI_LOGE("[%s]: Could not deduce output format from file extension.", muxer->filename);
        return ret;
    }

    ret = cvi_muxer_add_video_stream(muxer->stAVFormatCtx, &muxer->stAVStream[CVI_MUXER_FRAME_TYPE_VIDEO], muxer->stattr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0]);
    ret |= cvi_muxer_add_video_stream(muxer->stAVFormatCtx, &muxer->stAVStream[CVI_MUXER_FRAME_TYPE_SUB_VIDEO], muxer->stattr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1]);
    ret |= cvi_muxer_add_audio_stream(muxer->stAVFormatCtx, &muxer->stAVStream[CVI_MUXER_FRAME_TYPE_AUDIO], muxer->stattr.staudiocodec);
    ret |= cvi_muxer_add_subtitle_stream(muxer->stAVFormatCtx, &muxer->stAVStream[CVI_MUXER_FRAME_TYPE_SUBTITLE], muxer->stattr.stsubtitlecodec);
    ret |= cvi_muxer_add_data_stream(muxer->stAVFormatCtx, &muxer->stAVStream[CVI_MUXER_FRAME_TYPE_THUMBNAIL], muxer->stattr.stthumbnailcodec);
    if(ret != 0){
        avformat_free_context(muxer->stAVFormatCtx);
        muxer->stAVFormatCtx = NULL;
        return ret;
    }

    if (muxer->stattr.devmod && strlen(muxer->stattr.devmod) > 0) {
        av_dict_set(&muxer->stAVFormatCtx->metadata, "title", muxer->stattr.devmod, 0);
    }

#ifdef MUXER_DEBUG_LOG
    av_dump_format(muxer->stAVFormatCtx, 0, muxer->filename, 1);
#endif

    if (muxer->stAVFormatCtx->oformat->flags & AVFMT_NOFILE) {
        return -1;
    }

    char value[24] = {0};
    struct tm *lt;
    time_t curtime;
    time(&curtime);

    lt = localtime(&curtime);
    memset(value, 0, sizeof(value));
    strftime(value, sizeof(value), "%Y-%m-%d %H:%M:%S", lt);

    /* Enable defragment funciton in ffmpeg */
    AVDictionary *dict = NULL;
    av_dict_set(&dict, "truncate", "false", 0);
    // av_dict_set_int(&dict, "blocksize", 512 * 1024, 0);
    av_dict_set(&muxer->stAVFormatCtx->metadata, "creation_time", value, 0);
    if (muxer->stattr.presize > 0) {
        av_dict_set_int(&dict, "preallocsize", muxer->stattr.presize, 0);
    }else{
        av_dict_set_int(&dict, "preallocsize", 0, 0);
    }

    ret = avio_open2(&muxer->stAVFormatCtx->pb, muxer->filename, AVIO_FLAG_WRITE, NULL, &dict);
    av_dict_free(&dict);
    if (ret < 0) {
        CVI_LOGE("[%s]: could not open file, error code: [%d]", muxer->filename, ret);
        avformat_free_context(muxer->stAVFormatCtx);
        muxer->stAVFormatCtx = NULL;
        return -1;
    }

    AVDictionary *opt = NULL;
#ifdef FLUSH_MOOV_STREAM_ON
    av_dict_set(&opt, "movflags", "flush_moov", 0);
    uint64_t moov_size = _CalcReservedMoovSize(muxer->stattr.u64SplitTimeLenMSec, muxer->stattr.stvideocodec.framerate);
    av_dict_set_int(&opt, "moov_size", moov_size, 0);
    av_dict_set_int(&opt, "flush_moov_stream", muxer->stAVStream[CVI_MUXER_FRAME_TYPE_VIDEO]->id, 0);
#endif
    ret = avformat_write_header(muxer->stAVFormatCtx, &opt);
    if (ret < 0) {
        avio_closep(&muxer->stAVFormatCtx->pb);
        muxer->stAVFormatCtx->pb = NULL;
        avformat_free_context(muxer->stAVFormatCtx);
        muxer->stAVFormatCtx = NULL;
        return -1;
    }
    CVI_LOGD("[%s]: create file success.", muxer->filename);

    return 0;
}

void CVI_MUXER_FlushPackets(void *muxer, int32_t flag)
{
    CVI_MUXER_CTX_S *hmuxer = (CVI_MUXER_CTX_S *)muxer;
    if(hmuxer && hmuxer->stAVFormatCtx){
        hmuxer->stAVFormatCtx->flush_packets = flag;
    }
}


int32_t CVI_MUXER_Create(CVI_MUXER_ATTR_S attr, void **muxer)
{
    CVI_MUXER_CTX_S *hmuxer = (CVI_MUXER_CTX_S *)malloc(sizeof(CVI_MUXER_CTX_S));
    if(hmuxer != NULL)
    {
        memset(hmuxer, 0x0, sizeof(CVI_MUXER_CTX_S));
        memcpy(&hmuxer->stattr, &attr, sizeof(CVI_MUXER_ATTR_S));
        if(attr.staudiocodec.codec == CVI_MUXER_TRACK_AUDIO_CODEC_AAC){
            hmuxer->aacCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
            if(hmuxer->aacCodec == NULL){
                CVI_LOGE("AV_CODEC_ID_AAC codec not found");
                return -1;
            }

            hmuxer->aacCtx = avcodec_alloc_context3(hmuxer->aacCodec);
            if(hmuxer->aacCtx == NULL){
                CVI_LOGE("avcodec_alloc_context3 alloc failed");
                return -1;
            }

            hmuxer->aacCtx->profile = FF_PROFILE_AAC_LOW;
            hmuxer->aacCtx->codec_type = AVMEDIA_TYPE_AUDIO;
            hmuxer->aacCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
            hmuxer->aacCtx->channels = attr.staudiocodec.chns;
            hmuxer->aacCtx->channel_layout = av_get_default_channel_layout(hmuxer->aacCtx->channels);
            hmuxer->aacCtx->sample_rate = hmuxer->stattr.staudiocodec.samplerate;
            hmuxer->aacCtx->bit_rate = hmuxer->aacCtx->sample_rate * hmuxer->aacCtx->channels;

            int32_t ret = avcodec_open2(hmuxer->aacCtx, hmuxer->aacCodec, NULL);
            if (ret < 0) {
                CVI_LOGE("open av codec failed %d", ret);
                return -1;
            }
            CVI_LOGD("audio framerate %f samplerate %u", hmuxer->stattr.staudiocodec.framerate, hmuxer->stattr.staudiocodec.samplerate);
            // hmuxer->swr = swr_alloc();
            // av_opt_set_int(hmuxer->swr, "in_channel_layout",  av_get_default_channel_layout(hmuxer->aacCtx->channels), 0);
            // av_opt_set_int(hmuxer->swr, "out_channel_layout", hmuxer->aacCtx->channel_layout,  0);
            // av_opt_set_int(hmuxer->swr, "in_sample_rate",     16000/*hmuxer->aacCtx->sample_rate*/, 0);
            // av_opt_set_int(hmuxer->swr, "out_sample_rate",    8000/*hmuxer->aacCtx->sample_rate*/, 0);
            // av_opt_set_sample_fmt(hmuxer->swr, "in_sample_fmt",  AV_SAMPLE_FMT_S16, 0);
            // av_opt_set_sample_fmt(hmuxer->swr, "out_sample_fmt", AV_SAMPLE_FMT_FLTP,  0);
            // swr_init(hmuxer->swr);

            hmuxer->aacFrame = av_frame_alloc();
            hmuxer->aacFrame->channels = hmuxer->aacCtx->channels;
            hmuxer->aacFrame->format = hmuxer->aacCtx->sample_fmt;
            hmuxer->aacFrame->channel_layout = hmuxer->aacCtx->channel_layout;
            hmuxer->aacFrame->sample_rate = hmuxer->stattr.staudiocodec.samplerate;
            hmuxer->aacFrame->nb_samples = hmuxer->aacCtx->frame_size;
            av_frame_get_buffer(hmuxer->aacFrame, 0);

            hmuxer->aacOuts = (uint8_t *)malloc(hmuxer->aacFrame->nb_samples * 5);
        }
        *muxer = hmuxer;
        return 0;
    }
    *muxer = NULL;
    return -1;
}

int32_t CVI_MUXER_Start(void *muxer, const char *fname)
{
    CVI_MUXER_CTX_S *hmuxer = (CVI_MUXER_CTX_S *)muxer;
    if(!hmuxer) {
        return -1;
    }
    strncpy(hmuxer->filename, fname, sizeof(hmuxer->filename) - 1);
    int32_t ret = cvi_muxer_create_file(hmuxer);
    if(ret != 0) {
        hmuxer->stattr.pfncallback(CVI_MUXER_OPEN_FILE_FAILED, fname, hmuxer->stattr.pfnparam, NULL);
        hmuxer->stattr.pfncallback(CVI_MUXER_STOP, fname, hmuxer->stattr.pfnparam, NULL);
    }
    return ret;
}

static int32_t cvi_muxer_write_video_packet(CVI_MUXER_CTX_S *hmuxer, CVI_MUXER_FRAME_TYPE_E type, CVI_MUXER_FRAME_INFO_S *packet){
    if(packet->dataLen > 0){
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.pts = packet->pts;
        pkt.dts = packet->pts;
        pkt.stream_index = hmuxer->stAVStream[type]->id;
        pkt.duration = 1;
        pkt.data = (uint8_t *)((uint8_t *)packet + sizeof(CVI_MUXER_FRAME_INFO_S));
        //CVI_LOGD("%#x %#x %#x %#x %#x %lld", pkt.data[0], pkt.data[1], pkt.data[2], pkt.data[3], pkt.data[4], pkt.pts);
        pkt.size = packet->dataLen;

        if(packet->isKey == 1) {
            pkt.flags = AV_PKT_FLAG_KEY;
        } else {
            pkt.flags = 0;
        }
        AVRational tb_src;
        if (type == CVI_MUXER_FRAME_TYPE_VIDEO) {
            tb_src = av_d2q(1.0 / hmuxer->stattr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_0].framerate, INT_MAX);
            av_packet_rescale_ts(&pkt, tb_src, hmuxer->stAVStream[CVI_MUXER_FRAME_TYPE_VIDEO]->time_base);
        } else if (type == CVI_MUXER_FRAME_TYPE_SUB_VIDEO){
            tb_src = av_d2q(1.0 / hmuxer->stattr.stvideocodec[CVI_MUXER_VIDEO_TRACK_IDX_1].framerate, INT_MAX);
            av_packet_rescale_ts(&pkt, tb_src, hmuxer->stAVStream[CVI_MUXER_FRAME_TYPE_SUB_VIDEO]->time_base);
        }
        int32_t ret = av_write_frame(hmuxer->stAVFormatCtx, &pkt);
        if (ret != 0) {
            CVI_LOGE("[%s]: failed to write packet, error [%d]", hmuxer->filename, ret);
            if (hmuxer->stattr.pfnparam) {
                hmuxer->stattr.pfncallback(CVI_MUXER_SEND_FRAME_FAILED, hmuxer->filename, hmuxer->stattr.pfnparam, NULL);
            }
        }
    }
    return 0;
}

static int32_t cvi_muxer_write_thumbnail_packet(CVI_MUXER_CTX_S *hmuxer, CVI_MUXER_FRAME_INFO_S *packet){
    if(hmuxer->stattr.stthumbnailcodec.en && packet->extraLen > 0){
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.pts = 0;
        pkt.dts = pkt.pts;
        pkt.duration = 1;
        pkt.stream_index = hmuxer->stAVStream[CVI_MUXER_FRAME_TYPE_THUMBNAIL]->id;
        pkt.data = (uint8_t *)((uint8_t *)packet + sizeof(CVI_MUXER_FRAME_INFO_S) + packet->dataLen);
        pkt.size = packet->extraLen;
        int32_t ret = av_write_frame(hmuxer->stAVFormatCtx, &pkt);
        if (ret != 0) {
            CVI_LOGE("[%s]: failed to write packet, error [%d]", hmuxer->filename, ret);
            if (hmuxer->stattr.pfnparam) {
                hmuxer->stattr.pfncallback(CVI_MUXER_SEND_FRAME_FAILED, hmuxer->filename, hmuxer->stattr.pfnparam, NULL);
            }
        }
    }

    return 0;
}

static void cvi_muxer_add_adts_header(uint8_t * in, int32_t len, const int32_t profile, const int32_t samplerate, const int32_t channels)
{
    static const int32_t sampling_frequencies[] = {
        96000,  // 0x0
        88200,  // 0x1
        64000,  // 0x2
        48000,  // 0x3
        44100,  // 0x4
        32000,  // 0x5
        24000,  // 0x6
        22050,  // 0x7
        16000,  // 0x8
        12000,  // 0x9
        11025,  // 0xa
        8000   // 0xb
        // 0xc d e f是保留的
    };

    int32_t sampling_frequency_index = 0x8;
    int32_t adtsLen = len;

    int32_t frequencies_size = sizeof(sampling_frequencies) / sizeof(sampling_frequencies[0]);
    int32_t i = 0;
    for(i = 0; i < frequencies_size; i++){
        if(sampling_frequencies[i] == samplerate){
            sampling_frequency_index = i;
            break;
        }
    }
    if(i >= frequencies_size){
        return;
    }

    in[0] = 0xff;         //syncword:0xfff                          高8bits
    in[1] = 0xf0;         //syncword:0xfff                          低4bits
    in[1] |= (0 << 3);    //MPEG Version:0 for MPEG-4,1 for MPEG-2  1bit
    in[1] |= (0 << 1);    //Layer:0                                 2bits
    in[1] |= 1;           //protection absent:1                     1bit

    in[2] = (profile) << 6;            //profile:profile               2bits
    in[2] |= (sampling_frequency_index & 0x0f) << 2; //sampling frequency index:sampling_frequency_index  4bits
    in[2] |= (0 << 1);             //private bit:0                   1bit
    in[2] |= (channels & 0x04) >> 2; //channel configuration:channels  高1bit

    in[3] = (channels & 0x03) << 6; //channel configuration:channels 低2bits
    in[3] |= (0 << 5);               //original：0                1bit
    in[3] |= (0 << 4);               //home：0                    1bit
    in[3] |= (0 << 3);               //copyright id bit：0        1bit
    in[3] |= (0 << 2);               //copyright id start：0      1bit
    in[3] |= ((adtsLen & 0x1800) >> 11);           //frame length：value   高2bits

    in[4] = (uint8_t)((adtsLen & 0x7f8) >> 3);     //frame length:value    中间8bits
    in[5] = (uint8_t)((adtsLen & 0x7) << 5);       //frame length:value    低3bits
    in[5] |= 0x1F;                                 //buffer fullness:0x7ff 高5bits
    in[6] = 0xFC;      //‭11111100‬       //buffer fullness:0x7ff 低6bits
    // number_of_raw_data_blocks_in_frame：
    //    表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧。
}


static int32_t cvi_muxer_pcm_to_aac(CVI_MUXER_CTX_S *hmuxer, uint8_t *in, int32_t inlen, uint8_t *out)
{
    int32_t ret = 0;

#if 0
    uint8_t **convert_data = (uint8_t**) calloc(hmuxer->aacCtx->channels, sizeof(*convert_data));
	av_samples_alloc(convert_data, NULL, hmuxer->aacCtx->channels, hmuxer->aacCtx->frame_size, hmuxer->aacCtx->sample_fmt, 0);
    int32_t size = av_samples_get_buffer_size(NULL, hmuxer->aacCtx->channels, hmuxer->aacCtx->frame_size, hmuxer->aacCtx->sample_fmt, 1);
    int32_t length = hmuxer->aacFrame->nb_samples * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16) * hmuxer->aacCtx->channels;
    int64_t delay = 0;
    const uint8_t *inBuf[1];
    inBuf[0] = in;
    inlen = inlen;
    ret = swr_convert(hmuxer->swr, convert_data, hmuxer->aacFrame->nb_samples, inBuf, inlen);
    if(ret <= 0){
        CVI_LOGE("swr_convert failed %d", hmuxer->aacFrame->pkt_size);
        goto END;
    }

    delay = swr_get_delay(hmuxer->swr , hmuxer->aacFrame->sample_rate);
    if(delay > 0){
        CVI_LOGE("swr_convert delay %lld %d", delay, ret);
    }

#else

    ret = av_frame_make_writable(hmuxer->aacFrame);
    if(ret != 0){
        CVI_LOGE("av_frame_make_writable failed %d", ret);
        return ret;
    }

    int16_t *inBuf = (int16_t *)in;
    for(int32_t c = 0; c < hmuxer->aacCtx->channels; c++){
        for(int32_t i = 0; i < hmuxer->aacFrame->nb_samples && i < inlen / 2; i++) {
            // s16 => fltp  aac
            *((float*)(hmuxer->aacFrame->data[c]) + i) = (float)inBuf[i] / 32767.0;
        }
    }
#endif

    ret = avcodec_send_frame(hmuxer->aacCtx, hmuxer->aacFrame);
    if (ret < 0) {
        CVI_LOGE("avcodec send frame failed %d", ret);
        return ret;
    }

    int32_t cnt = 0;
    while(ret >= 0){
        ret = avcodec_receive_packet(hmuxer->aacCtx, &hmuxer->packet);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            ret = 0;
            break;
        }
        if (ret < 0) {
            CVI_LOGE("avcodec recv frame failed %d", ret);
            break;
        }
        memcpy(out + 7 + cnt, hmuxer->packet.data, hmuxer->packet.size);
        cnt += hmuxer->packet.size;
    }

    if(cnt > 0){
        ret = cnt + 7;
        cvi_muxer_add_adts_header(out, ret, hmuxer->aacCtx->profile, hmuxer->aacCtx->sample_rate, hmuxer->aacCtx->channels);
    }
    return ret;
}

static int32_t cvi_muxer_write_audio_packet(CVI_MUXER_CTX_S *hmuxer, CVI_MUXER_FRAME_INFO_S *packet)
{
    if(packet->dataLen > 0){
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.pts = packet->pts;
        pkt.dts = packet->pts;
        pkt.stream_index = hmuxer->stAVStream[CVI_MUXER_FRAME_TYPE_AUDIO]->id;
        pkt.duration = 1;
        pkt.size = packet->dataLen;
        pkt.data = (uint8_t *)((uint8_t *)packet + sizeof(CVI_MUXER_FRAME_INFO_S) + CVI_MUXER_EXT_DATA_LEN);
        if(hmuxer->stattr.staudiocodec.codec == CVI_MUXER_TRACK_AUDIO_CODEC_AAC){
            memcpy(hmuxer->aacOuts, g_ext_audio_data, CVI_MUXER_EXT_DATA_LEN);
            pkt.size = cvi_muxer_pcm_to_aac(hmuxer, pkt.data, pkt.size, hmuxer->aacOuts + CVI_MUXER_EXT_DATA_LEN);
            pkt.data = hmuxer->aacOuts + CVI_MUXER_EXT_DATA_LEN;
            if(pkt.size <= 0){
                return 0;
            }
        }
        AVRational tb_src = av_d2q(1.0 / hmuxer->stattr.staudiocodec.framerate, INT_MAX);
        av_packet_rescale_ts(&pkt, tb_src, hmuxer->stAVStream[CVI_MUXER_FRAME_TYPE_AUDIO]->time_base);
    #ifdef MUXER_AUDIO_DEBUG
        static int32_t audio_cnt = 0;
        if(audio_cnt == 0){
            if(hmuxer->stattr.staudiocodec.codec == CVI_MUXER_TRACK_AUDIO_CODEC_AAC){
                audio_fd = open("/mnt/sd/audio.aac", O_CREAT | O_WRONLY);
            }else{
                audio_fd = open("/mnt/sd/audio.pcm", O_CREAT | O_WRONLY);
            }
        }
        if(audio_fd > 0){
            write(audio_fd, pkt.data, pkt.size);
        }

        if(audio_fd > 0 && audio_cnt++ > 100){
            close(audio_fd);
            audio_fd = -1;
        }
    #endif
        int32_t ret = av_write_frame(hmuxer->stAVFormatCtx, &pkt);
        if (ret != 0) {
            CVI_LOGE("[%s]: failed to write packet, error [%d]", hmuxer->filename, ret);
            if (hmuxer->stattr.pfnparam) {
                hmuxer->stattr.pfncallback(CVI_MUXER_SEND_FRAME_FAILED, hmuxer->filename, hmuxer->stattr.pfnparam, NULL);
            }
        }
    }
    return 0;
}

static int32_t cvi_muxer_write_subtitle_packet(CVI_MUXER_CTX_S *hmuxer, CVI_MUXER_FRAME_INFO_S *packet)
{
    if(hmuxer->stattr.stsubtitlecodec.en && packet->dataLen > 0){
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.pts = packet->pts;
        pkt.dts = packet->pts;
        pkt.stream_index = hmuxer->stAVStream[CVI_MUXER_FRAME_TYPE_SUBTITLE]->id;
        pkt.duration = 0;
        pkt.data = (uint8_t *)((uint8_t *)packet + sizeof(CVI_MUXER_FRAME_INFO_S));
        pkt.size = packet->dataLen;
        AVRational tb_src = av_d2q(1.0 / hmuxer->stattr.stsubtitlecodec.framerate, INT_MAX);
        av_packet_rescale_ts(&pkt, tb_src, hmuxer->stAVStream[CVI_MUXER_FRAME_TYPE_SUBTITLE]->time_base);
        int32_t ret = av_write_frame(hmuxer->stAVFormatCtx, &pkt);
        if (ret != 0) {
            CVI_LOGE("[%s]: failed to write packet, error [%d]", hmuxer->filename, ret);
            if (hmuxer->stattr.pfnparam) {
                hmuxer->stattr.pfncallback(CVI_MUXER_SEND_FRAME_FAILED, hmuxer->filename, hmuxer->stattr.pfnparam, NULL);
            }
        }
    }

    return 0;
}


int32_t CVI_MUXER_WritePacket(void *muxer,CVI_MUXER_FRAME_TYPE_E type ,CVI_MUXER_FRAME_INFO_S *packet)
{
    int32_t ret = -1;
    CVI_MUXER_CTX_S *hmuxer = (CVI_MUXER_CTX_S *)muxer;
    if(!hmuxer) {
        return ret;
    }
    switch(packet->type)
    {
        case CVI_MUXER_FRAME_TYPE_VIDEO:
        case CVI_MUXER_FRAME_TYPE_SUB_VIDEO:
        case CVI_MUXER_FRAME_TYPE_THUMBNAIL:
            ret = cvi_muxer_write_video_packet(hmuxer, type, packet);
            ret |= cvi_muxer_write_thumbnail_packet(hmuxer, packet);
        break;

        case CVI_MUXER_FRAME_TYPE_AUDIO:
            ret = cvi_muxer_write_audio_packet(hmuxer, packet);
        break;

        case CVI_MUXER_FRAME_TYPE_SUBTITLE:
            ret = cvi_muxer_write_subtitle_packet(hmuxer, packet);
        break;

        default:
            CVI_LOGI("packet type is error %d", packet->type);
        break;
    }
    return ret;
}

void CVI_MUXER_Stop(void *muxer)
{
    CVI_MUXER_CTX_S *hmuxer = (CVI_MUXER_CTX_S *)muxer;
    if(!hmuxer) {
        return;
    }
    av_write_frame(hmuxer->stAVFormatCtx, NULL);
    int32_t ret = av_write_trailer(hmuxer->stAVFormatCtx);
    if (0 != ret) {
        CVI_LOGE("[%s]: av_write_trailer failed, error code: [%d]", hmuxer->filename, ret);
    }

    if(hmuxer->stAVFormatCtx->pb){
        ret = avio_closep(&hmuxer->stAVFormatCtx->pb);
        if (0 != ret) {
            CVI_LOGE("[%s]: avio_closep failed, error code: [%d]", hmuxer->filename, ret);
        }
        hmuxer->stAVFormatCtx->pb = NULL;
    }
    avformat_free_context(hmuxer->stAVFormatCtx);
    hmuxer->stAVFormatCtx = NULL;
}

void CVI_MUXER_Destroy(void *muxer)
{
    CVI_MUXER_CTX_S *hmuxer = (CVI_MUXER_CTX_S *)muxer;
    if(hmuxer)
    {
        if(hmuxer->stAVFormatCtx){
            if(hmuxer->stAVFormatCtx->pb){
                avio_closep(&hmuxer->stAVFormatCtx->pb);
                hmuxer->stAVFormatCtx->pb = NULL;
            }
            avformat_free_context(hmuxer->stAVFormatCtx);
            hmuxer->stAVFormatCtx = NULL;
        }

        if(hmuxer->aacOuts){
            free(hmuxer->aacOuts);
        }

        if(hmuxer->aacFrame){
            av_frame_free(&hmuxer->aacFrame);
        }

        if(hmuxer->aacCtx){
            avcodec_free_context(&hmuxer->aacCtx);
        }

        if(hmuxer->swr){
            swr_free(&hmuxer->swr);
        }

        free(hmuxer);
    }
}




