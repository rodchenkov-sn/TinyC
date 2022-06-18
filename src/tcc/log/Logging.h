#ifndef TINYC_LOGGING_H
#define TINYC_LOGGING_H

#include <spdlog/spdlog.h>

inline void logInit()
{
    spdlog::set_pattern("%^[%l]%$ %v");
}

#define TC_LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define TC_LOG_WARN(...) spdlog::warn(__VA_ARGS__)
#define TC_LOG_ERROR(...) spdlog::error(__VA_ARGS__)
#define TC_LOG_CRITICAL(...) spdlog::critical(__VA_ARGS__)

#endif
