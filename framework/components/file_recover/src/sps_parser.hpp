#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include "types.hpp"

namespace cvi_file_recover {

class SPSParser final
{
public:
    bool parse(Codec codec, const std::vector<uint8_t>& parse_data);
    Info getFrameInfo() const;

private:
    void init(const std::vector<uint8_t>& parse_data);
    size_t getDataSize() const;
    void ebspToRbsp();
    void parseH264();
    void parseH265();
    void parseH265TierProfile(int32_t max_sub_layers_minus1);
    int32_t getBits(uint64_t bits_offset, uint32_t n);
    int32_t readBit();
    int32_t readBits(uint32_t n);
    int32_t readExponentialGolombCode();
    int32_t readSE();

private:
    std::vector<uint8_t> data;
    uint64_t current_bit;
    Info frame_info{};
};

} // namespace cvi_file_recover
