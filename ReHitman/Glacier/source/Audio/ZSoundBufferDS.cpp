#include <Glacier/Audio/ZSoundBufferDS.h>
#include <Glacier/Audio/ZEaxSource.h>
#include <Glacier/Audio/ZSynthDS.h>
#include <Glacier/ZUniMemory.h>

#include <dsound.h>

#include <algorithm>
#include <cmath>

namespace Glacier
{
    _ZSoundBufferDS::_ZSoundBufferDS(ZSynth* _synth)
        : ZSoundBufferWintel(_synth)
        , m_pDSBuffer(nullptr)
        , m_pDS3DBuffer(nullptr)
        , m_pEaxSource(nullptr)
    {
    }

    _ZSoundBufferDS::~_ZSoundBufferDS()
    {
        Free();
    }

    bool _ZSoundBufferDS::NeedData(int* _needed)
    {
        const int halfSize = static_cast<int>(m_lBufferSize / 2);
        if (!halfSize)
            return false;

        const int segment = GetPlayCursor() / halfSize;
        if (segment == m_lCurPlaySeg)
            return false;

        *_needed = halfSize * m_lCurPlaySeg;
        m_lCurPlaySeg = segment;
        return true;
    }

    bool _ZSoundBufferDS::Create(const SWaveHeader* _wave, uint32_t _bufferType, uint32_t _flags)
    {
        const uint32_t bufferType = _wave->m_lNumChannels == 2 ? 2 : _bufferType;
        if (!ZSoundBufferWintel::Create(_wave, bufferType, _flags))
            return false;

        WAVEFORMATEX format{};
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = static_cast<WORD>(_wave->m_lNumChannels);
        format.nSamplesPerSec = _wave->m_lSampleRate;
        format.wBitsPerSample = 16;
        format.nBlockAlign = static_cast<WORD>(2 * format.nChannels);
        format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

        auto* synth = static_cast<ZSynthDS*>(m_pSoundCon);
        DSBUFFERDESC description{};
        description.dwSize = sizeof(description);
        description.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY |
            DSBCAPS_GETCURRENTPOSITION2;
        if (synth->m_bEnableHardware)
            description.dwFlags |= DSBCAPS_LOCHARDWARE;
        if (m_dwBufferType || format.nChannels != 1)
            description.dwFlags |= DSBCAPS_CTRLPAN;
        else
            description.dwFlags |= DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
        description.dwBufferBytes = m_lBufferSize;
        description.lpwfxFormat = &format;

        if (FAILED(synth->GetDS()->CreateSoundBuffer(&description, &m_pDSBuffer, nullptr)))
            return false;

        if (!m_dwBufferType && FAILED(m_pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer,
                reinterpret_cast<void**>(&m_pDS3DBuffer))))
        {
            m_pDSBuffer->Release();
            m_pDSBuffer = nullptr;
            return false;
        }

