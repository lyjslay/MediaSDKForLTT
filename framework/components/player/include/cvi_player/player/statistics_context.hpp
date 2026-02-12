#pragma once

namespace cviplayer {

struct StatisticsContext {
    int32_t frame_drops_early{0};
    int32_t frame_drops_late{0};
};

} // namespace cviplayer
