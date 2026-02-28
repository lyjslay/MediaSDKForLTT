#include "cvi_demuxer/ffmpeg_demuxer.hpp"
#include "cvi_demuxer/utils/file.hpp"
#include "cvi_demuxer/utils/packet.hpp"
#include "cvi_demuxer/utils/codec.hpp"
#include "cvi_log.h"
#include <unistd.h>
#include <sys/stat.h>
namespace cvi_demuxer {

using std::string;

namespace {

int32_t avContextInterruptCb(void *ctx)
{
    bool *opened = static_cast<bool *>(ctx);
    return (*opened) ? 0 : 1;
}

void copyCodecNameFromParm(AVCodecParameters *codec_parm, char *target, size_t target_length)
{
    const char *codec_name = avcodec_get_name(codec_parm->codec_id);
    if (codec_name != nullptr) {
        strncpy(target, codec_name, target_length);
    }
}

} // anonymous namespace

FFmpegDemuxer::~FFmpegDemuxer()
{
    close();
    freeInput();
}

int32_t FFmpegDemuxer::open()
{
    if (isOpened()) {
        CVI_LOGW("Already opened");
        return 0;
    }

    if (input == nullptr) {
        CVI_LOGE("Data input is null");
        return -1;
    }

    if (Demuxer::open() < 0) {
        Demuxer::close();
        return -1;
    }

    int32_t ret = openInput();
    if (ret != 0) {
        close();
        if (ret == 1) {
            return 1;
        } else {
            return -1;
        }
    }

    findStreams();
    updateStreamInfo();

    return 0;
}

int32_t FFmpegDemuxer::openTs()
{
    if (isOpened()) {
        CVI_LOGW("Already opened");
        return 0;
    }

    if (input == nullptr) {
        CVI_LOGE("Data input is null");
        return -1;
    }

    if (Demuxer::open() < 0) {
        Demuxer::close();
        return -1;
    }

    int32_t ret = openInputts();
    if (ret != 0) {
        close();
        if (ret == 1) {
            return 1;
        } else {
            return -1;
        }
    }

    findStreams();
    updateStreamInfo();

    return 0;
}

void FFmpegDemuxer::close()
{
    Demuxer::close();
    if (av_context != nullptr) {
        avformat_close_input(&av_context);
        av_context = nullptr;
    }
}

int32_t FFmpegDemuxer::read(AVPacket *packet)
{
    if ((av_context == nullptr) || (packet == nullptr)) {
        return -1;
    }

    if (packet->buf != nullptr) {
        CVI_LOGD("Packet buffer not null, may have memory leak");
    }

    int32_t type = getfilenameextern();
    int32_t ret = av_read_frame(av_context, packet);
    if (ret == 0) {
        if (utils::isH26X(video_codec_id) &&
            (packet->stream_index == stream_index[AVMEDIA_TYPE_VIDEO])) {
            utils::avccToAnnexbPacket(packet->data, packet->size, type);
        }
    } else {
        if (avio_feof(av_context->pb) != 0) {
            ret = AVERROR_EOF;
        }
    }

    return ret;
}

int32_t FFmpegDemuxer::readandcreationtime(AVPacket *packet, char *creationtime)
{
    if ((av_context == nullptr) || (packet == nullptr)) {
        return -1;
    }

    if (packet->buf != nullptr) {
        CVI_LOGD("Packet buffer not null, may have memory leak");
    }

    if (NULL != getcreationtime()) {
        memcpy(creationtime, getcreationtime(), 64);
    }
    int32_t type = getfilenameextern();
    int32_t ret = av_read_frame(av_context, packet);
    if (ret == 0) {
        if (utils::isH26X(video_codec_id) &&
            (packet->stream_index == stream_index[AVMEDIA_TYPE_VIDEO])) {
            utils::avccToAnnexbPacket(packet->data, packet->size, type);
        }
    } else {
        if (avio_feof(av_context->pb) != 0) {
            ret = AVERROR_EOF;
        }
    }

    return ret;
}

char* FFmpegDemuxer::getcreationtime()
{
    if (0 == getfilenameextern())
    {
        return NULL;
    }
    AVDictionaryEntry *tag = NULL;
    if (NULL == av_dict_get(av_context->metadata, "creation_time", tag, AV_DICT_MULTIKEY)) {
        CVI_LOGE("video info no creation_time");
        return NULL;
    } else {
        tag = av_dict_get(av_context->metadata, "creation_time", tag, AV_DICT_MULTIKEY);
        if (NULL == tag) {
            printf("This file has no creation time. It defaults to 1970-01-01-01-01-00");
            tag->value = (char *)"1970-01-01-01-01-00";
        }
    }
    return tag->value;
}

void FFmpegDemuxer::pause()
{
    Demuxer::pause();
    if (av_context != nullptr) {
        av_read_pause(av_context);
    }
}

void FFmpegDemuxer::resume()
{
    Demuxer::resume();
    if (av_context != nullptr) {
        av_read_play(av_context);
    }
}

int32_t FFmpegDemuxer::getNumberOfStream() const
{
    return (av_context == nullptr) ? 0 : av_context->nb_streams;
}

int32_t FFmpegDemuxer::getStreamIndex(int32_t media_type) const
{
    if ((media_type >= 0) && (media_type < AVMEDIA_TYPE_NB)) {
        return stream_index[media_type];
    }

    return -1;
}

AVCodecID FFmpegDemuxer::getVideoCodecId() const
{
    return video_codec_id;
}

AVStream *FFmpegDemuxer::getAvStream(int32_t index) const
{
    if ((index >= 0) && (static_cast<uint32_t>(index) < av_context->nb_streams)) {
        return av_context->streams[index];
    }

    return nullptr;
}

void FFmpegDemuxer::setInputFormat(AVInputFormat *input_format)
{
    this->input_format = input_format;
}

char *FFmpegDemuxer::getInput() const
{
    return input;
}

void FFmpegDemuxer::setInput(const std::string &input)
{
    freeInput();
    this->input = av_strdup(input.c_str());
}

int32_t FFmpegDemuxer::seek(int64_t target, int64_t offset, int32_t flag)
{
    // if target is time, unit is micro seconds
    int64_t seek_min = offset > 0 ? target - offset: INT64_MIN;
    int64_t seek_max = offset < 0 ? target - offset: INT64_MAX;
    //printf("### iformat:%s, oformat:%s, seek: target:%lld, offset:%lld, flag:%d, seek_min:%lld, seek_max:%lld\n", av_context->iformat->name, av_context->oformat->name, target, offset, flag, seek_min, seek_max);
    return avformat_seek_file(av_context, -1, seek_min, target, seek_max, flag);
}

bool FFmpegDemuxer::isRealTime() const
{
    if (av_context == nullptr) {
        return false;
    }

    if ((strncmp(av_context->iformat->name, "rtp", 3) == 0) ||
        (strncmp(av_context->iformat->name, "rtsp", 4) == 0) ||
        (strncmp(av_context->iformat->name, "sdp", 3) == 0)) {
        return true;
    }

    if ((av_context->pb != nullptr) &&
        ((strncmp(av_context->url, "rtp:", 4) == 0) ||
         (strncmp(av_context->url, "udp:", 4) == 0))) {
        return true;
    }

    return false;
}

double FFmpegDemuxer::getMaxFrameDuration() const
{
    if (av_context == nullptr) {
        return 0;
    }

    return ((av_context->iformat->flags & AVFMT_TS_DISCONT) != 0) ? 10.0 : 3600.0;
}

void FFmpegDemuxer::dumpInputInfo() const
{
    if (av_context != nullptr) {
        av_dump_format(av_context, 0, input, 0);
    }
}

int64_t FFmpegDemuxer::getStartTime() const
{
    if (av_context == nullptr) {
        return AV_NOPTS_VALUE;
    }

    return av_context->start_time;
}

void FFmpegDemuxer::setSubStreamFlag(bool subflag)
{
    subsign = subflag;
}

int32_t FFmpegDemuxer::getMediaInfo(CviDemuxerMediaInfo &info) const
{
    int32_t i = 0, number_of_video_stream = 0;

    if (input == nullptr) {
        CVI_LOGE("Demuxer not set input");
        return -1;
    }

    strncpy(info.file_name, input, sizeof(info.file_name));
    info.file_size = utils::getFileSize(input);
    // get format by guess
    AVOutputFormat *format = av_guess_format(NULL, input, NULL);
    if (format != nullptr) {
        strncpy(info.format, format->name, sizeof(info.format));
    }

    if (!isOpened()) {
        CVI_LOGE("Demuxer not opened");
        return -1;
    }

    info.duration_sec = av_context->duration*av_q2d(AV_TIME_BASE_Q);
    info.start_time_sec = getStartTime()*av_q2d(AV_TIME_BASE_Q);
    info.bit_rate = av_context->bit_rate;
    // get info from streams
    int32_t number_of_streams = getNumberOfStream();
    for (i = 0, number_of_video_stream = 0; i < number_of_streams; ++i) {
        AVStream *stream = getAvStream(i);
        AVCodecParameters *codec_parm = stream->codecpar;
        if (codec_parm->codec_type == AVMEDIA_TYPE_VIDEO) {
            if ((subsign) || (number_of_video_stream == 0)) {
                info.frame_rate = av_q2d(stream->avg_frame_rate);
                info.width = codec_parm->width;
                info.height = codec_parm->height;
                info.video_duration_sec = stream->duration*av_q2d(stream->time_base);
                info.used_video_stream_index = stream->index;
                copyCodecNameFromParm(codec_parm, info.video_codec, sizeof(info.video_codec));
                if (number_of_video_stream < CVI_DEMUXER_STREAM_MAX_NUM) {
                    info.stream_resolution[number_of_video_stream].stream_index = stream->index;
                    info.stream_resolution[number_of_video_stream].width = info.width;
                    info.stream_resolution[number_of_video_stream].height = info.height;
                    strncpy(info.stream_resolution[number_of_video_stream].codec, info.video_codec, sizeof(info.video_codec));
                }
                number_of_video_stream++;
            }
        } else if (codec_parm->codec_type == AVMEDIA_TYPE_AUDIO) {
            info.sample_rate = static_cast<uint32_t>(codec_parm->sample_rate);
            info.audio_duration_sec = stream->duration*av_q2d(stream->time_base);
            info.used_audio_stream_index = stream->index;
            info.audio_channel_layout = codec_parm->channels;
            copyCodecNameFromParm(codec_parm, info.audio_codec, sizeof(info.audio_codec));
        }
    }

    if (info.duration_sec < 0) {
        if (info.audio_duration_sec > 0) {
            info.duration_sec = info.audio_duration_sec;
        } else {
            info.duration_sec = info.video_duration_sec;
        }
    }

    return 0;
}

int32_t FFmpegDemuxer::openInput()
{
    av_context = avformat_alloc_context();
    if (av_context == nullptr) {
        CVI_LOGE("Could not allocate context");
        return -1;
    }

    av_context->interrupt_callback.callback = avContextInterruptCb;
    av_context->interrupt_callback.opaque = &opened;

    updateInputFormatIfNeed(utils::getFileExtensionName(input));
    int32_t err = avformat_open_input(&av_context, input, input_format, NULL);
    if (err < 0) {
        CVI_LOGE("Open input %s failed", input);
        return 1;
    }

    av_format_inject_global_side_data(av_context);

    av_context->pb->eof_reached = 0;

    return 0;
}

double FFmpegDemuxer::getfileinfotime()
{
    double infotime;
    infotime = ((av_context->duration) / 1000000.0);
    return infotime;
}

int32_t FFmpegDemuxer::getfilenameextern()
{
    char *fileextern = NULL;
    int32_t result = 0;

    if (nullptr == input) {
        return result;
    }

    fileextern = (strrchr(input, '.') + 1);
    if ((0 == (strcmp(fileextern, "TS"))) || (0 == (strcmp(fileextern, "ts")))) {
        result = 0;
    } else if ((0 == (strcmp(fileextern, "MOV"))) || (0 == (strcmp(fileextern, "mov")))) {
        result = 1;
    } else {
        result = 2;
    }

    return result;
}

uint32_t FFmpegDemuxer::Ue(unsigned char *pBuff, uint32_t nLen, uint32_t &nStartBit)
{
    uint32_t nZeroNum = 0;
    while(nStartBit < nLen * 8) {
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) {
            break;
        }
        nZeroNum++;
        nStartBit++;
    }
    nStartBit ++;