        if (synth->EaxEnabled() && m_pDS3DBuffer && synth->m_bEnableHardware)
        {
            m_pEaxSource = ZUniMemory::New<ZEaxSource>();
            if (m_pEaxSource->Init(m_pDSBuffer, true))
                m_pEaxSource->Update();
            else
            {
                ZUniMemory::Delete(m_pEaxSource);
                m_pEaxSource = nullptr;
            }
        }
        return true;
    }

    void _ZSoundBufferDS::Start()
    {
        if (!m_pDSBuffer)
            return;
        m_bPlaying = true;
        m_pDSBuffer->SetCurrentPosition(m_lBufferId * (m_lBufferSize / 2));
        m_pDSBuffer->Play(0, 0, m_bLooping ? DSBPLAY_LOOPING : 0);
    }

    void _ZSoundBufferDS::Start(bool _loop)
    {
        if (!m_pDSBuffer)
            return;
        m_bPlaying = true;
        m_pDSBuffer->SetCurrentPosition(0);
        m_pDSBuffer->Play(0, 0, _loop ? DSBPLAY_LOOPING : 0);
    }

    void _ZSoundBufferDS::Stop()
    {
        if (m_pDSBuffer)
            m_pDSBuffer->Stop();
        ZSoundBufferWintel::Stop();
    }

    _ZSoundBuffer* _ZSoundBufferDS::Duplicate()
    {
        return static_cast<ZSynthDS*>(m_pSoundCon)->Duplicate(this);
    }

    int _ZSoundBufferDS::GetPlayCursor()
    {
        DWORD cursor = 0;
        if (m_pDSBuffer)
            m_pDSBuffer->GetCurrentPosition(&cursor, nullptr);
        return static_cast<int>(cursor);
    }

    void _ZSoundBufferDS::Free()
    {
        if (m_pDS3DBuffer)
            m_pDS3DBuffer->Release();
        ZUniMemory::Delete(m_pEaxSource);
        if (m_pDSBuffer)
            m_pDSBuffer->Release();
        m_pDS3DBuffer = nullptr;
        m_pDSBuffer = nullptr;
        m_pEaxSource = nullptr;
        _ZSoundBuffer::Free();
    }

    void _ZSoundBufferDS::Resume()
    {
        if (m_pDSBuffer)
            m_pDSBuffer->Play(0, 0, m_bLooping ? DSBPLAY_LOOPING : 0);
    }

    void _ZSoundBufferDS::Pause()
    {
        if (m_pDSBuffer)
            m_pDSBuffer->Stop();
    }

    void _ZSoundBufferDS::ResetVolume()
    {
        if (!m_pDSBuffer)
            return;
        LONG volume = 0;
        m_pDSBuffer->GetVolume(&volume);
        m_pDSBuffer->SetVolume(volume == DSBVOLUME_MIN ? -9000 : DSBVOLUME_MIN);
    }

    void _ZSoundBufferDS::Update(SStartSound2D* _command)
    {
        m_pDSBuffer->SetPan(std::clamp(_command->m_lPan, -10000, 10000));
    }

    void _ZSoundBufferDS::Update(SStartSound3D* _command)
    {
        if (_command->m_lBufferType == 10)
        {
            m_pDSBuffer->SetPan(std::clamp(Calc3DPan(_command), -10000, 10000));
            return;
        }
        if (_command->m_lBufferType != 0 || !m_pDS3DBuffer)
            return;

        DS3DBUFFER parameters{};
        parameters.dwSize = sizeof(parameters);
        parameters.vPosition = {_command->m_vPosition.x, _command->m_vPosition.y, _command->m_vPosition.z};
        parameters.vVelocity = {_command->m_vVelocity.x, _command->m_vVelocity.y, _command->m_vVelocity.z};
        parameters.dwInsideConeAngle = static_cast<DWORD>(_command->m_fInnerConeAngle);
        parameters.dwOutsideConeAngle = static_cast<DWORD>(_command->m_fOuterConeAngle);
        parameters.vConeOrientation = {_command->m_vConeOrientation.x, _command->m_vConeOrientation.y,
            _command->m_vConeOrientation.z};
        parameters.lConeOutsideVolume = static_cast<LONG>(_command->m_fOuterConeVolume);
        parameters.flMinDistance = _command->m_fMinDistance;
        parameters.flMaxDistance = _command->m_fMaxDistance;
        parameters.dwMode = DS3DMODE_NORMAL;
        m_pDS3DBuffer->SetAllParameters(&parameters, DS3D_DEFERRED);
    }

    void _ZSoundBufferDS::Update(SStartSoundBFormat* _command)
    {
        m_pDSBuffer->SetPan(std::clamp(CalcBFormatPan(_command), -10000, 10000));
    }

    void _ZSoundBufferDS::Update(SStartSound* _command)
    {
        if (m_fVolumePercent > 0.001f)
            m_fVolumePercent = (std::min)(m_fVolumePercent, 100.0f);
        else
            m_fVolumePercent = 0.0f;
        const LONG volume = m_fVolumePercent == 0.0f ? DSBVOLUME_MIN :
            static_cast<LONG>(std::log10(m_fVolumePercent * 0.01f) * 2000.0f);
        m_pDSBuffer->SetVolume(volume);

        const double pitch = static_cast<double>(_command->m_lPitch);
        DWORD frequency = pitch >= 0.0 ?
            static_cast<DWORD>((pitch * 0.00083333335 + 1.0) * m_rWave->m_lSampleRate) :
            static_cast<DWORD>(m_rWave->m_lSampleRate / (pitch * -0.00083333335 + 1.0));
        m_pDSBuffer->SetFrequency(std::clamp<DWORD>(frequency, 100, 100000));
    }

    void _ZSoundBufferDS::UpdateFilters()
    {
        if (m_lChainIdx == -1)
            return;
        SChain& chain = m_pSoundCon->m_Chains[m_lChainIdx];

        if (m_pEaxSource && !m_dwBufferType)
        {
            auto& properties = m_pEaxSource->m_Properties;
            properties.m_lObstruction = 0;
            properties.m_fObstructionLFRatio = 0.0f;
            properties.m_lOcclusion = 0;
            properties.m_fOcclusionLFRatio = 0.0f;
            properties.m_fOcclusionRoomRatio = 0.0f;
            properties.m_fOcclusionDirectRatio = 0.0f;
            properties.m_lExclusion = 0;
            properties.m_fExclusionLFRatio = 1.0f;

            for (int i = 0; i < chain.m_lNumFilters; ++i)
            {
                const auto& filter = chain.m_Filters[i];
                if (filter.m_lType == 1)
                {
                    const float factor = static_cast<const SCmdOcclusionBase*>(filter.m_pFilter)->m_fOpenness;
                    properties.m_lOcclusion -= static_cast<int32_t>((1.0f - factor) * 4500.0f);
                    properties.m_fOcclusionLFRatio = 0.25f;
                    properties.m_fOcclusionRoomRatio = 2.0f;
                    properties.m_fOcclusionDirectRatio = 1.0f;
                }
            }
            m_pEaxSource->Update();
            return;
        }

        for (int i = 0; i < chain.m_lNumFilters; ++i)
        {
            const auto& filter = chain.m_Filters[i];
            if (filter.m_lType == 1)
            {
                const float factor = static_cast<const SCmdOcclusionBase*>(filter.m_pFilter)->m_fOpenness;
                m_fVolumePercent -= (1.0f - factor) * 50.0f;
            }
        }
        m_fVolumePercent = (std::max)(m_fVolumePercent, 0.0f);
    }

    void _ZSoundBufferDS::UnlockBuffer(void* _buffer1, int _size1, void* _buffer2, int _size2)
    {
        if (m_pDSBuffer)
            m_pDSBuffer->Unlock(_buffer1, _size1, _buffer2, _size2);
    }

    bool _ZSoundBufferDS::LockBuffer(int _offset, int _size, void** _buffer1, int* _size1,
        void** _buffer2, int* _size2)
    {
        if (!m_pDSBuffer)
            return false;
        return SUCCEEDED(m_pDSBuffer->Lock(_offset, _size, _buffer1,
            reinterpret_cast<DWORD*>(_size1), _buffer2, reinterpret_cast<DWORD*>(_size2), 0));
    }
}
