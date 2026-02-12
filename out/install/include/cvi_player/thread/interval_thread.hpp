#pragma once

#include <atomic>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <mutex>

namespace cvi_player {

class IntervalThread
{
public:
    ~IntervalThread()
    {
        stop();
    }

    template <typename Fun, typename WaitDuration>
    void start(Fun&& fun, const WaitDuration& wait_duration)
    {
        if (running) {
            stop();
        }

        paused = false;
        running = true;
        interval_thread = std::thread([this, fun, wait_duration]() {
            while (wait_for(wait_duration)) {
                fun();
            }
        });
    }

    void stop()
    {
        if (!running) {
            return;
        }

        paused = false;
        running = false;
        timeout_cv.notify_all();
        if (interval_thread.joinable()) {
            interval_thread.join();
        }
    }

    void pause()
    {
        paused = true;
        timeout_cv.notify_one();
    }

    void resume()
    {
        paused = false;
        timeout_cv.notify_one();
    }

private:
    template <typename WaitDuration>
    bool wait_for(const WaitDuration& wait_duration) {
        std::unique_lock<std::mutex> lock(cv_mutex);
        do {
            timeout_cv.wait_for(lock, wait_duration);
        } while (running && paused);

        return running;
    }

private:
    std::atomic<bool> running{false};
    std::atomic<bool> paused{false};
    std::mutex cv_mutex;
    std::condition_variable timeout_cv;
    std::thread interval_thread;
};

} // namespace cvi_player
