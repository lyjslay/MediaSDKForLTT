#pragma once

#include <cstdint>

namespace cvi_player {

struct SeekRequest {
    int32_t flags{0};
    int64_t pos;
    int64_t offset;
};

} // namespace cvi_player
