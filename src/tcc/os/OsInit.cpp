#include "OsInit.h"

#include "linux/LinuxInit.h"
#include "windows/WindowsInit.h"

void osInit(bool dumpOnCrash)
{
#if defined(TC_WINDOWS)
    windowsInit(dumpOnCrash);
#elif defined(TC_LINUX)
    linuxInit();
#endif
}