    unsigned long dwRet = 0;
    for (uint32_t i = 0; i < nZeroNum; i++) {
        dwRet <<= 1;
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) {
            dwRet += 1;
        }
        nStartBit++;
    }

    return (1 << nZeroNum) - 1 + dwRet;
}

int32_t FFmpegDemuxer::Se(unsigned char *pBuff, uint32_t nLen, uint32_t &nStartBit)
{
    int32_t UeVal = Ue(pBuff, nLen, nStartBit);
    double k = UeVal;
    int32_t nValue = ceil(k / 2);
    if (UeVal % 2 == 0) {
        nValue=-nValue;
    }
    return nValue;
}

unsigned long FFmpegDemuxer::u(uint32_t BitCount, unsigned char * buf, uint32_t &nStartBit)
{
    unsigned long dwRet = 0;
    for (uint32_t i = 0; i < BitCount; i++) {
        dwRet <<= 1;
        if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8))) {
            dwRet += 1;
        }
        nStartBit++;
    }
    return dwRet;
}

void FFmpegDemuxer::de_emulation_prevention(unsigned char* buf, uint32_t* buf_size)
{
    int32_t i = 0, j = 0;
    unsigned char* tmp_ptr = NULL;
    int32_t tmp_buf_size = 0;
    int32_t val = 0;

    tmp_ptr = buf;
    tmp_buf_size=*buf_size;
    for (i = 0; i < (tmp_buf_size - 2); i++) {
        //check for 0x000003
        val = (tmp_ptr[i]^0x00) + (tmp_ptr[i+1]^0x00) + (tmp_ptr[i+2]^0x03);
        if (val == 0) {
            //kick out 0x03
            for (j = i + 2; j < tmp_buf_size - 1; j++) {
                tmp_ptr[j] = tmp_ptr[j + 1];
            }

            //and so we should devrease bufsize
            (*buf_size)--;
        }
    }

    return;
}

