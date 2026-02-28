#include "cvi_player/queue/frame_queue.hpp"
#include "cvi_player/utils/frame.hpp"
#include <algorithm>
#include "cvi_log.h"

namespace cvi_player {

using std::lock_guard;
using std::unique_lock;

constexpr int32_t FRAME_QUEUE_MAX_SIZE = FFMAX(AUDIO_QUEUE_SIZE, FFMAX(VIDEO_QUEUE_SIZE, SUBTITLE_QUEUE_SIZE));

FrameQueue::~FrameQueue()
{
    abort();
    destroyFrames();
}

void FrameQueue::init(const int32_t max_size, const bool keep_last)
{
    this->abort_request = false;
    this->keep_last = keep_last;
    try {
        this->max_size = FFMIN(max_size, FRAME_QUEUE_MAX_SIZE);
        queue = std::make_unique<Frame []>(this->max_size);
        for (int32_t i = 0; i < this->max_size; ++i) {
            queue[i].frame = av_frame_alloc();
            if (queue[i].frame == nullptr) {
                throw std::runtime_error("failed to alloc memory");
            }
        }
    } catch(...) {
        CVI_LOGE("Frame queue init failed");
        destroyFrames();
        this->max_size = 0;
    }
}

void FrameQueue::abort()
{
    lock_guard<std::mutex> lock(mutex);
    abort_request = true;
    cv.notify_all();
}

void FrameQueue::commit()
{
    if (++windex == max_size) {
        windex = 0;
    }

    unique_lock<std::mutex> lock(mutex);
    size = std::min(size + 1, max_size);
    cv.notify_one();
}

void FrameQueue::next()
{
    if (keep_last && (rindex_readed == 0)) {
        rindex_readed = 1;
        return;
    }

    unrefFrame(&queue[rindex]);
    if (++rindex == max_size) {
        rindex = 0;
    }

    unique_lock<std::mutex> lock(mutex);
    size = std::max(size - 1, 0);
    cv.notify_one();
}

int32_t FrameQueue::getSize() const
{
    return size - rindex_readed;
}

int32_t FrameQueue::getRightIndexReaded() const
{
    return rindex_readed;
}

bool FrameQueue::isAbort() const
{
    return abort_request;
}

void FrameQueue::destroyFrames()
{
    for (int32_t i = 0; i < max_size; ++i) {
        Frame *vp = &queue[i];
        unrefFrame(vp);
        av_frame_free(&(vp->frame));
    }
}

void FrameQueue::unrefFrame(Frame *vp) const
{
    utils::releaseFrameData(vp->frame);
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->subtitle);
}

Frame *FrameQueue::peek()
{
    return &queue[(rindex + rindex_readed) % max_size];
}

Frame *FrameQueue::peekNext()
{
    return &queue[(rindex + rindex_readed + 1) % max_size];
}

Frame *FrameQueue::peekLast()
{
    return &queue[rindex];
}

Frame *FrameQueue::peekWritable()
{
    unique_lock<std::mutex> lock(mutex);
    while ((size >= max_size) &&
           (!abort_request)) {
        cv.wait(lock);
    }
    lock.unlock();

    if (abort_request) {
        return nullptr;
    }

    return &queue[windex];
}

Frame *FrameQueue::peekReadable()
{
    unique_lock<std::mutex> lock(mutex);
    while (((size - rindex_readed) <= 0) &&
           (!abort_request)) {
        cv.wait(lock);
    }
    lock.unlock();

    if (abort_request) {
        return nullptr;
    }

    return &queue[(rindex + rindex_readed) % max_size];
}

} // namespace cvi_player
