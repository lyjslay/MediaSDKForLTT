#pragma once

#include <string>
#include "cvi_demuxer/packet.h"

namespace cvi_thumbnail_extractor {

class ThumbnailExtractor final
{
public:
    int32_t getThumbnail(const std::string& input, CviDemuxerPacket& thumbnail);

private:
    int32_t getThumbnailFromDemuxer(const std::string& input, CviDemuxerPacket& thumbnail);
    int32_t getThumbnailFromJpegFile(const std::string& input, CviDemuxerPacket& thumbnail);
    int32_t ReplaceStr(char *sSrc, char const *sMatchStr, char const *sReplaceStr);
    long getTick(char *str_time);
};

} // namespace cvi_thumbnail_extractor
