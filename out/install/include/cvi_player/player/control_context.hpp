#pragma once

#include <cstdint>

extern "C"
{
    #include <libavcodec/avcodec.h>
}

namespace cvi_player {

struct ControlContext {
    bool finished{false};
    bool eof{false};
    int32_t play_loop_times{1};
    bool paused{false};
    bool last_paused{false};
    bool realtime{false};
    int64_t start_time{AV_NOPTS_VALUE};
    double frame_timer{0};
    double max_frame_duration{0}; /* maximum duration of a frame - above this, we consider the jump a timestamp discontinuity */
};

} // namespace cvi_player
