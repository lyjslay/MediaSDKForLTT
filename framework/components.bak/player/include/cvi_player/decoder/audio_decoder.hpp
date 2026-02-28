#pragma once

#include "av_decoder.hpp"
#include "cvi_demuxer/ffmpeg_demuxer.hpp"

namespace cvi_player {

class AudioDecoder final : public AvDecoder {
public:
    explicit AudioDecoder(AVCodecContext *avctx);

private:
    virtual int32_t getFrame(AVFrame *frame) override;
    virtual CVI_ERROR enqueueFrame(AVFrame *src_frame) override;
    virtual void createFrameBufferIfNull(AVFrame *frame) const override;
    std::unique_ptr<cvi_demuxer::FFmpegDemuxer> demuxer;
};

} // namespace cvi_player
