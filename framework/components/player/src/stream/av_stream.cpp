#include "cvi_player/stream/av_stream.hpp"

namespace cvi_player {

using std::shared_ptr;

AvStream::AvStream(int32_t index, AVStream *stream)
: BaseStream(index, stream)
{
    clock = std::make_shared<Clock>();
}

bool AvStream::isPaused() const
{
    return clock->isPaused();
}

void AvStream::setPaused(bool paused)
{
    clock->setPaused(paused);
}

Clock &AvStream::getClock()
{
    return *clock;
}

shared_ptr<Clock> AvStream::getSharedClock()
{
    return clock;
}

void AvStream::setClockSpeed(double speed)
{
    clock->setSpeed(speed);
}

double AvStream::getClockTime() const
{
    return clock->getClockTime();
}

double AvStream::getClockPts() const
{
    return clock->getPts();
}

void AvStream::setSyncContext(shared_ptr<SyncContext> sync_context)
{
    if (decoder) {
        dynamic_cast<AvDecoder*>(decoder.get())->setSharedSyncContext(std::move(sync_context));
    }
}

void AvStream::setDecodeHandler(AvDecodeHandler handler)
{
    if (decoder) {
        dynamic_cast<AvDecoder*>(decoder.get())->setHandler(std::move(handler));
    }
}

void AvStream::setOutputHandler(OutputHandler output_handler)
{
    this->output_handler = std::move(output_handler);
}

bool AvStream::needRefresh() const
{
    return BaseStream::needRefresh() && (!isPaused());
}

} // namespace cvi_player
