#pragma once

#include "decoder.hpp"
#include <cstdint>
#include "av_decode_handler.hpp"
#include "cvi_player/player/sync_context.hpp"

namespace cvi_player {

class AvDecoder : public Decoder
{
public:
    explicit AvDecoder(AVCodecContext *avctx);
    ~AvDecoder() = default;

    void setHandler(const AvDecodeHandler &handler);
    void setHandler(AvDecodeHandler &&handler);
    void setSharedSyncContext(std::shared_ptr<SyncContext> sync_context);
    void setCodecParameters(AVCodecParameters *params);
    SyncContext *getSyncContext();

protected:
    virtual void decode() override;
    int32_t decodeFrame(AVFrame *frame);
    virtual int32_t decodePacket(AVPacket *packet) const;
    virtual int32_t getFrame(AVFrame *frame);
    virtual CVI_ERROR enqueueFrame(AVFrame *src_frame) = 0;
    virtual void createFrameBufferIfNull(AVFrame *frame) const = 0;
    virtual bool needDrop(int64_t pts) const;
    virtual void decodespecial() override;
    int decodeFramespecial();
    virtual void getframe() override;
    int getdecodeFrame(AVFrame *frame);

protected:
    AvDecodeHandler handler;
    std::shared_ptr<SyncContext> sync_context;
    AVCodecParameters *codec_params{nullptr}; // need codec params when avctx is null
};

} // namespace cvi_player
