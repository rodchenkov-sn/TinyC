#ifndef TINYC_WINDOWSMINIDUMP_H
#define TINYC_WINDOWSMINIDUMP_H

#include "os/Platform.h"

#ifdef TC_WINDOWS

bool WindowsCreateMinidump(struct _EXCEPTION_POINTERS* ep);

#endif

#endif//TINYC_WINDOWSMINIDUMP_H
