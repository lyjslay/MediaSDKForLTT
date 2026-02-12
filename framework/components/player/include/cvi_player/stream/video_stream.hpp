#pragma once

#include "av_stream.hpp"
#include <memory>
#include <limits>
#include "cvi_demuxer/packet.h"
#include "video_parameters.h"

namespace cvi_player {

class VideoStream final : public AvStream
{
public:
    VideoStream(int32_t index, AVStream *stream);
    ~VideoStream();

    virtual bool isValidPacket(const AVPacket& packet) const override;
    void setMaxFrameDuration(double duration);
    void setParameters(const CviPlayerVideoParameters &parameter);
    CviDemuxerPacket *getExtraPacket();
    CviDemuxerPacket *getmediumExtraPacket();

private:
    virtual void refresh(double &remaining_time) override;
    void write();
    double getFrameDuration(const Frame &frame, const Frame &next_frame);
    double computeTargetDelay(double delay);

private:
    double last_refresh_time{0};
    double max_frame_duration{3600};
    int32_t max_packet_size{std::numeric_limits<int32_t>::max()};
    std::unique_ptr<CviDemuxerPacket> extra_packet;
    uint8_t *extra_packet_buffer{nullptr};
};

} // namespace cvi_player
