#include "WindowsMinidump.h"

#ifdef TC_WINDOWS

#include <dbghelp.h>
#include <tchar.h>
#include <windows.h>

bool WindowsCreateMinidump(_EXCEPTION_POINTERS* ep)
{
    HANDLE file = CreateFile(
        _T("minidump.dmp"),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (file == INVALID_HANDLE_VALUE) {
        return false;
    }

    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = ep;
    mei.ClientPointers = FALSE;

    MINIDUMP_CALLBACK_INFORMATION mci;
    mci.CallbackRoutine = nullptr;
    mci.CallbackParam = nullptr;

    using MiniDumpWriteDump = BOOL(WINAPI*)(
        HANDLE hProcess,
        DWORD ProcessId,
        HANDLE hFile,
        MINIDUMP_TYPE DumpType,
        CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
        CONST PMINIDUMP_USER_STREAM_INFORMATION UserEncoderParam,
        CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

    HMODULE dbgHelp = LoadLibrary(_T("dbghelp.dll"));
    if (!dbgHelp) {
        return false;
    }

    auto miniDumpWriteDump = (MiniDumpWriteDump)GetProcAddress(dbgHelp, "MiniDumpWriteDump");
    if (!miniDumpWriteDump) {
        return false;
    }

    HANDLE hProcess = GetCurrentProcess();
    DWORD dwProcessId = GetCurrentProcessId();

    BOOL bWriteDump = miniDumpWriteDump(
        hProcess,
        dwProcessId,
        file,
        MiniDumpNormal,
        &mei,
        nullptr,
        &mci);

    if (!bWriteDump) {
        return false;
    }

    CloseHandle(file);
    FreeLibrary(dbgHelp);

    return true;
}

#endif
