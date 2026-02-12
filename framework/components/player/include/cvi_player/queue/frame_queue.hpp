#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include "cvi_player/frame/frame_internal.hpp"

namespace cvi_player {

constexpr int32_t AUDIO_QUEUE_SIZE = 3;
constexpr int32_t VIDEO_QUEUE_SIZE = 3;
constexpr int32_t SUBTITLE_QUEUE_SIZE = 3;

class FrameQueue {
public:
    FrameQueue() = default;
    ~FrameQueue();

    void init(const int32_t max_size, const bool keep_last);
    void abort();
    void commit();
    void next();
    int32_t getSize() const;
    int32_t getRightIndexReaded() const;
    bool isAbort() const;
    Frame *peek();
    Frame *peekNext();
    Frame *peekLast();
    Frame *peekWritable();
    Frame *peekReadable();

private:
    void destroyFrames();
    void unrefFrame(Frame *vp) const;

private:
    std::atomic<bool> abort_request{false};
    std::unique_ptr<Frame[]> queue;
    int32_t rindex{0};
    int32_t windex{0};
    int32_t size{0};
    int32_t max_size{0};
    bool keep_last{false};
    int32_t rindex_readed{0};
    mutable std::mutex mutex;
    std::condition_variable cv;
};

} // namespace cvi_player
