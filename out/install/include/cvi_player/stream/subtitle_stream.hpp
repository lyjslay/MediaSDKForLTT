#pragma once

#include "base_stream.hpp"

namespace cvi_player {

class SubtitleStream final : public BaseStream
{
public:
    SubtitleStream(int32_t index, AVStream *stream);
};

} // namespace cvi_player
