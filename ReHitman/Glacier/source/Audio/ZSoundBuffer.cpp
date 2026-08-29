#include <Glacier/Audio/ZSoundBuffer.h>
#include <Glacier/Audio/ZSynth.h>

#include <cstring>

namespace Glacier
{
    _ZSoundBuffer::_ZSoundBuffer(ZSynth* _synth)
        : m_pSoundCon(_synth)
        , m_lBufferSize(0)
        , m_lCurPlaySeg(0)
        , m_lBufferId(0)
        , m_bWaitingForMetaSync(false)
        , m_lBufferIndex(0)
        , m_bLowpassEnabled(false)
        , m_bAddIdCmd(false)
        , m_VoiceState(VS_READY)
        , m_lPlayCursor(0)
        , m_lPrevPlayCursor(0)
        , m_field34(0)
        , m_pPoseData(nullptr)
        , m_lCrossFadePct(0)
        , m_bFinished(false)
        , m_pStream(nullptr)
        , m_lChainIdx(0)
        , m_field4C(0)
        , m_lPrio(0)
        , m_dwBufferType(0)
        , m_lDopplerPitch(0)
        , m_field5C(0)
        , m_field60(0)
        , m_field64(0)
        , m_field68(0)
        , m_field6C(0)
        , m_field70(0)
        , m_field74(0)
        , m_rWave(nullptr)
        , m_rSndObj(0)
        , m_dwWriteOffset(0)
        , m_dwPlayPos(0)
        , m_dwSize(0)
        , m_pData(nullptr)
        , m_eBufferType(SBT_NORMAL)
        , m_bPause(false)
        , m_bPlaying(false)
        , m_bLooping(false)
        , m_bReady(false)
        , m_bInUse(false)
        , m_bSegmentDataNeeded(false)
    {
        std::memset(&m_Inf, 0, sizeof(m_Inf));
    }

    bool _ZSoundBuffer::NeedData(int*)
    {
        return true;
    }

    _ZSoundBuffer::~_ZSoundBuffer() = default;

    bool _ZSoundBuffer::Create(const SWaveHeader* _wave, uint32_t _bufferType, uint32_t)
    {
        m_bWaitingForMetaSync = false;
        m_bLowpassEnabled = false;
        m_bAddIdCmd = false;
        m_lBufferId = 0;
        Free();
        m_field4C = 0;
        m_bPause = false;
        m_bFinished = false;
        m_rWave = _wave;
        m_dwBufferType = _bufferType;

        if (static_cast<int8_t>(_wave->m_iFlags) >= 0)
        {
            m_eBufferType = SBT_NORMAL;
            m_lBufferSize = _wave->m_lDataSize;
        }
        else
        {
            m_eBufferType = SBT_DISCSTREAM;
            m_bWaitingForMetaSync = _wave->m_iPosChunkSize != 0;

            uint32_t minimumRate = 0;
            uint32_t maximumRate = 0;
            if (GetLayerSampleRates(minimumRate, maximumRate))
            {
                const uint32_t bytesPerSample = (2 * (minimumRate / 10) + 1) & ~1u;
                m_lBufferSize = 2 * _wave->m_lNumChannels * (_wave->m_lSampleRate / minimumRate) * bytesPerSample;
            }
            else
            {
                m_lBufferSize = (4 * _wave->m_lNumChannels * (_wave->m_lSampleRate / 10) + 15) & ~15u;
            }
        }

        m_bReady = false;
        m_pStream = nullptr;
        m_bSegmentDataNeeded = true;
        m_bInUse = true;
        std::memset(&m_Inf, 0, sizeof(m_Inf));
        m_lPlayCursor = m_lBufferSize;
        return true;
    }

    void _ZSoundBuffer::Destroy()
    {
    }

    void _ZSoundBuffer::Start()
    {
    }

    void _ZSoundBuffer::Start(SSynthCmdBase*)
    {
    }

    void _ZSoundBuffer::Start(bool)
    {
    }

    void _ZSoundBuffer::Update(SStartSoundBase* _command)
    {
        if ((_command->m_lFlags & 2) != 0)
            m_bWaitingForMetaSync = false;

        m_lChainIdx = static_cast<int32_t>(_command->m_lPrio);
        m_lPrio = _command->m_lSndRef;
        m_bLooping = _command->m_lHeaderOffset != 0;
        m_rSndObj = static_cast<uint32_t>(_command->m_lPitch);
        m_bLowpassEnabled = _command->m_lLowpassEnabled != 0;

        if (m_VoiceState != VS_STARTFADE && m_VoiceState != VS_FADING)
        {
            m_field5C = _command->m_field38;
            m_field60 = _command->m_field3C;
            m_field64 = _command->m_field40;
            m_field68 = _command->m_field44;
            m_field6C = static_cast<int32_t>(_command->m_lBufferType);
            m_field70 = _command->m_field48;
            m_field74 = _command->m_field4C;
        }

        if (m_bPause)
        {
            m_bPause = false;
            Resume();
        }
    }

