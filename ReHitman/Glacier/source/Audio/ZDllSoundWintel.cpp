#include <Glacier/Audio/ZDllSoundWintel.h>

#include <Glacier/Audio/GlobalsWintel.h>
#include <Glacier/Audio/ZSynthDS.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>

namespace Glacier
{
    namespace
    {
        SSynthCaps g_DirectSoundCaps{};
        char g_MetaMemory[8 * 0x1000]{};

        DWORD WINAPI SynthThread(void* _parameter)
        {
            auto* synth = static_cast<ZSynthWintel*>(_parameter);
            while (synth->m_bRunning)
            {
                if (synth->CmdFrameReady())
                {
                    synth->CommitFrame();
                    synth->SignalCmdFrameProcessed();
                }
                if (!synth->m_bRunning)
                    break;
                synth->Render();
                Sleep(12);
            }
            return 0;
        }
    }

    ZDllSoundWintel::ZDllSoundWintel()
        : m_lNumSynths(0)
        , m_pMetaMemory(g_MetaMemory)
        , m_lSelectedSynth(0)
        , m_pSynth(nullptr)
        , m_hSynthThread(nullptr)
    {
    }

    ZDllSoundWintel::~ZDllSoundWintel()
    {
        FreeSynth();
        if (g_hFrameReady)
        {
            CloseHandle(g_hFrameReady);
            g_hFrameReady = nullptr;
        }
        if (g_hSoundEngineReady)
        {
            CloseHandle(g_hSoundEngineReady);
            g_hSoundEngineReady = nullptr;
        }
        if (g_hCmdFrameProcessed)
        {
            CloseHandle(g_hCmdFrameProcessed);
            g_hCmdFrameProcessed = nullptr;
        }
    }

    ZDllSoundWintel* ZDllSoundWintel::BuildInstance()
    {
        return ZUniMemory::New<ZDllSoundWintel>();
    }

    void ZDllSoundWintel::End()
    {
        FreeSynth();
    }

    void ZDllSoundWintel::Initialize()
    {
        ZSynthDS probe;
        if (probe.Probe())
        {
            g_DirectSoundCaps = *probe.GetCaps();
            g_DirectSoundCaps.m_lIndex = 0;
            m_lNumSynths = 1;
        }

        // TODO: Finish this place after ZSynthOpenAL will be reversed
        // Probe and append the OpenAL synthesizer capabilities here.

        // TODO: Finish this place after ZSynthDP will be reversed
        // Probe and append the DieselPower synthesizer capabilities here.

        g_hSoundEngineReady = CreateEventA(nullptr, false, true, nullptr);
        g_hCmdFrameProcessed = CreateEventA(nullptr, true, true, nullptr);
    }

    bool ZDllSoundWintel::InstallSynthesizer()
    {
        if (!m_lNumSynths)
            return false;

        FreeSynth();
        m_pSynth = ZUniMemory::New<ZSynthDS>();
        m_lSelectedSynth = 0;
        m_pSynth->m_pPoseDestAddr = m_pMetaMemory;

        if (!m_pSynth->Initialize())
        {
            ZUniMemory::Delete(m_pSynth);
            m_pSynth = nullptr;
            return false;
        }

        DWORD threadId = 0;
        m_hSynthThread = CreateThread(nullptr, 0, SynthThread, m_pSynth, 0, &threadId);
        if (!m_hSynthThread)
        {
            m_pSynth->Free();
            ZUniMemory::Delete(m_pSynth);
            m_pSynth = nullptr;
            return false;
        }
        SetThreadPriority(m_hSynthThread, THREAD_PRIORITY_HIGHEST);
        m_bSoundInitialized = true;
        return true;
    }

    bool ZDllSoundWintel::RenderFrame()
    {
        return ZDllSound::RenderFrame(m_RefTabB);
    }

