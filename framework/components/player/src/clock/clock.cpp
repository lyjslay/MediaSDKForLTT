#include "cvi_player/clock/clock.hpp"
#include <cmath>
#include <limits>

extern "C"
{
    #include <libavutil/time.h>
}

namespace cvi_player {

using std::shared_ptr;
using std::isnan;

Clock::Clock()
{
    queue_serial = std::make_shared<int32_t>(0);
    serial = std::make_shared<int32_t>(0);
    setClock(std::numeric_limits<double>::quiet_NaN(), -1);
}

double Clock::getClockTime() const
{
    if (*queue_serial != getSerial()) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    if (paused) {
        return pts;
    }

    double time = av_gettime_relative() / 1000000.0;
    return pts_drift + time - (time - last_updated) * (1.0 - speed);
}

double Clock::getLastUpdated() const
{
    return last_updated;
}

double Clock::getPts() const
{
    return pts;
}

double Clock::getSpeed() const
{
    return speed;
}

int32_t Clock::getSerial() const
{
    return *serial;
}

void Clock::setPaused(bool paused)
{
    this->paused = paused;
}

void Clock::setClock(double pts, int32_t serial)
{
    double time = av_gettime_relative() / 1000000.0;
    setClockTimeAt(pts, serial, time);
}

void Clock::setSpeed(double speed)
{
    syncTime();
    this->speed = speed;
}

void Clock::syncTime()
{
    setClock(getClockTime(), getSerial());
}

void Clock::syncToClock(const Clock &target)
{
    double clock_time = getClockTime();
    double target_time = target.getClockTime();
    if (!isnan(target_time) && (isnan(clock_time) || (fabs(clock_time - target_time) > Clock::NOSYNC_THRESHOLD))) {
        setClock(target_time, target.getSerial());
    }
}

void Clock::setQueueSharedSerial(shared_ptr<int32_t> serial)
{
    queue_serial = std::move(serial);
}

shared_ptr<int32_t> Clock::getSharedSerial() const
{
    return serial;
}

bool Clock::isPaused() const
{
    return paused;
}

void Clock::setClockTimeAt(double pts, int32_t serial, double time)
{
    this->pts = pts;
    this->last_updated = time;
    this->pts_drift = this->pts - time;
    *this->serial = serial;
}

} // namespace cvi_player
