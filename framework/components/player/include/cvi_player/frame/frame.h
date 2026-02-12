#pragma once

typedef struct CviPlayerFrame
{
    uint8_t **data; // pointer to AVFrame data
    int32_t *linesize; // pointer to AVFrame linesize
    int32_t width;
    int32_t height;
    int32_t packet_size;
    int64_t pts;
} CVI_PLAYER_FRAME_S;