    void ZDllSoundWintel::AddSourceCommand(ZSoundObject* _object, SStartSoundBase* _command)
    {
        if (!_object || !_command)
            return;

        _command->m_lPitch = _object->GetCalculatedPitch();
        reinterpret_cast<SStartSound*>(_command)->m_fVolume =
            _object->GetCalculatedVolumeAttenuation();
        _command->m_lBufferType = _object->m_eSourceType;

        switch (_object->m_eSourceType)
        {
        case 0:
        case 10:
        {
            auto* command3D = reinterpret_cast<SStartSound3D*>(_command);
            command3D->m_lType = 2;
            command3D->m_lSize = sizeof(SStartSound3D);
            command3D->m_lBufferType = _object->m_eSourceType;
            command3D->m_vPosition = _object->m_vPosition;
            command3D->m_vVelocity = {};
            command3D->m_vConeOrientation = _object->m_vOrientation;
            command3D->m_fMinDistance = _object->m_fMinDist;
            command3D->m_fMaxDistance = _object->m_fMaxDist;
            command3D->m_fInnerConeAngle = _object->m_fInnerConeAngle;
            command3D->m_fOuterConeAngle = _object->m_fOuterConeAngle;
            command3D->m_fOuterConeVolume = static_cast<float>(_object->m_OuterConeVolume) * 100.0f;
            break;
        }
        case 11:
        {
            auto* commandBFormat = reinterpret_cast<SStartSoundBFormat*>(_command);
            commandBFormat->m_lType = 9;
            commandBFormat->m_lSize = sizeof(SStartSoundBFormat);
            commandBFormat->m_lBufferType = 11;
            commandBFormat->m_fW = _object->m_vBFormat.x;
            commandBFormat->m_fX = _object->m_vBFormat.y;
            commandBFormat->m_fY = _object->m_vBFormat.z;
            commandBFormat->m_fZ = _object->m_vBFormat.w;
            break;
        }
        default:
        {
            auto* command2D = reinterpret_cast<SStartSound2D*>(_command);
            command2D->m_lType = 1;
            command2D->m_lSize = sizeof(SStartSound2D);
            command2D->m_lBufferType = 2;
            command2D->m_lPan = _object->m_lPan;
            break;
        }
        }
    }

