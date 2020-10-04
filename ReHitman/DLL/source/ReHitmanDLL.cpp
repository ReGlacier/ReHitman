/**
 * @brief DLL Entry point for our project
 */
#include <Windows.h>

#include <DLL/ReHitman.h>

#ifndef _WIN32
#error "Supported only x86"
#endif

DWORD g_dwWorkerThreadId = 0x0;

static constexpr auto kDefaultStackSize = 0x0;

DWORD WINAPI WorkerThread(void* arg)
{
    return ReHitman::Core::EntryPoint(arg);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        CreateThread(nullptr, kDefaultStackSize, (LPTHREAD_START_ROUTINE)WorkerThread, nullptr, 0, &g_dwWorkerThreadId);
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        FreeLibraryAndExitThread(hinstDLL, 0);
    }

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}