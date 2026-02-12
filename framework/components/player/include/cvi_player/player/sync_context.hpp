#pragma once

#include <memory>
#include "cvi_player/clock/clock.hpp"
#include "cvi_player/types.hpp"

namespace cvi_player {

class SyncContext {
public:
    void setType(AV_SYNC_TYPE type);
    void setSharedClock(std::shared_ptr<Clock> clock);
    AV_SYNC_TYPE getType() const;
    double getClockTime() const;
    double getClockPts() const;

private:
    AV_SYNC_TYPE type{AV_SYNC_TYPE::AUDIO_MASTER};
    std::shared_ptr<Clock> clock;
};

} // namespace cvi_player