int32_t FFmpegDemuxer::h264_decode_sps(unsigned char * buf, uint32_t nLen, int32_t &width, int32_t &height, float &fps)
{
    uint32_t StartBit = 0;
    fps = 0;
    de_emulation_prevention(buf, &nLen);

    u(1, buf, StartBit);
    u(2, buf, StartBit);
    int32_t nal_unit_type = u(5, buf, StartBit);
    int32_t seq_scaling_matrix_present_flag = 0;
    int32_t frame_cropping_flag = 0;
    int32_t video_signal_type_present_flag = 0;
    int32_t frame_crop_left_offset = 0;
    int32_t frame_crop_right_offset = 0;
    int32_t frame_crop_top_offset = 0;
    int32_t frame_crop_bottom_offset = 0;
    if (nal_unit_type == 7) {
        int32_t profile_idc = u(8, buf, StartBit);
        u(1, buf, StartBit);//(buf[1] & 0x80)>>7;
        u(1, buf, StartBit);//(buf[1] & 0x40)>>6;
        u(1, buf, StartBit);//(buf[1] & 0x20)>>5;
        u(1, buf, StartBit);//(buf[1] & 0x10)>>4;
        u(4, buf, StartBit);
        u(8, buf, StartBit);
        Ue(buf, nLen, StartBit);

        if (profile_idc == 100 || profile_idc == 110 ||
            profile_idc == 122 || profile_idc == 144) {
            int32_t chroma_format_idc = Ue(buf, nLen, StartBit);
            if (chroma_format_idc == 3) {
                u(1, buf, StartBit);
            }
            Ue(buf, nLen, StartBit);
            Ue(buf, nLen, StartBit);
            u(1, buf, StartBit);

            seq_scaling_matrix_present_flag = u(1, buf, StartBit);

            if (seq_scaling_matrix_present_flag) {
                for(int32_t i = 0; i < 8; i++) {
                    u(1, buf, StartBit);
                }
            }
        }

        Ue(buf, nLen, StartBit);
        int32_t pic_order_cnt_type = Ue(buf, nLen, StartBit);
        if (pic_order_cnt_type == 0) {
            Ue(buf, nLen, StartBit);
        } else if(pic_order_cnt_type == 1) {
            u(1, buf, StartBit);
            Se(buf, nLen, StartBit);
            Se(buf, nLen, StartBit);

            int32_t num_ref_frames_in_pic_order_cnt_cycle = Ue(buf, nLen,StartBit);
            // int32_t *offset_for_ref_frame = new int32_t[num_ref_frames_in_pic_order_cnt_cycle];
            for(int32_t i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
                // offset_for_ref_frame[i] = Se(buf, nLen, StartBit);
                Se(buf, nLen, StartBit);
            }
            // delete [] offset_for_ref_frame;
        }
        Ue(buf, nLen, StartBit);
        u(1, buf, StartBit);
        int32_t pic_width_in_mbs_minus1 = Ue(buf, nLen, StartBit);
        int32_t pic_height_in_map_units_minus1 = Ue(buf, nLen, StartBit);

        width = (pic_width_in_mbs_minus1 + 1) * 16;
        height = (pic_height_in_map_units_minus1 + 1) * 16;

        int32_t frame_mbs_only_flag = u(1, buf, StartBit);
        if (!frame_mbs_only_flag) {
            u(1, buf, StartBit);
        }

        u(1, buf, StartBit);
        frame_cropping_flag = u(1, buf, StartBit);
        if (frame_cropping_flag) {
            frame_crop_left_offset = Ue(buf, nLen, StartBit);
            frame_crop_right_offset = Ue(buf, nLen, StartBit);
            frame_crop_top_offset = Ue(buf, nLen, StartBit);
            frame_crop_bottom_offset = Ue(buf, nLen, StartBit);
            width = ((pic_width_in_mbs_minus1 + 1) * 16) - frame_crop_left_offset * 2 - frame_crop_right_offset * 2;
            height = ((2 - frame_mbs_only_flag) * (pic_height_in_map_units_minus1 +1) * 16) - (frame_crop_top_offset * 2) - (frame_crop_bottom_offset * 2);
        }
        int32_t vui_parameter_present_flag = u(1, buf, StartBit);
        if (vui_parameter_present_flag) {
            int32_t aspect_ratio_info_present_flag = u(1, buf, StartBit);
            if (aspect_ratio_info_present_flag) {
                int32_t aspect_ratio_idc = u(8, buf, StartBit);
                if (aspect_ratio_idc == 255) {
                    u(16, buf, StartBit);
                    u(16, buf, StartBit);
                }
            }
            int32_t overscan_info_present_flag = u(1, buf, StartBit);
            if (overscan_info_present_flag) {
                u(1, buf, StartBit);
            }
            video_signal_type_present_flag = u(1, buf, StartBit);
            if (video_signal_type_present_flag) {
                u(3, buf, StartBit);
                u(1, buf, StartBit);
                int32_t colour_description_present_flag = u(1, buf, StartBit);
                if (colour_description_present_flag) {
                    u(8, buf, StartBit);
                    u(8, buf, StartBit);
                    u(8, buf, StartBit);

                }
            }
            int32_t chroma_loc_info_present_flag = u(1, buf, StartBit);
            if (chroma_loc_info_present_flag) {
                Ue(buf, nLen, StartBit);
                Ue(buf, nLen, StartBit);
            }
            int32_t timing_info_present_flag = u(1, buf, StartBit);
            if (timing_info_present_flag) {
                int32_t num_units_in_tick = u(32, buf, StartBit);
                int32_t time_scale = u(32, buf, StartBit);
                fps = (float)time_scale / (2.0f * (float)num_units_in_tick);
            }
        }
        return true;
    } else {
        return false;
    }
}

