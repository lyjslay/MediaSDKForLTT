#pragma once

#include <string>
#include "cvi_file_recover/event.h"
#include "event_handler.hpp"
#include "file.hpp"

namespace cvi_file_recover {

class Container : public EventHandler<CviFileRecoverEvent>
{
public:
    virtual ~Container()
    {
        close();
    }

    virtual int32_t open(const std::string& file_path,std::ios_base::openmode mode = std::fstream::in | std::fstream::out | std::fstream::binary)
    {
        close();
        this->file_path = file_path;
        return file.open(file_path, mode);
    }

    virtual void close()
    {
        parsed = false;
        file_path = "";
        file.close();
    }

    virtual int32_t parse()
    {
        parsed = true;
        file.clear();
        return 0;
    }

    virtual int32_t check()
    {
        return 0;
    };

    virtual void dump()
    {};

    virtual int32_t recover(const std::string &device_model, bool has_create_time, bool PreallocFlage, int32_t file_type) = 0;
    virtual void save(const std::string &file_path) = 0;

    std::string getFilePath() const
    {
        return file_path;
    }

    bool isParsed() const
    {
        return parsed;
    }

protected:
    std::string file_path{""};
    bool parsed{false};
    File file;
};

} // namespace cvi_file_recover
