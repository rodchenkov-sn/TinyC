#include "OsTerminate.h"

#include "Platform.h"

void osTerminate()
{
#ifdef TC_WINDOWS
    int* _p = nullptr;
    *_p = 42;
#elif TC_LINUX
    exit(1);
#endif
}
