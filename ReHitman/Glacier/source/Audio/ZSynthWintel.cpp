#include <Glacier/Audio/GlobalsWintel.h>
#include <Glacier/Audio/ZIOStreamerWintel.h>
#include <Glacier/Audio/ZSynthWintel.h>
#include <Glacier/Audio/Eax3.h>
#include <Glacier/Audio/ZWintelRoomReverb.h>
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
    void ZSynthWintel::GetEaxProps(EaxListenerProperties* _eax, const ZWintelRoomReverb* _environment)
    {
        _eax->m_lEnvironment = _environment->m_lEnvironment;
        _eax->m_fEnvironmentSize = _environment->m_fEnvironmentSize;
        _eax->m_fEnvironmentDiffusion = _environment->m_fEnvironmentDiffusion;
        _eax->m_lRoom = _environment->m_lRoom;
        _eax->m_lRoomHF = _environment->m_lRoomHF;
        _eax->m_lRoomLF = _environment->m_lRoomLF;
        _eax->m_fDecayTime = _environment->m_fDecayTime;
        _eax->m_fDecayHFRatio = _environment->m_fDecayHFRatio;
        _eax->m_fDecayLFRatio = _environment->m_fDecayLFRatio;
        _eax->m_lReflections = _environment->m_lReflections;
        _eax->m_fReflectionsDelay = _environment->m_fReflectionsDelay;
        _eax->m_lReverb = _environment->m_lReverb;
        _eax->m_fReverbDelay = _environment->m_fReverbDelay;
        _eax->m_fEchoTime = _environment->m_fEchoTime;
        _eax->m_fEchoDepth = _environment->m_fEchoDepth;
        _eax->m_fModulationTime = _environment->m_fModulationTime;
        _eax->m_fModulationDepth = _environment->m_fModulationDepth;
        _eax->m_fAirAbsorptionHF = _environment->m_fAirAbsorptionHF;
        _eax->m_fHFReference = _environment->m_fHFReference;
        _eax->m_fLFReference = _environment->m_fLFReference;
        _eax->m_fRoomRolloffFactor = _environment->m_fRoomRolloffFactor;
        _eax->m_lFlags = _environment->m_lFlags;
    }

    void ZSynthWintel::GetEnvProps(ZWintelRoomReverb* _environment, const EaxListenerProperties* _eax)
    {
        _environment->m_lEnvironment = _eax->m_lEnvironment;
        _environment->m_fEnvironmentSize = _eax->m_fEnvironmentSize;
        _environment->m_fEnvironmentDiffusion = _eax->m_fEnvironmentDiffusion;
        _environment->m_lRoom = _eax->m_lRoom;
        _environment->m_lRoomHF = _eax->m_lRoomHF;
        _environment->m_lRoomLF = _eax->m_lRoomLF;
        _environment->m_fDecayTime = _eax->m_fDecayTime;
        _environment->m_fDecayHFRatio = _eax->m_fDecayHFRatio;
        _environment->m_fDecayLFRatio = _eax->m_fDecayLFRatio;
        _environment->m_lReflections = _eax->m_lReflections;
        _environment->m_fReflectionsDelay = _eax->m_fReflectionsDelay;
        _environment->m_lReverb = _eax->m_lReverb;
        _environment->m_fReverbDelay = _eax->m_fReverbDelay;
        _environment->m_fEchoTime = _eax->m_fEchoTime;
        _environment->m_fEchoDepth = _eax->m_fEchoDepth;
        _environment->m_fModulationTime = _eax->m_fModulationTime;
        _environment->m_fModulationDepth = _eax->m_fModulationDepth;
        _environment->m_fAirAbsorptionHF = _eax->m_fAirAbsorptionHF;
        _environment->m_fHFReference = _eax->m_fHFReference;
        _environment->m_fLFReference = _eax->m_fLFReference;
        _environment->m_fRoomRolloffFactor = _eax->m_fRoomRolloffFactor;
        _environment->m_lFlags = _eax->m_lFlags;
    }

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
        m_pStreamer = ZUniMemory::New<ZIOStreamerWintel>();
        m_pStreamer->Create(128, 0x4000, 300);
        m_pStreamer->SetMetaMem(m_pPoseDestAddr);
        return true;
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

    void ZSynthWintel::CreateChain(SChain*, int)
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
        if (!g_pSysInterface)
            return GetDesktopWindow();
        if (g_pSysInterface->WindowFirst)
            return static_cast<ZRenderX86*>(g_pSysInterface->WindowFirst)->m_hWnd;
        return g_pSysInterface->MainhWnd ? g_pSysInterface->MainhWnd : GetDesktopWindow();
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
        SSynthCaps* caps = GetCaps();
        ZASSERT(caps != nullptr);
        const int maximum = caps->m_lMaxBuffers;
        m_lNumBuffers = _count <= maximum ? _count : maximum;
        m_lMaxNumPlaying = m_lNumBuffers;
        return m_lNumBuffers;
    }

    bool ZSynthWintel::SetUseEAX(bool _enabled)
    {
        SSynthCaps* caps = GetCaps();
        ZASSERT(caps != nullptr);
        m_bUseEAX = _enabled && caps->m_bEAX;
        return m_bUseEAX;
    }

    SSynthCaps* ZSynthWintel::GetCaps()
    {
        return nullptr;
    }

    int ZSynthWintel::Probe()
    {
        return 0;
    }
}
