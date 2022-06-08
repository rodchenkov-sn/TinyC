#include "WindowsInit.h"

#ifdef TC_WINDOWS

// order matters
// clang-format off
#include <tchar.h>
#include <windows.h>
#include <dbghelp.h>
// clang-format on

class WindowsExceptionHandler {
public:
    explicit WindowsExceptionHandler(bool createDump)
        : create_dump_(createDump)
    {
    }

    void handleSEH(PEXCEPTION_POINTERS ep)
    {
        TC_LOG_CRITICAL("unhandled exception happened");

        if (create_dump_) {
            if (auto dmpPathName = createMinidump(ep, "minidump.dmp")) {
                // filename starts with "\\?\"
                TC_LOG_INFO("dump -> {}", dmpPathName->substr(4));
            } else {
                TC_LOG_WARN("could not create dump");
            }
        } else {
            TC_LOG_INFO("please rerun with --dump to create dump file");
        }
    }

private:
    static std::optional<std::string> createMinidump(_EXCEPTION_POINTERS* ep, std::string_view fileName)
    {
        HANDLE file = CreateFileA(
            fileName.data(),
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (file == INVALID_HANDLE_VALUE) {
            return std::nullopt;
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
            return std::nullopt;
        }

        auto miniDumpWriteDump = (MiniDumpWriteDump)GetProcAddress(dbgHelp, "MiniDumpWriteDump");
        if (!miniDumpWriteDump) {
            return std::nullopt;
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
            return std::nullopt;
        }

        std::string dmpFilePathName;

        const auto len = GetFinalPathNameByHandleA(file, nullptr, 0, FILE_NAME_NORMALIZED);

        dmpFilePathName.resize(len + 1);

        GetFinalPathNameByHandleA(file, dmpFilePathName.data(), dmpFilePathName.size(), FILE_NAME_NORMALIZED);

        CloseHandle(file);
        FreeLibrary(dbgHelp);

        return dmpFilePathName;
    }

    bool create_dump_;
};

static WindowsExceptionHandler* eh = nullptr;

static LONG WINAPI handleException(PEXCEPTION_POINTERS ep)
{
    eh->handleSEH(ep);

    exit(1);
};

void windowsInit(bool dumpOnCrash)
{
    eh = new WindowsExceptionHandler(dumpOnCrash);
    SetUnhandledExceptionFilter(handleException);
}

#endif// TC_WINDOWS
