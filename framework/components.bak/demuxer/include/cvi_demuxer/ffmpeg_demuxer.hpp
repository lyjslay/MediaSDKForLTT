#pragma once

#include "demuxer.hpp"
#include "media_info.h"
#include <stdint.h>
#include <string>

extern "C"
{
    #include <libavformat/avformat.h>
}

namespace cvi_demuxer {

class FFmpegDemuxer final : public Demuxer<AVPacket>
{
public:
    ~FFmpegDemuxer();

    virtual int32_t open() override;
    int32_t openTs();
    virtual void close() override;
    virtual int32_t read(AVPacket *packet) override;
    virtual void pause() override;
    virtual void resume() override;
    int32_t getNumberOfStream() const;
    int32_t getStreamIndex(int32_t media_type) const;
    AVCodecID getVideoCodecId() const;
    AVStream *getAvStream(int32_t index) const;
    void setInputFormat(AVInputFormat *input_format);
    char *getInput() const;
    void setInput(const std::string &input);
    int32_t seek(int64_t target, int64_t offset = 0, int32_t flag = 0);
    bool isRealTime() const;
    double getMaxFrameDuration() const;
    void dumpInputInfo() const;
    int64_t getStartTime() const;
    int32_t getMediaInfo(CviDemuxerMediaInfo &info) const;
    int32_t getfilenameextern();
    double getfileinfotime();
    char* getcreationtime();
    int32_t readandcreationtime(AVPacket *packet, char *creationtime);
    void setSubStreamFlag(bool subflag);

private:
    int32_t openInput();
    int32_t openInputts();
    void findStreams();
    void saveStreamsParameters();
    void updateStreamInfo();
    void freeInput();
    void updateInputFormatIfNeed(const std::string &input_extension);
    int32_t tspspsparse(CVI_DEMUXER_STREAM_INFO_S *streaminfo);
    bool getmjpegstreaminfo();

    uint32_t Ue(unsigned char *pBuff, uint32_t nLen, uint32_t &nStartBit);
    int32_t Se(unsigned char *pBuff, uint32_t nLen, uint32_t &nStartBit);
    unsigned long u(uint32_t BitCount, unsigned char * buf, uint32_t &nStartBit);
    void de_emulation_prevention(unsigned char* buf, uint32_t* buf_size);
    int32_t h264_decode_sps(unsigned char * buf, uint32_t nLen, int32_t &width, int32_t &height, float &fps);

private:
    AVFormatContext *av_context{nullptr};
    AVInputFormat *input_format{nullptr};
    char *input{nullptr};
    AVCodecID video_codec_id{AV_CODEC_ID_NONE};
    int32_t stream_index[AVMEDIA_TYPE_NB]{0};
    bool subsign = false;
};

} // namespace cvi_demuxer
