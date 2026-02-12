#pragma once

#include <stdint.h>

typedef struct CviDemuxerPacket
{
    uint8_t *data;
    int32_t size;
    int64_t pts;
    double duration;
    long creationtime;
    int32_t errorccode[4]; // File initialization, thumbnail, Creation time, duration
} CVI_DEMUXER_PACKET_S;
