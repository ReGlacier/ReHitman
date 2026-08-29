#include <Glacier/Audio/ZSynth.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>

namespace Glacier
{
    ZSynth::ZSynth()
        : m_lMaxNumGroupedPlaying(4)
        , m_pLayerData(nullptr)
        , m_lLayerSize(0)
        , m_pBuffers(nullptr)
        , m_pStreamer(nullptr)
        , m_lNumChains(0)
        , m_lFmvVolume(100)
        , m_lTransferRequestCnt(0)
        , m_bDriveReadyRequest(false)
        , m_bDriveReady(false)
        , m_bDriveLocked(false)
        , m_bMemStreamReady(false)
        , m_bMemStreamRunning(false)
        , m_bMemStreamNeedData(false)
        , m_lMemStreamAddr(0)
        , m_lMemStreamSize(0)
        , m_lMemStreamReceived(0)
        , m_lMemStreamBytesPlayed(0)
        , m_pMemStreamBuffer(nullptr)
        , m_lMemStreamBufferSize(0)
        , m_lMuteAllFadeTime(0)
        , m_lCurrentFrameSegment(0)
        , m_lBufferIdCnt(0)
        , m_lStartedBuffersCnt(0)
        , m_lStoppedBuffersCnt(0)
        , m_lDelayedBuffersCnt(0)
        , m_lPosesLoadedCnt(0)
        , m_pPoseDestAddr(nullptr)
        , m_bRunning(false)
        , m_pCmdFrameBuffer(nullptr)
        , m_pCmd(nullptr)
        , m_lNumPlaying(0)
        , m_lMaxNumPlaying(16)
        , m_lNumBuffers(16)
        , m_pWaveHeaders(nullptr)
        , m_lWaveHeadersSize(0)
        , m_lNumUsedBuffers(0)
        , m_pListener(nullptr)
        , m_lCurNumPlaying(0)
        , m_lCurLowestPrio(0)
    {
        for (ZBufferGroup& group : m_BufferGroups)
        {
            std::memset(&group, 0, sizeof(group));
        }
        std::memset(m_Chains, 0, sizeof(m_Chains));
        std::memset(m_TransferRequests, 0, sizeof(m_TransferRequests));
        std::memset(m_FrameSegment, 0, sizeof(m_FrameSegment));
        std::memset(m_tBufferId, 0, sizeof(m_tBufferId));
        std::memset(m_StartedBuffers, 0, sizeof(m_StartedBuffers));
        std::memset(m_StoppedBuffers, 0, sizeof(m_StoppedBuffers));
        std::memset(m_DelayedBuffers, 0, sizeof(m_DelayedBuffers));
        std::memset(m_PosesBuffers, 0, sizeof(m_PosesBuffers));
    }

    void ZSynth::AllocateBuffers()
    {
        m_pBuffers = static_cast<_ZSoundBuffer**>(
            ZUniMemory::Allocate(sizeof(_ZSoundBuffer*) * m_lNumBuffers));

        for (int32_t i = 0; i < m_lNumBuffers; ++i)
        {
            m_pBuffers[i] = AllocateBuffer();
            m_pBuffers[i]->m_lBufferIndex = i;
        }
    }

    void ZSynth::FreeBuffers()
    {
    }

    _ZSoundBuffer* ZSynth::GetBuffer(uint32_t _priority, SWaveHeader* _wave, uint32_t, bool _required)
    {
        _ZSoundBuffer* freeBuffer = nullptr;
        _ZSoundBuffer* lowestPriority = nullptr;
        for (int i = 0; i < m_lNumBuffers; ++i)
        {
            _ZSoundBuffer* buffer = m_pBuffers[i];
            if (!buffer->m_bInUse)
            {
                if (!freeBuffer)
                    freeBuffer = buffer;
                continue;
            }
            if ((static_cast<int8_t>(_wave->m_iFlags) >= 0 || buffer->m_lCurPlaySeg == 2)
                && (!lowestPriority || buffer->m_lPrio < lowestPriority->m_lPrio))
            {
                lowestPriority = buffer;
            }
        }

        if (_required && freeBuffer)
            return freeBuffer;
        if (lowestPriority && lowestPriority->m_lPrio < _priority)
            return lowestPriority;
        return freeBuffer;
    }

    void ZSynth::PushScene()
    {
    }

    void ZSynth::PopScene()
    {
    }

    void ZSynth::InitFrame()
    {
        m_pCmd = m_pCmdFrameBuffer;
    }

    bool ZSynth::SendFrame()
    {
        return CmdFrameReady();
    }

