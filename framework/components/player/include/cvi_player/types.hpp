#pragma once

namespace cvi_player {

enum class CVI_ERROR : int32_t
{
    NONE,
    FAILURE,
    NO_MEMORY,
    NULL_PTR,
};

enum class AV_SYNC_TYPE : int32_t
{
    AUDIO_MASTER,
    VIDEO_MASTER,
    EXTERNAL_CLOCK, // synchronize to an external clock
};

} // namespace cvi_player
