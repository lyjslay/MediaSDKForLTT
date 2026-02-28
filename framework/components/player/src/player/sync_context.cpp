#include "cvi_player/player/sync_context.hpp"

namespace cvi_player {

using std::shared_ptr;

void SyncContext::setType(AV_SYNC_TYPE type)
{
    this->type = type;
}

void SyncContext::setSharedClock(shared_ptr<Clock> clock)
{
    this->clock = std::move(clock);
}

AV_SYNC_TYPE SyncContext::getType() const
{
    return type;
}

double SyncContext::getClockTime() const
{
    return (clock) ? clock->getClockTime() : 0;
}

double SyncContext::getClockPts() const
{
    return (clock) ? clock->getPts() : 0;
}

} // namespace cvi_player
