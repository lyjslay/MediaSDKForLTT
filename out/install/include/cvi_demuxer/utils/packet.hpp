#pragma once

#include <algorithm>
#include <cstdint>
#include <type_traits>
#include "cvi_demuxer/packet.h"
#include "codec.hpp"

extern "C"
{
    #include <libavcodec/avcodec.h>
}

namespace cvi_demuxer {
namespace utils {

constexpr int32_t PACKET_START_CODE_SIZE = 3;
constexpr uint8_t PACKET_START_CODE[PACKET_START_CODE_SIZE] = {0, 0, 1};
constexpr uint8_t H264_NAL_TYPE_SPS = 7;
constexpr uint8_t H265_NAL_TYPE_SPS = 32;
constexpr uint8_t H264_NAL_TYPE_IDR = 5;
constexpr uint8_t H265_NAL_TYPE_IDR = 19;

template <typename SourceType, typename TargetType>
inline void shallowCopyPacket(SourceType *source, TargetType *target)
{
    static_assert(std::is_same<SourceType, AVPacket>::value || std::is_same<SourceType, CviDemuxerPacket>::value,
        "Source type is not valid");
    static_assert(std::is_same<TargetType, AVPacket>::value || std::is_same<TargetType, CviDemuxerPacket>::value,
        "Target type is not valid");
    if (source == nullptr || target == nullptr) {
        return;
    }

    target->data = source->data;
    target->size = source->size;
    target->pts = source->pts;
}

inline void deepCopyPacket(AVPacket *source, CviDemuxerPacket *target)
{
    if (source == nullptr || target == nullptr) {
        return;
    }

    if (target->data != nullptr) {
        delete [] target->data;
        target->data = nullptr;
    }
    target->data = new uint8_t[source->size];
    memcpy(target->data, source->data, source->size);
    target->size = source->size;
    target->pts = source->pts;
}

template <uint64_t max_check_length>
inline bool hasStartCode(const uint8_t *data)
{
    static_assert(max_check_length > 0, "Max check length should > 0");

    bool find = false;
    for (uint64_t i = 0; i < max_check_length; ++i) {
        bool match = true;
        for (int32_t j = 0; j < PACKET_START_CODE_SIZE; ++j) {
            if (data[i + j] != PACKET_START_CODE[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            find = true;
            break;
        }
    }

    return find;
}

inline void avccToAnnexbPacket(uint8_t *data, uint64_t data_length, int32_t filetype)
{
    // return if data length too small or it is annexb packet already
    if (filetype == 0) {
        if ((data_length < PACKET_START_CODE_SIZE) ||
            hasStartCode<PACKET_START_CODE_SIZE>(data)) {
           return;
        }
    }
    if (data_length < PACKET_START_CODE_SIZE ||  hasStartCode<PACKET_START_CODE_SIZE>(data)) {
        return;
    }
    // number of bytes for packet payload length
    constexpr int32_t packet_length_size = 4;
    constexpr int32_t length_diff = packet_length_size - PACKET_START_CODE_SIZE;
    // no space left to put start code
    if (length_diff < 0) {
        return;
    }

    uint64_t max_check_pos = std::max(data_length - packet_length_size, static_cast<uint64_t>(0));
    uint64_t current_pos = 0;
    uint32_t length = 0;
    // replace payload length to start code
    while (current_pos < max_check_pos) {
        for (int32_t i = 0; i < packet_length_size; ++i) {
            length = (length << 8 | data[current_pos + i]);
        }

        for (int32_t i = 0; i < PACKET_START_CODE_SIZE; ++i) {
            data[current_pos + length_diff + i] = PACKET_START_CODE[i];
        }

        current_pos += (packet_length_size + length);
    }
}

inline uint8_t getSpsNalType(AVCodecID codec)
{
    return (codec == AV_CODEC_ID_H265) ? H265_NAL_TYPE_SPS : H264_NAL_TYPE_SPS;
}

inline uint8_t getIdrNalType(AVCodecID codec)
{
    return (codec == AV_CODEC_ID_H265) ? H265_NAL_TYPE_IDR : H264_NAL_TYPE_IDR;
}

inline uint8_t getNalType(AVCodecID codec, const uint8_t *data, uint64_t start_pos)
{
    uint8_t nal_type = data[start_pos];
    if (codec == AV_CODEC_ID_H264) {
        nal_type = (nal_type & 0x1F);
    } else if (codec == AV_CODEC_ID_H265) {
        nal_type = (nal_type & 0x7E) >> 1;
    }

    return nal_type;
}

inline bool hasNalUnit(uint8_t target_nal_type, AVCodecID codec, const uint8_t *data, uint64_t max_length)
{
    if (!isH26X(codec)) {
        return false;
    }

    bool find = false;
    uint64_t max_find_length = std::max(max_length - PACKET_START_CODE_SIZE, static_cast<uint64_t>(0));
    for (uint64_t i = 0; i < max_find_length;) {
        if (hasStartCode<1>(data + i)) {
            uint8_t nal_type = getNalType(codec, data, i + PACKET_START_CODE_SIZE);
            if (nal_type == target_nal_type) {
                find = true;
                break;
            }
            i += PACKET_START_CODE_SIZE;
        } else {
            ++i;
        }
    }

    return find;
}

} // namespace utils
} // namespace cvi_demuxer
