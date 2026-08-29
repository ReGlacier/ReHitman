#include <Glacier/Audio/GlobalsWintel.h>
#include <Glacier/Audio/ZSynthWintel.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/Render/ZRenderX86.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <cstdio>
#include <cstring>
#include <Windows.h>


namespace Glacier
{
    ZSynthWintel::ZSynthWintel()
        : m_field1BB1C(0)
        , m_field1BB28(false)
        , m_bUseEAX(false)
        , m_pWaveData(nullptr)
        , m_bDisplayStatus(false)
    {
    }

    ZSynthWintel::~ZSynthWintel() = default;

    bool ZSynthWintel::Initialize()
    {
        g_hFrameReady = CreateEventA(nullptr, false, false, nullptr);

        if (!ZSynth::Initialize())
        {
            return false;
        }

        return CreateListener();
    }

    void ZSynthWintel::Reset()
    {
        if (m_pWaveData)
        {
            ZUniMemory::Free(m_pWaveData);
            m_pWaveData = nullptr;
        }
        ZSynth::Reset();
    }

    void ZSynthWintel::Free()
    {
        ZSynth::Free();
    }

    bool ZSynthWintel::CreateSoundStreamer()
    {
        // TODO: Finish this place after ZIOStreamerWintel will be reversed
        return false;
    }

    void ZSynthWintel::InstallWaveHeaders(int _size, const char* _fileName)
    {
        m_pWaveHeaders = static_cast<char*>(ZUniMemory::Allocate(_size));
        g_pSysFile->Load(_fileName, m_pWaveHeaders, _size, 0, false);
    }

    void ZSynthWintel::InstallWaves(int _size, const char* _fileName)
    {
        m_pWaveData = static_cast<char*>(ZUniMemory::Allocate(_size));
        g_pSysFile->Load(_fileName, m_pWaveData, _size, 0, false);
    }

    void ZSynthWintel::CommitFrame()
    {
        ZSynth::CommitFrame();
        if (m_bDisplayStatus)
            DisplayStatus();
    }

    void ZSynthWintel::Render()
    {
        if (!g_pSysInterface->m_lIsActive)
            return;

        if (m_pStreamer)
            m_pStreamer->Update();
        ZSynth::Render();
    }

    void ZSynthWintel::CreateFilterChains()
    {
        for (int i = 0; i < m_lNumChains; ++i)
        {
            char description[512]{};
            std::sprintf(description, "chain %2d: ", i);

            const SChain& chain = m_Chains[i];
            for (int filterIndex = 0; filterIndex < chain.m_lNumFilters; ++filterIndex)
            {
                const char* filterName = "Unknown ";
                switch (chain.m_Filters[filterIndex].m_lType)
                {
                case 1:
                    filterName = "lowp ";
                    break;
                case 2:
                    filterName = "reverb ";
                    break;
                case 3:
                    filterName = "default reverb ";
                    break;
                }
                std::strcat(description, filterName);
            }
        }
    }

    void ZSynthWintel::CreateChain(void*, int)
    {
    }

    void ZSynthWintel::CreateChainsEnd()
    {
    }

    void ZSynthWintel::RemoveChains(int* _chains, int _count)
    {
        for (int i = 0; i < _count; ++i)
            RemoveChain(_chains[i]);
    }

    void ZSynthWintel::RemoveChain(int)
    {
    }

    bool ZSynthWintel::SoundEngineReady()
    {
        return WaitForSingleObject(g_hSoundEngineReady, 0) == 0;
    }

    void ZSynthWintel::SignalFrameReady()
    {
        SetEvent(g_hFrameReady);
    }

    bool ZSynthWintel::CmdFrameReady()
    {
        return WaitForSingleObject(g_hCmdFrameProcessed, 0) == WAIT_TIMEOUT;
    }

    void ZSynthWintel::SignalCmdFrameProcessed()
    {
        SetEvent(g_hCmdFrameProcessed);
    }

    bool ZSynthWintel::CreateListener()
    {
        return true;
    }

    void* ZSynthWintel::GetWindowHandle()
    {
        if (g_pSysInterface->WindowFirst)
            return static_cast<ZRenderX86*>(g_pSysInterface->WindowFirst)->m_hWnd;
        return g_pSysInterface->MainhWnd;
    }

    char* ZSynthWintel::GetWaveform(int _offset)
    {
        return m_pWaveData && _offset ? m_pWaveData + _offset : nullptr;
    }

    void ZSynthWintel::DisplayStatus()
    {
        for (int i = 0; i < m_lNumBuffers; ++i)
        {
            if (m_pBuffers[i] && m_pBuffers[i]->m_rWave)
            {
                const char* objectName = m_pWaveHeaders + m_pBuffers[i]->m_rWave->m_field04;
                (void)objectName;
            }
        }
    }

    int ZSynthWintel::SetNumBuffers(int _count)
    {
        void* caps = GetCaps();
        ZASSERT(caps != nullptr);
        const int maximum = *reinterpret_cast<int*>(static_cast<char*>(caps) + 196);
        m_lNumBuffers = _count <= maximum ? _count : maximum;
        m_lMaxNumPlaying = m_lNumBuffers;
        return m_lNumBuffers;
    }

    bool ZSynthWintel::SetUseEAX(bool _enabled)
    {
        void* caps = GetCaps();
        ZASSERT(caps != nullptr);
        m_bUseEAX = _enabled && *reinterpret_cast<bool*>(static_cast<char*>(caps) + 200);
        return m_bUseEAX;
    }

    void* ZSynthWintel::GetCaps()
    {
        return nullptr;
    }

    int ZSynthWintel::Probe()
    {
        return 0;
    }
}
