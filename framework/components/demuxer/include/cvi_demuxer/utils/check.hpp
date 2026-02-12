#pragma once

namespace cvi_demuxer {
namespace utils {

template <typename T>
inline bool hasNullptr(T ptr)
{
    return (ptr == nullptr);
}

template <typename First, typename... Rest>
inline bool hasNullptr(First first, Rest... rest)
{
    return hasNullptr(first) || hasNullptr(rest...);
}

} // namespace utils
} // namespace cvi_demuxer