int32_t FFmpegDemuxer::tspspsparse(CVI_DEMUXER_STREAM_INFO_S *streaminfo)
{
    FILE* file = fopen(input, "rb");
    if (file == NULL) {
        CVI_LOGE("open fail\n");
        return -1;
    }

    unsigned char sps_buffer[22];
    memset(sps_buffer, 0, sizeof(sps_buffer));
    long original_pos = ftell(file);
    fseek(file, 600, SEEK_SET);
    fread(sps_buffer, sizeof(unsigned char), sizeof(sps_buffer)/sizeof(sps_buffer[0]), file);
    if (sps_buffer[0] != 0x67) {
        fseek(file, original_pos, SEEK_SET);
        fseek(file, 594, SEEK_SET);
        memset(sps_buffer, 0, sizeof(sps_buffer));
        fread(sps_buffer, sizeof(unsigned char), sizeof(sps_buffer)/sizeof(sps_buffer[0]), file);
    }

    h264_decode_sps(sps_buffer, 22, streaminfo->videowidth, streaminfo->videoheight, streaminfo->frame_rate);

    streaminfo->videoden = 10;
    streaminfo->videonum = streaminfo->frame_rate * streaminfo->videoden;
    streaminfo->audionum = 1;
    streaminfo->audioden = 9000;
    streaminfo->frame_rate = streaminfo->frame_rate;

    int64_t pespacket = 0;
    struct stat statue;
    int32_t fd = fileno(file);
    fstat(fd, &statue);
    int32_t tshardcount = 0;
    tshardcount = statue.st_size / 188;
    if (tshardcount > 1500) {
        fseek(file, (tshardcount - 1500) * 188, SEEK_SET);
    }

    unsigned char packet_buffer[188];
    memset(packet_buffer, 0, sizeof(packet_buffer));
    while(fread(packet_buffer, sizeof(unsigned char), sizeof(packet_buffer)/sizeof(packet_buffer[0]), file)) {
        if (packet_buffer[0] == 0x47 && packet_buffer[1] == 0x41 && packet_buffer[12] == 0x00) {
            if (packet_buffer[12] == 0x00 && packet_buffer[13] == 0x00 &&
                packet_buffer[14] == 0x01 && packet_buffer[15] == 0xE0) {
                pespacket = (((packet_buffer[21] & 0x0E) >> 1) << 30) | (((packet_buffer[22] << 7) | ((packet_buffer[23] & 0xFE) >> 1)) << 15) | ((packet_buffer[24] << 7) | ((packet_buffer[25] & 0xFE) >> 1));
            }
        }
    }

    streaminfo->duration = (pespacket + 3600) / 0.09;

    fclose(file);

    return 0;
}

