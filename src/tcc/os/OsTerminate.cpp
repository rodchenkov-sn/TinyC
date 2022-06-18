#include "OsTerminate.h"

void osTerminate()
{
#if defined(TC_WINDOWS)
    int* _p = nullptr;
    *_p = 42;
#elif defined(TC_LINUX)
    exit(1);
#endif
}
