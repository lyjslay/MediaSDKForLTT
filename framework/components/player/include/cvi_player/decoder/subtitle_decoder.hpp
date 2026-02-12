#pragma once

#include "decoder.hpp"

namespace cvi_player {

class SubtitleDecoder final : public Decoder {
public:
    explicit SubtitleDecoder(AVCodecContext *avctx);

private:
    virtual void decode() override;
    int32_t decodeFrame(AVSubtitle &subtitle);
    int32_t decodePacket(AVPacket &packet, AVSubtitle &subtitle);
    void enqueueFrame(Frame &frame);
    virtual void getframe() override;
    virtual void decodespecial() override;
};

} // namespace cvi_player