int32_t FFmpegDemuxer::openInputts()
{
    av_context = avformat_alloc_context();
    if (av_context == nullptr) {
        CVI_LOGE("Could not allocate context");
        return -1;
    }

    av_context->interrupt_callback.callback = avContextInterruptCb;
    av_context->interrupt_callback.opaque = &opened;

    updateInputFormatIfNeed(utils::getFileExtensionName(input));
    int32_t err = avformat_open_input(&av_context, input, input_format, NULL);
    if (err < 0) {
        CVI_LOGE("Open input %s failed", input);
        return 1;
    }

    if (0 == getfilenameextern()) {
        CVI_DEMUXER_STREAM_INFO_S streaminfo = {0};
        tspspsparse(&streaminfo);

        av_context->probesize = (av_context->probesize / 50);
        //err = avformat_find_stream_info(av_context, NULL);
        av_context->flags = AVFMT_FLAG_NOBUFFER;
        av_context->duration = streaminfo.duration;
        av_context->bit_rate = 3289924;
        for (uint32_t index = 0; index < av_context->nb_streams; index++)
        {
            if (index == 0) {
                av_context->streams[index]->codecpar->codec_tag = 27;
                av_context->streams[index]->codecpar->bit_rate = 0;
                av_context->streams[index]->codecpar->bits_per_coded_sample = 0;
                av_context->streams[index]->codecpar->bits_per_raw_sample = 8;
                av_context->streams[index]->codecpar->profile = 77;
                av_context->streams[index]->codecpar->level = 50;
                av_context->streams[index]->codecpar->format = 0;
                av_context->streams[index]->codecpar->field_order = AV_FIELD_UNKNOWN;
                av_context->streams[index]->codecpar->color_range = AVCOL_RANGE_UNSPECIFIED;
                av_context->streams[index]->codecpar->color_primaries = AVCOL_PRI_UNSPECIFIED;
                av_context->streams[index]->codecpar->color_trc = AVCOL_TRC_UNSPECIFIED;
                av_context->streams[index]->codecpar->color_space = AVCOL_SPC_UNSPECIFIED;
                av_context->streams[index]->codecpar->chroma_location = AVCHROMA_LOC_LEFT;
                av_context->streams[index]->codecpar->sample_aspect_ratio.num = 0;
                av_context->streams[index]->codecpar->sample_aspect_ratio.den = 1;
                av_context->streams[index]->codecpar->video_delay = 0;
                av_context->streams[index]->duration = streaminfo.duration * 0.9;
                av_context->streams[index]->r_frame_rate.num = streaminfo.videonum;
                av_context->streams[index]->r_frame_rate.den = streaminfo.videoden;
                av_context->streams[index]->avg_frame_rate.num = streaminfo.videonum;
                av_context->streams[index]->avg_frame_rate.den = streaminfo.videoden;
                av_context->streams[index]->codecpar->width = streaminfo.videowidth;
                av_context->streams[index]->codecpar->height = streaminfo.videoheight;
                av_context->streams[index]->codecpar->codec_id = AV_CODEC_ID_H264;
                av_context->streams[index]->index = index;
            } else if (index == 1) {
                av_context->streams[index]->codecpar->codec_tag = 15;
                av_context->streams[index]->codecpar->bit_rate = 0;
                av_context->streams[index]->codecpar->bits_per_coded_sample = 0;
                av_context->streams[index]->codecpar->bits_per_raw_sample = 0;
                av_context->streams[index]->codecpar->profile = 1;
                av_context->streams[index]->codecpar->level = -99;
                av_context->streams[index]->codecpar->format = 8;
                av_context->streams[index]->codecpar->channel_layout = 3;
                av_context->streams[index]->codecpar->frame_size = 1024;
                av_context->streams[index]->codecpar->initial_padding = 0;
                av_context->streams[index]->codecpar->trailing_padding = 0;
                av_context->streams[index]->codecpar->seek_preroll = 0;
                av_context->streams[index]->duration = streaminfo.duration * 0.9;
                av_context->streams[index]->avg_frame_rate.num = 0;
                av_context->streams[index]->avg_frame_rate.den = 0;
                av_context->streams[index]->codecpar->sample_rate = 16000;
                av_context->streams[index]->codecpar->channels = 2;
                av_context->streams[index]->codecpar->codec_id = AV_CODEC_ID_AAC;
                av_context->streams[index]->index = index;
                av_context->streams[index]->r_frame_rate.num = 0;
                av_context->streams[index]->r_frame_rate.den = 0;
            } else {
                av_context->streams[index]->codecpar->width = streaminfo.videowidth;
                av_context->streams[index]->codecpar->height = streaminfo.videoheight;
            }
        }

        if (err < 0) {
            CVI_LOGE("%s: could not find codec parameters", input);
            return -1;
        }
    } else {
        if (getfilenameextern() != 1) {
            err = avformat_find_stream_info(av_context, NULL);
        }
        av_format_inject_global_side_data(av_context);
    }
    av_context->pb->eof_reached = 0;

    return 0;
}