    void ZDllSoundWintel::AddListenerCommands()
    {
        auto* command = reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd);
        command->m_lType = 0x20;
        command->m_lSize = 0x44;
        auto* vectors = reinterpret_cast<ZVector3*>(m_pSynthCmd + 8);
        vectors[0] = m_ListenerMatrix.ZAxis();
        vectors[1] = m_ListenerMatrix.YAxis();
        vectors[2] = m_ListenerMatrix.XAxis();
        vectors[3] = m_ListenerPosition;
        vectors[4] = m_ListenerVelocity;
        m_pSynthCmd += command->m_lSize;
    }

    void ZDllSoundWintel::FreeSynth()
    {
        if (!m_pSynth)
            return;

        m_pSynth->m_bRunning = false;
        if (g_hCmdFrameProcessed)
            SetEvent(g_hCmdFrameProcessed);
        if (m_hSynthThread)
        {
            WaitForSingleObject(m_hSynthThread, 5000);
            CloseHandle(m_hSynthThread);
            m_hSynthThread = nullptr;
        }
        m_pSynth->Free();
        ZUniMemory::Delete(m_pSynth);
        m_pSynth = nullptr;
        m_bSoundInitialized = false;
    }

    char* ZDllSoundWintel::GetSynthCmdArray()
    {
        return m_pSynth ? m_pSynth->GetResultFrameBuffer() : nullptr;
    }

    void ZDllSoundWintel::ProcessSynthCmdArray()
    {
        if (!m_pSynth)
            return;

        char* command = GetSynthCmdArray();
        while (command && reinterpret_cast<SSynthCmdBase*>(command)->m_lType != 0x10000)
        {
            ProcessSynthCmd(command);
            command += reinterpret_cast<SSynthCmdBase*>(command)->m_lSize;
        }
        *reinterpret_cast<uint32_t*>(GetSynthCmdArray()) = 0x10000;
        SignalSoundEngineReady();
    }

    bool ZDllSoundWintel::ProcessSynthCmd(const char* _command)
    {
        if (!_command)
            return false;

        const auto* command = reinterpret_cast<const SSynthCmdBase*>(_command);
        switch (command->m_lType)
        {
        case 0x1001:
        {
            const auto* result = reinterpret_cast<const uint32_t*>(_command + 8);
            ZSoundObject* object = m_ObjectManager.ConvRefToPtr(*result);
            if (object && static_cast<int32_t>(object->m_lSoundFlags) >= 0)
            {
                object->m_lSoundFlags &= ~0x40000u;
                const float latency = g_pSysInterface ?
                    static_cast<float>(g_pSysInterface->m_fRealTime.secs -
                        object->m_fCreateTime.secs) * (1.0f / 1024.0f) : 0.0f;
                object->m_fLatency = latency;
                object->m_fTimeLeft = latency + object->m_fTimeLeft - object->m_fDelay - 2.0f;
                object->NotifyStarted();
                if ((object->m_lSoundFlags & 0x20000000) != 0 && object->m_lChainIdxNotifyStarted)
                {
                    FreeRefChain(object->m_lChainIdxNotifyStarted);
                    object->m_lChainIdxNotifyStarted = 0;
                }
            }
            return true;
        }
        case 0x1002:
        {
            const auto* result = reinterpret_cast<const uint32_t*>(_command + 8);
            ZSoundObject* object = m_ObjectManager.ConvRefToPtr(*result);
            if (!object)
                return true;

            // TODO: Finish this place after ZSoundObject::NotifyTarget will be reversed
            object->Stopped();
            if ((object->m_lSoundFlags & 8) == 0)
            {
                bool groupStillPlaying = false;
                for (const SRefLink* link = m_AllocRef.GetLink(object->m_lChainIdxGroup);
                    link; link = m_AllocRef.GetLink(link->m_lNext))
                {
                    ZSoundObject* grouped = m_ObjectManager.ConvRefToPtr(link->m_rRef);
                    if (grouped && grouped != object && grouped->m_eState == STATE_PLAYING)
                    {
                        groupStillPlaying = true;
                        object->m_lSoundFlags |= 0x80000000u;
                    }
                }
                if (object->m_eState != STATE_DISABLED && !groupStillPlaying)
                    object->m_eState = STATE_DEPRICATED;
            }
            return true;
        }
        case 0x1003:
        {
            const auto* result = reinterpret_cast<const uint32_t*>(_command + 8);
            ZSoundObject* object = m_ObjectManager.ConvRefToPtr(*result);
            if (object)
            {
                object->m_fTimeLeft += 2.0f;
                object->m_lSoundFlags |= 0x40000u;
            }
            return true;
        }
        case 0x1004:
        {
            const auto* result = reinterpret_cast<const SBufferId*>(_command + 8);
            ZSoundObject* object = m_ObjectManager.ConvRefToPtr(result->m_lGeomRef);
            if (object)
                object->SetBufferId(static_cast<char>(result->m_lBufferId));
            return true;
        }
        case 0x1008:
        {
            const auto* result = reinterpret_cast<const SBufferId*>(_command + 8);
            ZSoundObject* object = m_ObjectManager.ConvRefToPtr(result->m_lGeomRef);
            if (object && m_pMetaMemory)
                object->SetPoseData(m_pMetaMemory + (result->m_lBufferId << 12));
            return true;
        }
        default:
            return false;
        }
    }

    bool ZDllSoundWintel::SynthModuleReady()
    {
        return WaitForSingleObject(g_hCmdFrameProcessed, 0) == WAIT_OBJECT_0;
    }

    void ZDllSoundWintel::ResetSynthReady()
    {
        ResetEvent(g_hCmdFrameProcessed);
    }

    void ZDllSoundWintel::SignalSoundEngineReady()
    {
        SetEvent(g_hSoundEngineReady);
    }

    bool ZDllSoundWintel::FrameReady()
    {
        return WaitForSingleObject(g_hFrameReady, 0) == WAIT_OBJECT_0;
    }

    bool ZDllSoundWintel::SendCmdFrameBuffer()
    {
        if (!m_bSoundInitialized || !m_pSynthCmd || !m_pSynthCmdBuffer ||
            m_pSynthCmd - m_pSynthCmdBuffer > m_lSynthCmdBufferSize)
        {
            return false;
        }
        ZASSERT(reinterpret_cast<SSynthCmdBase*>(m_pSynthCmd)->m_lType == 0x10000);
        ResetSynthReady();
        return true;
    }

    int32_t ZDllSoundWintel::GetNumSynths()
    {
        return m_lNumSynths;
    }

    SSynthCaps* ZDllSoundWintel::GetSynthCaps(const char* _name)
    {
        if (!_name || _stricmp(_name, g_DirectSoundCaps.m_szName) == 0)
            return GetSynthCaps(0);
        return GetSynthCaps(0);
    }

    SSynthCaps* ZDllSoundWintel::GetSynthCaps(int _index)
    {
        return m_lNumSynths && _index == 0 ? &g_DirectSoundCaps : nullptr;
    }

    int32_t ZDllSoundWintel::GetSynthIndex(const char* _name)
    {
        SSynthCaps* caps = GetSynthCaps(_name);
        return caps ? caps->m_lIndex : 0;
    }

    void ZDllSoundWintel::DisplaySynthStatus()
    {
        for (int i = 0; i < m_lNumSynths; ++i)
            GetSynthCaps(i);
    }

    void ZDllSoundWintel::UpdateSoundEngine()
    {
        SignalSoundEngineReady();
    }

    IDirectSound8* ZDllSoundWintel::GetDirectSound()
    {
        return m_pSynth ? static_cast<ZSynthDS*>(m_pSynth)->GetDS() : nullptr;
    }
}
