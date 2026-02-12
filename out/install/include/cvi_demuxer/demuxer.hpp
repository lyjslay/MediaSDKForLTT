#pragma once

#include <atomic>

namespace cvi_demuxer {

template <typename T>
class Demuxer
{
public:
    virtual ~Demuxer() = default;

    virtual int32_t read(T *data) = 0;

    virtual int32_t open()
    {
        opened = true;
        return 0;
    }

    virtual void close()
    {
        opened = false;
    }

    virtual void pause()
    {
        paused = true;
    }

    virtual void resume()
    {
        paused = false;
    }

    bool isOpened() const
    {
        return opened;
    }

    bool isPaused() const
    {
        return paused;
    }

protected:
    std::atomic<bool> opened{false};
    std::atomic<bool> paused{false};
};

} // namespace cvi_demuxer