void FFmpegDemuxer::findStreams()
{
    std::fill_n(stream_index, AVMEDIA_TYPE_NB, -1);

    if (subsign) {
        int last_video_stream_index = -1;

        for (uint32_t i = 0; i < av_context->nb_streams; i++) {
            if (av_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                last_video_stream_index = i;
            }
        }

        stream_index[AVMEDIA_TYPE_VIDEO] = last_video_stream_index;
    } else {
        stream_index[AVMEDIA_TYPE_VIDEO] =
            av_find_best_stream(av_context, AVMEDIA_TYPE_VIDEO,
                                stream_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
    }

    if (0 == getfilenameextern()) {
        stream_index[AVMEDIA_TYPE_AUDIO] =
            av_find_best_stream(av_context, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    } else {
        stream_index[AVMEDIA_TYPE_AUDIO] =
            av_find_best_stream(av_context, AVMEDIA_TYPE_AUDIO,
                                stream_index[AVMEDIA_TYPE_AUDIO],
                                stream_index[AVMEDIA_TYPE_VIDEO],
                                NULL, 0);
        stream_index[AVMEDIA_TYPE_SUBTITLE] =
            av_find_best_stream(av_context, AVMEDIA_TYPE_DATA,
                                (stream_index[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                (stream_index[AVMEDIA_TYPE_AUDIO] + 1):
                                (stream_index[AVMEDIA_TYPE_VIDEO] + 1)),
                                (stream_index[AVMEDIA_TYPE_AUDIO] >= 0 ?
                                stream_index[AVMEDIA_TYPE_AUDIO] :
                                stream_index[AVMEDIA_TYPE_VIDEO]),
                                NULL, 0);
    }
    stream_index[AVMEDIA_TYPE_DATA] =
        av_find_best_stream(av_context, AVMEDIA_TYPE_DATA,
                            (av_context->nb_streams - 1) , -1, NULL, 0);

    if (stream_index[AVMEDIA_TYPE_DATA] == stream_index[AVMEDIA_TYPE_SUBTITLE]) {
        stream_index[AVMEDIA_TYPE_SUBTITLE] = -1;
    }

    saveStreamsParameters();
}

bool FFmpegDemuxer::getmjpegstreaminfo() {
    FILE* file = fopen(input, "rb");
    if (file == NULL) {
        CVI_LOGE("open file fail");
        return false;
    }

    unsigned char readbuffer[1024*10*5];
    fread(readbuffer, sizeof(unsigned char), sizeof(readbuffer) / sizeof(readbuffer[0]), file);
    for (int32_t i = 0; i < (int32_t)(sizeof(readbuffer) / sizeof(readbuffer[0])); i++) {
        if (readbuffer[i] == 0xFF && readbuffer[i + 1] == 0xC0 && readbuffer[i + 2] == 0x00 && //SOFO
            readbuffer[i + 3] == 0x11&& readbuffer[i + 4] == 0x08) {
                int32_t height = readbuffer[i + 5] * 256 + readbuffer[i + 6];
                int32_t width = readbuffer[i + 7] * 256 + readbuffer[i + 8];
                if (width != 320 && height != 180) {
                    av_context->streams[0]->codecpar->width = width;
                    av_context->streams[0]->codecpar->height = height;
                    fclose(file);
                    return true;
                }
            }
    }
    fclose(file);
    return false;
}

void FFmpegDemuxer::updateStreamInfo() {
    if (video_codec_id == AV_CODEC_ID_MJPEG) {
        if (av_context->nb_streams <= 1) {
            if (getmjpegstreaminfo()) {
                av_context->duration = 40000;
            } else {
                av_context->probesize = (av_context->probesize / 50);
                auto err = avformat_find_stream_info(av_context, NULL);
                if (err < 0) {
                    CVI_LOGE("%s: could not find codec parameters", input);
                }
            }
        }
    }
}

void FFmpegDemuxer::saveStreamsParameters()
{
    AVStream *video_stream = getAvStream(getStreamIndex(AVMEDIA_TYPE_VIDEO));
    if (video_stream && video_stream->codecpar) {
        video_codec_id = video_stream->codecpar->codec_id;
    }
}

void FFmpegDemuxer::freeInput()
{
    if (input != nullptr) {
        av_free(input);
        input = nullptr;
    }
}

void FFmpegDemuxer::updateInputFormatIfNeed(const string &input_extension)
{
    string format{};
    if ("raw" == input_extension) {
        format = "s16le";
    }
    // not need to update input format
    if (format.empty()) {
        input_format = nullptr;
        return;
    }

    input_format = av_find_input_format(format.c_str());
    if (input_format == nullptr) {
        CVI_LOGW("Unsupport file format %s", format.c_str());
    }
}

} // namespace cvi_demuxer
