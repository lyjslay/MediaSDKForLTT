#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace cvi_player {

template <typename T>
class ThreadSafeQueue
{
public:
    ~ThreadSafeQueue() {
        abort();
    }

    bool dequeue(T &item) {
        std::unique_lock<std::mutex> lock(mutex);
        while((!abort_request) && queue.empty()) {
            cv.wait(lock);
        }

        if (abort_request) {
            return false;
        }

        item = std::move(queue.front());
        queue.pop();

        return true;
    }

    void enqueue(const T &item) {
        if (abort_request) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex);
        queue.push(item);
        cv.notify_one();
    }

    void enqueue(T &&item) {
        if (abort_request) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex);
        queue.emplace(std::move(item));
        cv.notify_one();
    }

    void abort() {
        abort_request = true;
        cv.notify_all();
    }

    bool isAborted() const {
        return abort_request;
    }

private:
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> abort_request{false};
};

} // namespace cvi_player
