#include "os/Platform.h"

#ifdef TC_WINDOWS

#include <windows.h>

#include <spdlog/spdlog.h>

#include "app/AppMain.h"
#include "WindowsMinidump.h"

static LONG WINAPI handleException(_EXCEPTION_POINTERS* ep)
{
    spdlog::critical("Unhandled exception happened");
    if (WindowsCreateMinidump(ep)) {
        spdlog::critical("Minidump created");
    } else {
        spdlog::critical("Could not create minidump");
    }

    exit(1);
}

int main(int argc, char** argv)
{
    SetUnhandledExceptionFilter(handleException);
    return appMain(argc, argv);
}

#endif
