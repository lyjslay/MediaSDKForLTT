#pragma once

#include <atomic>
#include <string>
#include <thread>
#include <memory>
#include "container.hpp"
#include "cvi_log.h"
#include "cvi_file_recover/event.h"
#include "cvi_player/service/event_service.hpp"

namespace cvi_file_recover {

class FileRecover final : public cvi_player::EventService<CviFileRecoverEvent>
{
public:
    ~FileRecover();

    int32_t open(const std::string &input_file_path);
    void join();
    void close();
    int32_t check() const;
    int32_t dump() const;
    int32_t recover(const std::string &output_file_path, const std::string &device_model, bool has_create_time);
    int32_t recoverAsync(const std::string &output_file_path, const std::string &device_model, bool has_create_time);
    void preallocatestate(bool PreallocFlage);

private:
    std::unique_ptr<Container> file;
    std::atomic<bool> is_recovering{false};
    std::thread recover_thread;
    std::string file_path;
};

} // namespace cvi_file_recover
