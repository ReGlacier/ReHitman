#include <Glacier/Audio/ZEaxListener.h>

#include <algorithm>

namespace Glacier
{
    const GUID& GetEAXListenerPropertiesGuid();

    ZEaxListener::ZEaxListener()
        : m_pSoundBuffer(nullptr)
    {
    }

    ZEaxListener::~ZEaxListener()
    {
        if (m_pSoundBuffer)
            m_pSoundBuffer->Release();
    }

    void ZEaxListener::Update()
    {
        if (m_pPropertySet)
            m_pPropertySet->Set(GetEAXListenerPropertiesGuid(), 1, nullptr, 0,
                &m_Properties, sizeof(m_Properties));
    }

    void ZEaxListener::GetDefaultValues()
    {
        m_Properties = {};
        m_Properties.m_fEnvironmentSize = 7.5f;
        m_Properties.m_fEnvironmentDiffusion = 1.0f;
        m_Properties.m_lRoom = -1000;
        m_Properties.m_lRoomHF = -100;
        m_Properties.m_fDecayTime = 1.49f;
        m_Properties.m_fDecayHFRatio = 0.83f;
        m_Properties.m_fDecayLFRatio = 1.0f;
        m_Properties.m_lReflections = -2602;
        m_Properties.m_fReflectionsDelay = 0.007f;
        m_Properties.m_lReverb = 200;
        m_Properties.m_fReverbDelay = 0.011f;
        m_Properties.m_fEchoTime = 0.25f;
        m_Properties.m_fModulationTime = 0.25f;
        m_Properties.m_fAirAbsorptionHF = -5.0f;
        m_Properties.m_fHFReference = 5000.0f;
        m_Properties.m_fLFReference = 250.0f;
        m_Properties.m_lFlags = 0x3F;
        if (m_pPropertySet)
            m_pPropertySet->Set(GetEAXListenerPropertiesGuid(), 0x80000001, nullptr, 0,
                &m_Properties, sizeof(m_Properties));
    }

    bool ZEaxListener::Init(IDirectSound* _directSound, IDirectSoundBuffer* _buffer, bool _setDefaults)
    {
        if (_buffer)
            return ZEaxBase::Init(_buffer, _setDefaults);

        WAVEFORMATEX format{};
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 1;
        format.nSamplesPerSec = 44100;
        format.nAvgBytesPerSec = 88200;
        format.nBlockAlign = 2;
        format.wBitsPerSample = 16;

        DSBUFFERDESC description{};
        description.dwSize = sizeof(description);
        description.dwFlags = 0x14;
        description.dwBufferBytes = 64;
        description.lpwfxFormat = &format;
        if (FAILED(_directSound->CreateSoundBuffer(&description, &m_pSoundBuffer, nullptr)))
            return false;
        return ZEaxBase::Init(m_pSoundBuffer, _setDefaults);
    }

    bool ZEaxListener::SetRoom(int32_t _room)
    {
        return m_pPropertySet && SUCCEEDED(m_pPropertySet->Set(GetEAXListenerPropertiesGuid(),
            2, nullptr, 0, &_room, sizeof(_room)));
    }

    bool ZEaxListener::SetEnvSize(float _size)
    {
        return m_pPropertySet && SUCCEEDED(m_pPropertySet->Set(GetEAXListenerPropertiesGuid(),
            0x80000003, nullptr, 0, &_size, sizeof(_size)));
    }

    void ZEaxListener::SetEnv(ZREF _environment)
    {
        m_rEnvironment = _environment;
        GetDefaultValues();
    }

    int32_t ZEaxListener::SetEnvValue(int32_t _current, int32_t _target, int32_t _step)
    {
        if (_current < _target && _target - _current > _step)
            return _target - _step;
        if (_current > _target && _current - _target > _step)
            return _target + _step;
        return _current;
    }

    float ZEaxListener::SetEnvValue(float _current, float _target, float _step)
    {
        if (_current < _target && _target - _current > _step)
            return _target - _step;
        if (_current > _target && _current - _target > _step)
            return _target + _step;
        return _current;
    }
}
