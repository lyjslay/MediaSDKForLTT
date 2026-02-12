#pragma once

#include <stdint.h>

typedef struct CviPlayerVideoParameters {
    uint32_t output_width;
    uint32_t output_height;
    uint32_t max_packet_size;
} CVI_PLAYER_VIDEO_PARAMETERS;
