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
        , m_bFrameClaimed(false)
        , m_lGroupType(0)
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
        , m_lVolume(0)
        , m_lVolumeL(0)
        , m_lVolumeR(0)
        , m_lVolumeS(0)
        , m_lPitch(0)
        , m_lVolumeLS(0)
        , m_lVolumeRS(0)
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
        m_lGroupType = 0;
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

        m_lChainIdx = _command->m_lPathIdx;
        m_lPrio = _command->m_lPrio;
        m_bLooping = _command->m_bLooping != 0;
        m_rSndObj = _command->m_lSndRef;
        m_bLowpassEnabled = _command->m_lLowpassEnabled != 0;

        if (m_VoiceState != VS_STARTFADE && m_VoiceState != VS_FADING)
        {
            m_lVolume = _command->m_lVolume;
            m_lVolumeL = _command->m_lVolumeL;
            m_lVolumeR = _command->m_lVolumeR;
            m_lVolumeS = _command->m_lVolumeS;
            m_lPitch = _command->m_lPitch;
            m_lVolumeLS = _command->m_lVolumeLS;
            m_lVolumeRS = _command->m_lVolumeRS;
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
        return 3;
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

    bool _ZSoundBuffer::AllocateWaveResource(ZIOStream* _stream, SStartSoundBase* _command)
    {
        if (m_eBufferType != SBT_DISCSTREAM)
            return true;
        if (_stream)
        {
            m_pStream = _stream;
            ++m_pStream->m_lReferenceCount;
            if (m_bLooping)
                m_pStream->m_bLoopStream = true;
            return true;
        }

        const SWaveHeader* firstLayer = m_rWave;
        if (m_rWave->m_iLayerInfo && static_cast<int8_t>(m_rWave->m_iLayerInfo) >= 0)
            firstLayer -= m_rWave->m_iLayerInfo;
        const uint32_t layerCount = m_rWave->m_iLayerInfo ? firstLayer->m_iLayerInfo & 0x7F : 1;

        uint32_t streamRate = m_rWave->m_lNumChannels * m_rWave->m_lSampleRate *
            m_rWave->m_lBitsPerSample / 8;
        if (m_rWave->m_iDataType == 4096)
        {
            const uint32_t ratio = m_rWave->m_lDataSize / m_rWave->m_lPackedSize;
            streamRate = 2 * ((2 * m_rWave->m_lNumChannels * m_rWave->m_lSampleRate) / ratio);
        }

        uint32_t streamSize = 0;
        for (uint32_t i = 0; i < layerCount; ++i)
            streamSize += firstLayer[i].m_lPackedSize;
        const uint32_t metaChunkSize = static_cast<uint32_t>(m_rWave->m_iPosChunkSize) << 10;
        const uint32_t waveChunkSize = static_cast<uint32_t>(m_rWave->m_iSoundChunkSize) << 10;
        uint32_t numMetaChunks = 0;
        if (waveChunkSize)
        {
            numMetaChunks = (streamSize + waveChunkSize - 1) / waveChunkSize;
            streamSize += numMetaChunks * metaChunkSize;
        }
        else if (metaChunkSize)
        {
            numMetaChunks = 1;
            streamSize += metaChunkSize;
        }

        if (!streamRate)
            streamRate = 0x8000;
        m_pStream = m_pSoundCon->m_pStreamer->AddAudioStream(streamRate, m_rWave->m_lDataOffset,
            streamSize, m_bLooping, 0, metaChunkSize, waveChunkSize, numMetaChunks, false);
        if (!m_pStream)
        {
            m_bInUse = false;
            m_bReady = false;
            return false;
        }

        ++m_pStream->m_lReferenceCount;
        auto* group = m_pStream->m_pUserData;
        group->m_pFirstHeader = m_rWave->m_iLayerInfo ? firstLayer : nullptr;
        if (m_rWave->m_iLayerInfo)
        {
            uint32_t minimumRate = 0x40000000;
            for (uint32_t i = 0; i < layerCount; ++i)
                minimumRate = (std::min)(minimumRate, firstLayer[i].m_lSampleRate);
            group->m_lSamples = (2 * (minimumRate / 10) + 1) & ~1u;
            group->m_lSampleSize = 0;
            for (uint32_t i = 0; i < layerCount; ++i)
            {
                auto& layer = group->m_Layers[i];
                layer.m_lByteOffset = group->m_lSampleSize;
                layer.m_lBytesPerSample = 2 * (firstLayer[i].m_lSampleRate / minimumRate) *
                    firstLayer[i].m_lNumChannels;
                group->m_lSampleSize += layer.m_lBytesPerSample;
            }
        }
        std::memset(&group->m_DecodeInfo, 0, sizeof(group->m_DecodeInfo));

        if (m_rWave->m_iDataType == 1 && _command->m_lStartOffset)
        {
            uint32_t byteOffset = streamRate * (_command->m_lStartOffset >> 8) +
                ((streamRate * static_cast<uint8_t>(_command->m_lStartOffset)) >> 8);
            if (byteOffset >= m_rWave->m_lDataSize)
                byteOffset %= m_rWave->m_lDataSize;
            const uint32_t frameSize = m_rWave->m_iLayerInfo ? group->m_lSampleSize :
                (m_rWave->m_lBitsPerSample >> 3) * m_rWave->m_lNumChannels;
            byteOffset -= byteOffset % frameSize;
            if (byteOffset)
            {
                m_pStream->SetStartOffset(byteOffset, waveChunkSize, metaChunkSize, numMetaChunks);
                m_bWaitingForMetaSync = false;
            }
        }
        return true;
    }
}
