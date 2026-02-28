#include "cvi_player/stream/subtitle_stream.hpp"
#include "cvi_player/decoder/subtitle_decoder.hpp"

namespace cvi_player {

SubtitleStream::SubtitleStream(int32_t index, AVStream *stream)
: BaseStream(index, stream)
{
    AVCodecContext *avctx = createCodecContext();
    decoder = std::make_unique<SubtitleDecoder>(avctx);
}

} // namespace cvi_player
