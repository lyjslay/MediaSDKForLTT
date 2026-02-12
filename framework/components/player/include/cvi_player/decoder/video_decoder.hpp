#pragma once

#include "av_decoder.hpp"
#include <cstdint>
#include "cvi_player/types.hpp"

namespace cvi_player {

class VideoDecoder final : public AvDecoder {
public:
    explicit VideoDecoder(AVCodecContext *avctx);
    ~VideoDecoder() = default;

    void setTimebase(AVRational time);
    void setFramerate(AVRational framerate);
    void setOutputSize(uint32_t width, uint32_t height);
    uint8_t *getExtraData(int32_t &data_size) const;
    uint8_t *getVideoExtraData(int &data_size) const;

private:
    virtual int32_t getFrame(AVFrame *frame) override;
    virtual CVI_ERROR enqueueFrame(AVFrame *src_frame) override;
    virtual void createFrameBufferIfNull(AVFrame *frame) const override;
    virtual bool needDrop(int64_t pts) const override;

private:
    AVRational timebase{1, 12800};
    AVRational framerate{1, 25};
    uint32_t output_width{0};
    uint32_t output_height{0};
};

} // namespace cvi_player
