#ifndef TINYC_PLATFORM_H
#define TINYC_PLATFORM_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#define TC_WINDOWS
#elif defined(__linux__)
#define TC_LINUX
#else
#error Unsupported platform
#endif

#endif//TINYC_PLATFORM_H
