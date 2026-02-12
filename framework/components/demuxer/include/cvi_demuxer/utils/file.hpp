#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <experimental/filesystem>
#include "cvi_log.h"

namespace cvi_demuxer {
namespace utils {

/**
 * @brief Get lower case file extension name from file name
 *
 * @param file_name is string of file name
 * @return lower case string for file extension name, and empty string if can't find extension
 */
inline std::string getFileExtensionName(const std::string& file_name)
{
    constexpr int32_t min_length = 2;
    if (file_name.length() < min_length) {
        return "";
    }

    std::size_t found_pos = file_name.find_last_of('.');
    if (found_pos == std::string::npos) {
        return "";
    }

    std::string extension = file_name.substr(found_pos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [] (const unsigned char c) {
            return std::tolower(c);
        }
    );

    return extension;
}

inline bool isJpegFile(const std::string& file_name)
{
    std::string extension = getFileExtensionName(file_name);
    return (extension == "jpeg") || (extension == "jpg");
}

inline uint64_t getFileSize(const std::string& file_name)
{
    uint64_t file_size = 0;
    try {
        file_size = std::experimental::filesystem::file_size(file_name);
    } catch (std::experimental::filesystem::filesystem_error &e) {
        CVI_LOGE("Can't get %s file size", file_name.c_str());
    }

    return file_size;
}

} // namespace utils
} // namespace cvi_demuxer
