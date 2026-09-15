#include "BinkVideoInterface.h"

#include <cstdio>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace BinkVideoInterface
{
    namespace
    {
        template <typename T>
        T Resolve(void* hModule, const char* pszName)
        {
#if defined(_WIN32)
            FARPROC pAddress = GetProcAddress(static_cast<HMODULE>(hModule), pszName);
            return reinterpret_cast<T>(pAddress);
#else
            (void)hModule;
            (void)pszName;
            return nullptr;
#endif
        }

        template <typename T>
        T ResolveEither(void* hModule, const char* pszDecorated, const char* pszPlain)
        {
            T pResult = Resolve<T>(hModule, pszDecorated);
            return pResult ? pResult : Resolve<T>(hModule, pszPlain);
        }
    }

    Interface& Interface::Instance()
    {
        static Interface s_Instance;
        return s_Instance;
    }

    Interface::Interface()
        : m_hModule(nullptr)
        , m_pOpen(nullptr)
        , m_pClose(nullptr)
        , m_pWait(nullptr)
        , m_pDoFrame(nullptr)
        , m_pCopyToBuffer(nullptr)
        , m_pNextFrame(nullptr)
        , m_pSetSoundSystem(nullptr)
        , m_pOpenDirectSound(nullptr)
        , m_pSetVolume(nullptr)
    {
#if defined(_WIN32)
        m_hModule = LoadLibraryA("binkw32.dll");
        if (!m_hModule)
        {
            std::fprintf(stderr, "BinkVideoInterface: unable to load binkw32.dll\n");
            return;
        }

        m_pOpen = ResolveEither<decltype(m_pOpen)>(m_hModule, "_BinkOpen@8", "BinkOpen");
        m_pClose = ResolveEither<decltype(m_pClose)>(m_hModule, "_BinkClose@4", "BinkClose");
        m_pWait = ResolveEither<decltype(m_pWait)>(m_hModule, "_BinkWait@4", "BinkWait");
        m_pDoFrame = ResolveEither<decltype(m_pDoFrame)>(m_hModule, "_BinkDoFrame@4", "BinkDoFrame");
        m_pCopyToBuffer = ResolveEither<decltype(m_pCopyToBuffer)>(m_hModule, "_BinkCopyToBuffer@28", "BinkCopyToBuffer");
        m_pNextFrame = ResolveEither<decltype(m_pNextFrame)>(m_hModule, "_BinkNextFrame@4", "BinkNextFrame");
        m_pSetSoundSystem = ResolveEither<decltype(m_pSetSoundSystem)>(m_hModule, "_BinkSetSoundSystem@8", "BinkSetSoundSystem");
        m_pOpenDirectSound = ResolveEither<decltype(m_pOpenDirectSound)>(m_hModule, "_BinkOpenDirectSound@12", "BinkOpenDirectSound");
        m_pSetVolume = ResolveEither<decltype(m_pSetVolume)>(m_hModule, "_BinkSetVolume@12", "BinkSetVolume");

        if (!IsAvailable())
            std::fprintf(stderr, "BinkVideoInterface: binkw32.dll is missing required exports\n");
#endif
    }

    Interface::~Interface()
    {
#if defined(_WIN32)
        if (m_hModule)
            FreeLibrary(static_cast<HMODULE>(m_hModule));
#endif
    }

    bool Interface::IsAvailable() const
    {
        return m_hModule && m_pOpen && m_pClose && m_pWait && m_pDoFrame
            && m_pCopyToBuffer && m_pNextFrame && m_pSetVolume;
    }

    HBINK Interface::Open(const char* pszFileName, uint32_t lFlags) { return IsAvailable() ? m_pOpen(pszFileName, lFlags) : nullptr; }
    bool Interface::GetInfo(HBINK pVideo, VideoInfo& rInfo) const
    {
        if (!pVideo)
            return false;
        const auto* pValues = reinterpret_cast<const uint32_t*>(pVideo);
        rInfo = {pValues[0], pValues[1], pValues[2], pValues[3]};
        return true;
    }
    void Interface::Close(HBINK pVideo) { if (pVideo && m_pClose) m_pClose(pVideo); }
    bool Interface::Wait(HBINK pVideo) { return !pVideo || !m_pWait || m_pWait(pVideo) != 0; }
    void Interface::DoFrame(HBINK pVideo) { if (pVideo && m_pDoFrame) m_pDoFrame(pVideo); }
    void Interface::CopyToBuffer(HBINK pVideo, void* pBuffer, int lPitch, int lHeight, int lDestX, int lDestY, uint32_t lFlags)
    { if (pVideo && m_pCopyToBuffer) m_pCopyToBuffer(pVideo, pBuffer, lPitch, lHeight, lDestX, lDestY, lFlags); }
    void Interface::NextFrame(HBINK pVideo) { if (pVideo && m_pNextFrame) m_pNextFrame(pVideo); }
    void Interface::SetSoundSystem(void* pOpenDirectSound, void* pSoundSystem)
    { if (m_pSetSoundSystem) m_pSetSoundSystem(pOpenDirectSound, pSoundSystem); }
    void* Interface::OpenDirectSound(uint32_t lRate, uint32_t lBits, uint32_t lChannels)
    { return m_pOpenDirectSound ? m_pOpenDirectSound(lRate, lBits, lChannels) : nullptr; }
    void Interface::SetVolume(HBINK pVideo, uint32_t lTrack, uint32_t lVolume)
    { if (pVideo && m_pSetVolume) m_pSetVolume(pVideo, lTrack, lVolume); }
}
