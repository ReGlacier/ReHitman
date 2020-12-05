#include <DLL/CrashHandlerReporter.h>
#include <Windows.h>
#include <DbgHelp.h>
#include <spdlog/spdlog.h>

bool isExceptionRequierMiniDump(EXCEPTION_POINTERS* frame)
{
    if (frame->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT  ||
        frame->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
    {
        return false;
    }
    return true;
}

void WriteMiniDump(EXCEPTION_POINTERS* exception = nullptr)
{
    //
    //	Credits https://stackoverflow.com/questions/5028781/how-to-write-a-sample-code-that-will-crash-and-produce-dump-file
    //
    auto hDbgHelp = LoadLibraryA("dbghelp");
    if (hDbgHelp == nullptr)
        return;
    auto pMiniDumpWriteDump = (decltype(&MiniDumpWriteDump))GetProcAddress(hDbgHelp, "MiniDumpWriteDump");
    if (pMiniDumpWriteDump == nullptr)
        return;

    char name[MAX_PATH];
    {
        auto nameEnd = name + GetModuleFileNameA(GetModuleHandleA(0), name, MAX_PATH);
        SYSTEMTIME t;
        GetSystemTime(&t);

        wsprintfA(nameEnd - strlen(".exe"),
                  "_%4d%02d%02d_%02d%02d%02d.dmp",
                  t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
    }

    auto hFile = CreateFileA(name, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        return;

    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
    exceptionInfo.ThreadId = GetCurrentThreadId();
    exceptionInfo.ExceptionPointers = exception;
    exceptionInfo.ClientPointers = FALSE;

    auto dumped = pMiniDumpWriteDump(
            GetCurrentProcess(),
            GetCurrentProcessId(),
            hFile,
            MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory),
            exception ? &exceptionInfo : nullptr,
            nullptr,
            nullptr);

    CloseHandle(hFile);
}

void NotifyAboutException(EXCEPTION_POINTERS* exceptionInfoFrame)
{
    spdlog::error("******************************************************************************");
    spdlog::error("                                 FATAL ERROR!								  ");
    spdlog::error("******************************************************************************");
    spdlog::error("Exception code   : {:08X}",   exceptionInfoFrame->ExceptionRecord->ExceptionCode);
    spdlog::error("Exception flags  : {:08X}",   exceptionInfoFrame->ExceptionRecord->ExceptionFlags);
    spdlog::error("Exception addr   : {:08X}", (DWORD)exceptionInfoFrame->ExceptionRecord->ExceptionAddress);
    spdlog::error("Registers:");
    spdlog::error("             EAX : {:08X}", exceptionInfoFrame->ContextRecord->Eax);
    spdlog::error("             EBX : {:08X}", exceptionInfoFrame->ContextRecord->Ebx);
    spdlog::error("             EDX : {:08X}", exceptionInfoFrame->ContextRecord->Edx);
    spdlog::error("             ECX : {:08X}", exceptionInfoFrame->ContextRecord->Ecx);
    spdlog::error("             EDI : {:08X}", exceptionInfoFrame->ContextRecord->Edi);
    spdlog::error("             ESI : {:08X}", exceptionInfoFrame->ContextRecord->Esi);
    spdlog::error("             EBP : {:08X}", exceptionInfoFrame->ContextRecord->Ebp);
    spdlog::error("             EIP : {:08X}", exceptionInfoFrame->ContextRecord->Eip);
    spdlog::error("             ESP : {:08X}", exceptionInfoFrame->ContextRecord->Esp);
    spdlog::error("******************************************************************************");

    MessageBox(
            NULL,
            "We got an fatal error.\nMinidump will be saved near exe.\nMore details in dev console.",
            "Hitman Blood Money | ReGlacier",
            MB_ICONERROR | MB_OK
    );

    WriteMiniDump(exceptionInfoFrame);
}

LONG WINAPI ExceptionFilterWin32(EXCEPTION_POINTERS* exceptionInfoFrame)
{
    if (isExceptionRequierMiniDump(exceptionInfoFrame))
    {
        NotifyAboutException(exceptionInfoFrame);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

LONG WINAPI VectoredExceptionHandlerWin32(EXCEPTION_POINTERS* exceptionInfoFrame)
{
    if (isExceptionRequierMiniDump(exceptionInfoFrame))
    {
        NotifyAboutException(exceptionInfoFrame);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

namespace ReHitman
{
    CrashHandlerReporter::CrashHandlerReporter()
    {
        m_prevHandler = reinterpret_cast<std::intptr_t>(SetUnhandledExceptionFilter(ExceptionFilterWin32));
    }

    CrashHandlerReporter::~CrashHandlerReporter()
    {
        SetUnhandledExceptionFilter(reinterpret_cast<LPTOP_LEVEL_EXCEPTION_FILTER>(m_prevHandler));
        if (!AddVectoredExceptionHandler(0UL, VectoredExceptionHandlerWin32))
        {
            spdlog::warn("AddVectoredExceptionHandler failed!");
        }
    }
}