#pragma once

#include <memory>

namespace cvi_player {

class Clock
{
public:
    Clock();
    ~Clock() = default;

    double getClockTime() const;
    double getSpeed() const;
    double getLastUpdated() const;
    double getPts() const;
    int32_t getSerial() const;
    std::shared_ptr<int32_t> getSharedSerial() const;
    bool isPaused() const;
    void setClock(double pts, int32_t serial);
    void setSpeed(double speed);
    void setPaused(bool paused);
    void setQueueSharedSerial(std::shared_ptr<int32_t> serial);
    void syncTime();
    void syncToClock(const Clock &target);

private:
    void setClockTimeAt(double pts, int32_t serial, double time);

public:
    static constexpr double NOSYNC_THRESHOLD{1.0};

private:
    double pts{0}; // clock base
    double pts_drift{0}; // clock base minus time at which we updated the clock
    double last_updated{0};
    double speed{1.0};
    bool paused{false};
    std::shared_ptr<int32_t> serial; // clock is based on a packet with this serial
    std::shared_ptr<int32_t> queue_serial;
};

} // namespace cvi_player