    void _ZSoundBuffer::Update()
    {
        UpdateVolume(false);
    }

    void _ZSoundBuffer::Stop()
    {
        m_pSoundCon->RemoveFromMetaTab(this);
        m_bPlaying = false;
        m_bInUse = false;
        m_rSndObj = 0;

        if (m_eBufferType == SBT_DISCSTREAM && m_pStream)
        {
            --m_pStream->m_lReferenceCount;
            if (!m_pStream->m_lReferenceCount)
                m_pSoundCon->m_pStreamer->RemoveStream(m_pStream);
            m_pStream = nullptr;
        }
    }

    _ZSoundBuffer* _ZSoundBuffer::Duplicate()
    {
        return nullptr;
    }

    bool _ZSoundBuffer::UpdateVolume(bool)
    {
        return m_rWave != nullptr;
    }

    int _ZSoundBuffer::CopyWaveData()
    {
        return 0;
    }

    int _ZSoundBuffer::GetPlayCursor()
    {
        return 0;
    }

    void _ZSoundBuffer::Unlock()
    {
    }

    void _ZSoundBuffer::Render()
    {
        const int previousCursor = m_lPlayCursor;
        m_lPlayCursor = GetPlayCursor();
        m_lPrevPlayCursor = previousCursor;

        if (m_eBufferType == SBT_DISCSTREAM && m_pStream)
        {
            if (m_bReady)
            {
                CopyWaveData();
            }
            else if (m_pStream->m_bPreloadReady)
            {
                m_lPrevPlayCursor = 0;
                m_lPlayCursor = static_cast<int32_t>(m_lBufferSize);
                if (CopyWaveData())
                    m_bReady = true;
            }
        }
        else if (m_eBufferType == SBT_NORMAL && m_bSegmentDataNeeded)
        {
            CopyWaveData();
            m_bSegmentDataNeeded = false;
            m_bReady = true;
        }
    }

    void _ZSoundBuffer::Free()
    {
        m_pSoundCon->RemoveFromMetaTab(this);
        if (m_eBufferType == SBT_DISCSTREAM && m_pStream)
        {
            --m_pStream->m_lReferenceCount;
            if (!m_pStream->m_lReferenceCount)
                m_pSoundCon->m_pStreamer->RemoveStream(m_pStream);
        }

        m_bPlaying = false;
        m_bReady = false;
        m_bInUse = false;
        m_pData = nullptr;
        m_rSndObj = 0;
        m_rWave = nullptr;
        m_dwSize = 0;
        m_dwPlayPos = 0;
        m_eBufferType = SBT_NORMAL;
        m_pStream = nullptr;
        m_lBufferSize = 0;
        m_lBufferId = 0;
        m_VoiceState = VS_READY;
        m_lPlayCursor = 0;
        m_lPrevPlayCursor = 0;
        m_lPrio = 0;
        std::memset(&m_Inf, 0, sizeof(m_Inf));
        m_pPoseData = nullptr;
    }

    bool _ZSoundBuffer::Allocate()
    {
        return true;
    }

    void _ZSoundBuffer::PauseCheck(int _pause)
    {
        if (_pause)
        {
            if (!m_bPause)
            {
                m_bPause = true;
                Pause();
            }
        }
        else if (m_bPause)
        {
            m_bPause = false;
            Resume();
        }
    }

    void _ZSoundBuffer::Resume()
    {
    }

    void _ZSoundBuffer::Pause()
    {
    }

    void _ZSoundBuffer::ResetVolume()
    {
    }

    bool _ZSoundBuffer::GetLayerSampleRates(uint32_t& _minimum, uint32_t& _maximum) const
    {
        if (!m_rWave->m_iLayerInfo)
            return false;

        const SWaveHeader* firstLayer = m_rWave;
        if (static_cast<int8_t>(m_rWave->m_iLayerInfo) >= 0)
            firstLayer -= m_rWave->m_iLayerInfo;

        _minimum = 0x40000000;
        _maximum = 0;
        const uint32_t layerCount = firstLayer->m_iLayerInfo & 0x7F;
        for (uint32_t i = 0; i < layerCount; ++i)
        {
            const uint32_t sampleRate = firstLayer[i].m_lSampleRate;
            if (sampleRate < _minimum)
                _minimum = sampleRate;
            if (sampleRate > _maximum)
                _maximum = sampleRate;
        }
        return true;
    }
}
