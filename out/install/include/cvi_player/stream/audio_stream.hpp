#pragma once

#include "av_stream.hpp"
#include <mutex>
#include "audio_parameters.h"

namespace cvi_player {

constexpr int32_t BUFFER_MIN_SIZE = 1280;

class AudioStream final : public AvStream
{
public:
    AudioStream(int32_t index, AVStream *stream);
    ~AudioStream();

    void setMuted(bool muted);
    void updateVolume(int32_t sign, double step);
    void setParameters(const CviPlayerAudioParameters &parameter);

protected:
    virtual void prepare() override;
    virtual void refresh(double &remaining_time) override;

private:
    void createFrameBuffer();
    void destroyFrameBuffer();
    void writeContinuouslyWithBuffer(const CviPlayerFrame &frame, int32_t &write_count);

private:
    int32_t audio_volume{0};
    bool muted{false};
    int64_t last_write_time{0};
    double last_remaining_time{0};
    int32_t buffer_current_size{0};
    int32_t buffer_total_size{BUFFER_MIN_SIZE};
    CviPlayerFrame frame_buffer{};
    mutable std::mutex buffer_mutex;
};

} // namespace cvi_player
