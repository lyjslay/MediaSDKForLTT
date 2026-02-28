#include "cvi_player/utils/frame.hpp"
#include "cvi_mem.h"

static const int32_t FRAME_PLANAR_NUM = 2;

namespace cvi_player {
namespace utils {

void shallowCopyFrame(AVFrame *source, CviPlayerFrame *target)
{
    if (source == nullptr || target == nullptr) {
        return;
    }

    target->data = source->data;
    target->linesize = source->linesize;
    target->width = source->width;
    target->height = source->height;
    target->pts = source->pts;
    target->packet_size = source->pkt_size;
}

void shallowCopyFrame(CviPlayerFrame *source, AVFrame *target)
{
    if (source == nullptr || target == nullptr) {
        return;
    }

    const int32_t channel_size = sizeof(target->data)/sizeof(*target->data);
    for (int32_t i = 0; i < channel_size; ++i) {
        target->data[i] = source->data[i];
        target->linesize[i] = source->linesize[i];
    }
    target->width = source->width;
    target->height = source->height;
    target->pts = source->pts;
    target->pkt_size = source->packet_size;
}

void releaseFrameData(AVFrame *frame) {
    if (!frame) {
        return;
    }

    if (frame->format == AV_PIX_FMT_YUV420P) {
        for (int32_t i = 0; i < FRAME_PLANAR_NUM; ++i) {
            // CVI_MEM_Free(frame->data[i]);
            CVI_MEM_VbFree(frame->data[i]);
            frame->data[i] = nullptr;
        }
    }
}

} // namespace utils
} // namespace cvi_player
