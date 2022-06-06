#ifndef TINYC_ASSERTIONS_H
#define TINYC_ASSERTIONS_H

#include <spdlog/spdlog.h>

#include "os/OsTerminate.h"

namespace details {

inline void doAssert(bool cond, std::string_view msg, std::string_view file, int line)
{
    if (!cond) {
        spdlog::critical("assertion failed at {}:{} -- {}", file, line, msg);
        osTerminate();
    }
}

}// namespace details

#define TC_ASSERT(x) details::doAssert((x), #x, __FILE__, __LINE__)

#define TC_UNUSED(x) (void)x

#endif
