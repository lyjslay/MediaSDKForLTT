#pragma once

#include <thread>
#include <functional>
#include "cvi_player/queue/thread_safe_queue.hpp"

namespace cvi_player {

template <typename T>
class EventService
{
public:
    using EventHandler = std::function<void(T*)>;

    EventService()
    {
        event_thread = std::thread([this]() {
            T event;
            while (!event_queue.isAborted()) {
                if (!event_queue.dequeue(event)) {
                    break;
                }

                if (event_handler) {
                    event_handler(&event);
                }
            }
        });
    }

    virtual ~EventService()
    {
        event_queue.abort();
        if (event_thread.joinable()) {
            event_thread.join();
        }
    }

    void setEventHandler(const EventHandler& handler)
    {
        event_handler = handler;
    }

    void setEventHandler(EventHandler&& handler)
    {
        event_handler = std::move(handler);
    }

    void publishEvent(const T& event)
    {
        event_queue.enqueue(event);
    }

    void publishEvent(T&& event)
    {
        event_queue.enqueue(std::move(event));
    }

private:
    std::thread event_thread;
    EventHandler event_handler;
    ThreadSafeQueue<T> event_queue;
};

} // namespace cvi_player