    bool ZSynth::SoundEngineReady()
    {
        return true;
    }

    void ZSynth::SignalFrameReady()
    {
    }

    void ZSynth::RenderBuffers()
    {
        for (int i = 0; i < m_lNumBuffers; ++i)
        {
            if (m_pBuffers[i] && m_pBuffers[i]->m_bInUse)
                m_pBuffers[i]->Render();
        }
    }

    void ZSynth::BuildFrame()
    {
    }

    void ZSynth::AddFrameCommands()
    {
    }

    void ZSynth::FadeAllBuffers(float)
    {
    }

    void ZSynth::SetEffectsImage()
    {
    }

    bool ZSynth::CmdFrameReady()
    {
        return true;
    }

    void ZSynth::SignalCmdFrameProcessed()
    {
    }

    uint32_t ZSynth::GetWaveDuration(SWaveHeader* _wave)
    {
        return (_wave->m_lNumSamples / _wave->m_lSampleRate) / _wave->m_lNumChannels;
    }

    SWaveHeader* ZSynth::GetWaveHeader(int _offset)
    {
        if (!m_pWaveHeaders || !_offset)
            return nullptr;
        return reinterpret_cast<SWaveHeader*>(m_pWaveHeaders + _offset);
    }

    ZSynth::~ZSynth() = default;

    bool ZSynth::Initialize()
    {
        Reset();
        if (!Create())
        {
            return false;
        }

        AllocateBuffers();
        m_bRunning = true;
        return true;
    }

    void ZSynth::Reset()
    {
        m_lStartedBuffersCnt = 0;
        m_lStoppedBuffersCnt = 0;
        m_lDelayedBuffersCnt = 0;
        m_lPosesLoadedCnt = 0;
        m_lBufferIdCnt = 0;
        m_lTransferRequestCnt = 0;
        std::memset(m_PosesBuffers, 0, sizeof(m_PosesBuffers));
        for (int i = 0; i < m_lNumBuffers; ++i)
        {
            m_pBuffers[i]->Stop();
            m_pBuffers[i]->Free();
            m_pBuffers[i]->m_VoiceState = VS_READY;
        }
        if (m_pStreamer)
            m_pStreamer->Reset();
        for (ZBufferGroup& group : m_BufferGroups)
        {
            group.m_lNumGrouped = 0;
            group.m_pReferenceBuffer = nullptr;
            group.m_pFirstHeader = nullptr;
        }
    }

    void ZSynth::Free()
    {
        Reset();
        if (m_pStreamer)
        {
            m_pStreamer->Destroy();
            m_pStreamer = nullptr;
        }
        FreeBuffers();
        m_pBuffers = nullptr;
    }

    void ZSynth::InstallStreamWaves(int, const char* _data, const char* _localizedData)
    {
        if (m_pStreamer)
            m_pStreamer->SetAudioStreamFile(_data, _localizedData);
    }

    void ZSynth::CommitFrame()
    {
    }

    void ZSynth::Render()
    {
        InitFrame();
        if (!SoundEngineReady())
        {
            return;
        }
        RenderBuffers();
        BuildFrame();
        SendFrame();
    }

    bool ZSynth::CopyWaveData(_ZSoundBuffer*)
    {
        return true;
    }

    _ZSoundBuffer* ZSynth::Duplicate(_ZSoundBuffer*)
    {
        return nullptr;
    }

    void ZSynth::StartMemStream()
    {
        m_bMemStreamRunning = true;
        m_bMemStreamNeedData = true;
        m_lMemStreamAddr = 0;
        m_lMemStreamSize = m_lMemStreamBufferSize;
        m_lMemStreamReceived = 0;
        m_lMemStreamBytesPlayed = 0;
        if (m_pMemStreamBuffer && m_lMemStreamBufferSize)
            std::memset(m_pMemStreamBuffer, 0, m_lMemStreamBufferSize);
    }

    void ZSynth::StopMemStream()
    {
        m_bMemStreamRunning = false;
        m_bMemStreamReady = false;
        if (m_pMemStreamBuffer && m_lMemStreamBufferSize)
            std::memset(m_pMemStreamBuffer, 0, m_lMemStreamBufferSize);
    }

    void ZSynth::SetFmvVolume(int)
    {
    }

    void ZSynth::RemoveFromMetaTab(_ZSoundBuffer* _buffer)
    {
        for (_ZSoundBuffer*& entry : m_PosesBuffers)
        {
            if (entry == _buffer)
                entry = nullptr;
        }
    }
}
