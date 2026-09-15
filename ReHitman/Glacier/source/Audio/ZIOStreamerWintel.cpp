#include <Glacier/Audio/ZIOStreamerWintel.h>
#include <Glacier/ZUniAssert.h>

#include <Windows.h>

namespace Glacier
{
    namespace
    {
        // Our debug method to override thread name
        void SetCurrentThreadName(PCWSTR pszThreadName)
        {
            using SetThreadDescriptionProc = HRESULT(WINAPI*)(HANDLE, PCWSTR);
            const auto pSetThreadDescription = reinterpret_cast<SetThreadDescriptionProc>(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "SetThreadDescription"));

            if (pSetThreadDescription)
            {
                pSetThreadDescription(GetCurrentThread(), pszThreadName);
            }
        }

        DWORD WINAPI IOThread(void* _streamer)
        {
            SetCurrentThreadName(L"Glacier IO Thread");

            auto* streamer = static_cast<ZIOStreamerWintel*>(_streamer);
            HANDLE events[] = {streamer->m_hShutdownEvent, streamer->m_hIOEvent};

            WaitForMultipleObjects(2, events, false, INFINITE);
            while (WaitForSingleObject(streamer->m_hShutdownEvent, 0) != WAIT_OBJECT_0)
            {
                if (WaitForSingleObject(streamer->m_hIOEvent, 0) == WAIT_OBJECT_0)
                {
                    const auto file = reinterpret_cast<HANDLE>(streamer->m_CurrentStream.m_lFileHandle);
                    if (file)
                    {
                        LONG offsetHigh = 0;
                        SetFilePointer(file, streamer->m_CurrentStream.m_lLoadOffset, &offsetHigh, FILE_BEGIN);

                        DWORD bytesRead = 0;
                        if (!ReadFile(file, streamer->m_CurrentStream.m_pLoadBuffer,
                                streamer->m_CurrentStream.m_lLoadSize, &bytesRead, nullptr))
                        {
                            char message[1024];
                            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), 0, message,
                                sizeof(message), nullptr);
                            ZASSERT(false);
                        }
                        ResetEvent(streamer->m_hIOEvent);
                    }
                }
                WaitForMultipleObjects(2, events, false, INFINITE);
            }
            return 0;
        }
    }

    bool ZIOStreamerWintel::Create(int _numBlocks, int _blockSize, uint32_t _maxLatency)
    {
        if (!ZIOStreamer::Create(_numBlocks, _blockSize, _maxLatency))
            return false;

        m_hIOEvent = CreateEventA(nullptr, true, false, nullptr);
        m_hShutdownEvent = CreateEventA(nullptr, false, false, nullptr);
        m_hThread = CreateThread(nullptr, 0, IOThread, this, 0, nullptr);
        SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
        return true;
    }

    void ZIOStreamerWintel::Destroy()
    {
        if (m_hThread)
        {
            SetEvent(m_hShutdownEvent);
            WaitForSingleObject(m_hThread, 5000);
        }
        ZIOStreamer::Destroy();
    }

    uint32_t ZIOStreamerWintel::GetFileHandle(const char* _fileName)
    {
        if (!*_fileName)
            return 0;
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(CreateFileA(_fileName, GENERIC_READ,
            FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)));
    }

    bool ZIOStreamerWintel::Busy()
    {
        return WaitForSingleObject(m_hIOEvent, 0) == WAIT_OBJECT_0;
    }

    void ZIOStreamerWintel::IssueRead()
    {
        SetEvent(m_hIOEvent);
    }
}
