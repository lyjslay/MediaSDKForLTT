#pragma once

#include <chrono>
#include <string>
#include <stdint.h>
#include "cvi_log.h"

namespace cvi_player {
namespace utils {

class Timer
{
public:
    Timer() = default;
    explicit Timer(std::string name) : name(std::move(name)) {}

    inline void tik() {
        last_tik = std::chrono::high_resolution_clock::now();
    }

    inline void tok() {
        duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - last_tik);
        total_duration += duration;
        count++;
    }

    inline void log() {
        CVI_LOGI("%s duration: %.4lf ms, average duration:%.4lf ms", name.c_str(), duration.count()/1000.0, (total_duration.count()/1000.0)/count);
    }

    inline void tokLog() {
        tok();
        log();
    }

private:
    std::chrono::high_resolution_clock::time_point last_tik;
    std::chrono::microseconds duration{0};
    std::chrono::microseconds total_duration{0};
    uint64_t count{0};
    std::string name{""};
};

} // namespace utils
} // namespace cvi_player
