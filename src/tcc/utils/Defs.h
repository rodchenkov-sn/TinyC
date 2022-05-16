#ifndef TINYC_ASSERTIONS_H
#define TINYC_ASSERTIONS_H

#include <spdlog/spdlog.h>

namespace details {

inline void doAssert(bool cond, std::string_view msg, std::string_view file, int line)
{
    if (!cond) {
        spdlog::critical("assertion failed at {}:{} -- {}", file, line, msg);
        exit(1);
    }
}

}// namespace details

#define ASSERT(x) details::doAssert((x), #x, __FILE__, __LINE__)
#define ASSERT_MSG(x, msg) details::doAssert((x), msg)
#define ASSERT_FAIL(msg) details::doAssert(false, msg)

#define UNUSED(x) (void)x

#endif
