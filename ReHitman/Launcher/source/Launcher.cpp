#include <Windows.h>

#include <iostream>
#include <psapi.h>
#include <tchar.h>
#include <cassert>
#include <cstdint>
#include <vector>
#include <string>
#include <string_view>
#include <map>

#ifndef _WIN32
#error "Supported only x86"
#endif

#define TARGET_DLL_NAME				"ReHitman.dll"
#define TARGET_EXE_NAME				"HitmanBloodMoney.exe"

///Time in milliseconds for CoreKill initialize process (TODO: Rewrite to IPC in future)
#define CK_AWAIT_TIME 1000

bool SpawnProcess(const char* processName, const char* basePath, HANDLE& processHandle, DWORD& processId, HANDLE& processMainThread)
{
    SECURITY_ATTRIBUTES process_sa;
    SECURITY_ATTRIBUTES thread_sa;
    STARTUPINFO process_si;
    PROCESS_INFORMATION process_info;

    ZeroMemory(&process_sa, sizeof(process_sa));
    ZeroMemory(&thread_sa, sizeof(thread_sa));
    ZeroMemory(&process_si, sizeof(process_si));

    process_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    process_sa.lpSecurityDescriptor = NULL;
    process_sa.bInheritHandle = TRUE;

    thread_sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    thread_sa.lpSecurityDescriptor = NULL;
    thread_sa.bInheritHandle = TRUE;

    process_si.cb = sizeof(STARTUPINFO);
    process_si.lpReserved = NULL;
    process_si.lpReserved2 = NULL;

    LPTSTR cmd_line = _tcsdup(TEXT(processName));

    BOOL result = CreateProcess(
            NULL
            , cmd_line
            , &process_sa
            , &thread_sa
            , TRUE
            , CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED | DETACHED_PROCESS
            , NULL
            , basePath
            , &process_si
            , &process_info);

    if (!result)
    {
        return false;
    }
    else
    {
        processHandle = process_info.hProcess;
        processId = process_info.dwProcessId;
        processMainThread = process_info.hThread;
    }

    return true;
}

void InjectDLL(DWORD processId, HANDLE targetProcess, HANDLE processSuspendedThread)
{
    assert(targetProcess);

    DWORD written = 0;
    void* patch_dll_path_addr = nullptr;

    /// ALLOCATE MEMORY FOR PATH TO DLL
    patch_dll_path_addr = VirtualAllocEx(targetProcess, NULL, sizeof(TARGET_DLL_NAME), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    assert(patch_dll_path_addr);

    /// WRITE PATH TO DLL INTO ALREADY ALLOCATED BUFFER
    BOOL store_dll_path_result = WriteProcessMemory(targetProcess, (LPVOID)patch_dll_path_addr, TARGET_DLL_NAME, sizeof(TARGET_DLL_NAME), &written);
    assert(store_dll_path_result && written);

    /// CREATE HANDLER FOR THE PROCESS
    SECURITY_ATTRIBUTES sa;
    DWORD remoteThreadId = 0;
    HANDLE threadHandle;

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;

    /// CREATE REMOTE THREAD FOR LOAD AND INIT OUR DLL
    threadHandle = CreateRemoteThread(targetProcess, &sa, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, patch_dll_path_addr, 0, &remoteThreadId);
    assert(threadHandle);

    std::cout << "[Launcher] wait for ready status from CoreKill component ...\n";

    WaitForSingleObject(threadHandle, INFINITY);
    DWORD exitCode = 0;
    if (GetExitCodeThread(threadHandle, &exitCode)) {
        std::cout << "Remote thread done (exit code: " << exitCode << ")\n";
        CloseHandle(threadHandle);	/// CLOSE THREAD HANDLER
    }

    Sleep(CK_AWAIT_TIME);

    /// OUR 'DEBUG' FEATURE.
    /// THE PROCESS WILL BE PAUSED BEFORE ANY KEY NOT PRESSED.
    /// YOU CAN READ ANY PARTS OF DATA FROM MEMORY, BY CHEAT ENGINE OR OTHER TOOL,
    /// AND ANALYZE POSSIBLE ERRORS BEFORE PROCESS WILL BE CRASHED.
#ifdef HM3_DEBUG_ENABLED
    std::cout << " < PROCESS PAUSED | DEBUG WHAT YOU WANT > \n";
	system("pause");
#endif

    /// IN ANY CASE WE MUST RELEASE ALLOCATED MEMORY AND RESUME PROCESS THREAD
    VirtualFreeEx(targetProcess, patch_dll_path_addr, 0, MEM_RELEASE);
    ResumeThread(processSuspendedThread);
}

int main(int, char**)
{
    /// NOTE: WORKING DIRECTORY MUST BE LOCATED NEAR HitmanBloodMoney.exe!
    char currentDirectoryBuffer[256];
    char pathToExeBuffer[512];
    char pathToDllBuffer[512];

    GetCurrentDirectory(256, (LPSTR)currentDirectoryBuffer);

    sprintf(pathToExeBuffer, "%s\\%s", currentDirectoryBuffer, TARGET_EXE_NAME);
    sprintf(pathToDllBuffer, "%s\\%s", currentDirectoryBuffer, TARGET_DLL_NAME);

    std::cout << "Found EXE: " << pathToExeBuffer << '\n';
    std::cout << "Found DLL: " << pathToDllBuffer << '\n';

    DWORD spawnedProcessId = 0;
    HANDLE spawnedHandle = 0;
    HANDLE spawnedThread = 0;
    bool spawnResult = SpawnProcess(pathToExeBuffer, currentDirectoryBuffer, spawnedHandle, spawnedProcessId, spawnedThread);
    assert(spawnResult);

    std::cout << "Target process ID is " << spawnedProcessId << "\n";
    InjectDLL(spawnedProcessId, spawnedHandle, spawnedThread);

    std::cout << "Game process started!\n";
    WaitForSingleObject(spawnedHandle, INFINITE);
    return 0;
}