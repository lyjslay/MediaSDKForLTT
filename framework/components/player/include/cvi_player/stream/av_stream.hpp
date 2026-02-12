#pragma once

#include <memory>
#include "base_stream.hpp"
#include "cvi_player/clock/clock.hpp"
#include "cvi_player/decoder/av_decode_handler.hpp"
#include "cvi_player/decoder/av_decoder.hpp"
#include "cvi_player/player/sync_context.hpp"

namespace cvi_player {

class AvStream : public BaseStream
{
public:
    using OutputHandler = std::function<void(CviPlayerFrame *)>;

    AvStream(int32_t index, AVStream *stream);

    bool isPaused() const;
    void setPaused(bool paused);
    Clock &getClock();
    std::shared_ptr<Clock> getSharedClock();
    void setClockSpeed(double speed);
    double getClockTime() const;
    double getClockPts() const;
    void setSyncContext(std::shared_ptr<SyncContext> sync_context);
    void setDecodeHandler(AvDecodeHandler handler);
    void setOutputHandler(OutputHandler output_handler);

protected:
    virtual bool needRefresh() const override;

protected:
    std::shared_ptr<Clock> clock;
    OutputHandler output_handler;
};

} // namespace cvi_player
