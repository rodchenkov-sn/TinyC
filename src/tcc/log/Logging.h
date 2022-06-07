#ifndef TINYC_LOGGING_H
#define TINYC_LOGGING_H

#include <spdlog/spdlog.h>

inline void logInit()
{
    spdlog::set_pattern("%^[%l]%$ %v");
}

#define TC_LOG_INFO(pattern, ...) spdlog::info(pattern, __VA_ARGS__)
#define TC_LOG_WARN(pattern, ...) spdlog::warn(pattern, __VA_ARGS__)
#define TC_LOG_ERROR(pattern, ...) spdlog::error(pattern, __VA_ARGS__)
#define TC_LOG_CRITICAL(pattern, ...) spdlog::critical(pattern, __VA_ARGS__)

#endif
