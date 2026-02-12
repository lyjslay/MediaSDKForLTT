#pragma once

#include <string>
#include <memory>
#include "cvi_player/frame/frame.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
}

namespace cvi_player {
namespace utils {

void shallowCopyFrame(AVFrame *source, CviPlayerFrame *target);
void shallowCopyFrame(CviPlayerFrame *source, AVFrame *target);
void releaseFrameData(AVFrame *frame);

} // namespace utils
} // namespace cvi_player
