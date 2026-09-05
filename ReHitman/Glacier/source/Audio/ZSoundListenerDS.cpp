#include <Glacier/Audio/ZSoundListenerDS.h>

#include <Glacier/Audio/ZSoundBufferDS.h>
#include <Glacier/Audio/ZEaxListener.h>
#include <Glacier/Audio/ZSynthDS.h>
#include <Glacier/Audio/ZSynthWintel.h>
#include <Glacier/Audio/ZWintelRoomReverb.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>

#include <dsound.h>

#include <algorithm>
#include <cmath>

namespace Glacier
{
    namespace
    {
        struct SInterpolatedRoomReverb : SSynthFilterBase
        {
            ZWintelRoomReverb m_From;
            ZWintelRoomReverb m_To;
            float m_fFactor;
        };

        static_assert(sizeof(SInterpolatedRoomReverb) == 0xF0);

        float InterpolateLinear(float _from, float _to, float _factor)
        {
            return _from * (1.0f - _factor) + _to * _factor;
        }

        int32_t InterpolateLinear(int32_t _from, int32_t _to, float _factor)
        {
            return static_cast<int32_t>(static_cast<double>(_from) * (1.0 - _factor) +
                static_cast<double>(_to) * _factor);
        }

        float InterpolateExponential(float _from, float _to, float _factor, float _offset = 0.0f)
        {
            return std::pow(_from + _offset, 1.0f - _factor) * std::pow(_to + _offset, _factor);
        }

        void Normalize(EaxVector& _vector)
        {
            const float length = std::sqrt(_vector.x * _vector.x + _vector.y * _vector.y +
                _vector.z * _vector.z);
            if (length > 1.0f)
            {
                _vector.x /= length;
                _vector.y /= length;
                _vector.z /= length;
            }
        }

        void InterpolateEaxProperties(const EaxListenerProperties& _from,
            const EaxListenerProperties& _to, float _factor, EaxListenerProperties& _result)
        {
            if (_factor >= 1.0f)
            {
                _result = _from;
                return;
            }
            if (_factor <= 0.0f)
            {
                _result = _to;
                return;
            }

            _result.m_lEnvironment = 26;
            _result.m_fEnvironmentSize = InterpolateExponential(
                _from.m_fEnvironmentSize, _to.m_fEnvironmentSize, _factor);
            _result.m_fEnvironmentDiffusion = InterpolateLinear(
                _from.m_fEnvironmentDiffusion, _to.m_fEnvironmentDiffusion, _factor);
            _result.m_lRoom = InterpolateLinear(_from.m_lRoom, _to.m_lRoom, _factor);
            _result.m_lRoomHF = InterpolateLinear(_from.m_lRoomHF, _to.m_lRoomHF, _factor);
            _result.m_lRoomLF = InterpolateLinear(_from.m_lRoomLF, _to.m_lRoomLF, _factor);
            _result.m_fDecayTime = InterpolateExponential(_from.m_fDecayTime, _to.m_fDecayTime, _factor);
            _result.m_fDecayHFRatio = InterpolateExponential(
                _from.m_fDecayHFRatio, _to.m_fDecayHFRatio, _factor);
            _result.m_fDecayLFRatio = InterpolateExponential(
                _from.m_fDecayLFRatio, _to.m_fDecayLFRatio, _factor);
            _result.m_lReflections = InterpolateLinear(
                _from.m_lReflections, _to.m_lReflections, _factor);
            _result.m_fReflectionsDelay = InterpolateExponential(
                _from.m_fReflectionsDelay, _to.m_fReflectionsDelay, _factor, 0.0001f);

            EaxVector reflectionsFrom = _from.m_vReflectionsPan;
            EaxVector reflectionsTo = _to.m_vReflectionsPan;
            Normalize(reflectionsFrom);
            Normalize(reflectionsTo);
            _result.m_vReflectionsPan = {
                InterpolateLinear(reflectionsFrom.x, reflectionsTo.x, _factor),
                InterpolateLinear(reflectionsFrom.y, reflectionsTo.y, _factor),
                InterpolateLinear(reflectionsFrom.z, reflectionsTo.z, _factor)};

            _result.m_lReverb = InterpolateLinear(_from.m_lReverb, _to.m_lReverb, _factor);
            _result.m_fReverbDelay = InterpolateExponential(
                _from.m_fReverbDelay, _to.m_fReverbDelay, _factor, 0.0001f);

            EaxVector reverbFrom = _from.m_vReverbPan;
            EaxVector reverbTo = _to.m_vReverbPan;
            Normalize(reverbFrom);
            Normalize(reverbTo);
            _result.m_vReverbPan = {
                InterpolateLinear(reverbFrom.x, reverbTo.x, _factor),
                InterpolateLinear(reverbFrom.y, reverbTo.y, _factor),
                InterpolateLinear(reverbFrom.z, reverbTo.z, _factor)};

            _result.m_fEchoTime = InterpolateExponential(_from.m_fEchoTime, _to.m_fEchoTime, _factor);
            _result.m_fEchoDepth = InterpolateLinear(_from.m_fEchoDepth, _to.m_fEchoDepth, _factor);
            _result.m_fModulationTime = InterpolateExponential(
                _from.m_fModulationTime, _to.m_fModulationTime, _factor);
            _result.m_fModulationDepth = InterpolateLinear(
                _from.m_fModulationDepth, _to.m_fModulationDepth, _factor);
            _result.m_fAirAbsorptionHF = InterpolateLinear(
                _from.m_fAirAbsorptionHF, _to.m_fAirAbsorptionHF, _factor);
            _result.m_fHFReference = InterpolateExponential(
                _from.m_fHFReference, _to.m_fHFReference, _factor);
            _result.m_fLFReference = InterpolateExponential(
                _from.m_fLFReference, _to.m_fLFReference, _factor);
            _result.m_fRoomRolloffFactor = InterpolateLinear(
                _from.m_fRoomRolloffFactor, _to.m_fRoomRolloffFactor, _factor);
            _result.m_lFlags = _from.m_lFlags & _to.m_lFlags;
            _result.m_fReflectionsDelay = (std::min)(_result.m_fReflectionsDelay, 0.3f);
            _result.m_fReverbDelay = (std::min)(_result.m_fReverbDelay, 0.1f);
        }
    }

    ZSoundListenerDS::ZSoundListenerDS()
        : m_pDSListener(nullptr)
        , m_pPrimary(nullptr)
        , m_pEaxBuffer(nullptr)
        , m_pEaxListener(nullptr)
    {
    }

    ZSoundListenerDS::~ZSoundListenerDS()
    {
        Free();
    }

    bool ZSoundListenerDS::Init()
    {
        if (!m_pPrimary)
            return false;

        if (FAILED(m_pPrimary->QueryInterface(IID_IDirectSound3DListener,
                reinterpret_cast<void**>(&m_pDSListener))))
        {
            return false;
        }

        auto* synth = static_cast<ZSynthDS*>(m_pSynth);
        if (synth->EaxEnabled())
        {
            m_pEaxListener = ZUniMemory::New<ZEaxListener>();
            m_pEaxBuffer = static_cast<_ZSoundBufferDS*>(synth->AllocateBuffer());

            SWaveHeader wave{};
            wave.m_iDataType = 1;
            wave.m_lSampleRate = 22050;
            wave.m_lBitsPerSample = 16;
            wave.m_lDataSize = 32;
            wave.m_lNumChannels = 1;
            wave.m_lNumSamples = 16;
            wave.m_lBlockAlign = 2;
            wave.m_lSamplesPerBlock = 2;

            if (!m_pEaxBuffer->Create(&wave, 0, 0) ||
                !m_pEaxListener->Init(synth->GetDS(), m_pEaxBuffer->m_pDSBuffer, true))
            {
                ZUniMemory::Delete(m_pEaxListener);
                m_pEaxListener = nullptr;
                ZUniMemory::Delete(m_pEaxBuffer);
                m_pEaxBuffer = nullptr;
            }
        }
        SetEnvironment(0);
        return true;
    }

    void ZSoundListenerDS::Free()
    {
        if (m_pEaxBuffer)
        {
            ZUniMemory::Delete(m_pEaxBuffer);
            m_pEaxBuffer = nullptr;
        }

        ZUniMemory::Delete(m_pEaxListener);
        m_pEaxListener = nullptr;

        if (m_pDSListener)
        {
            m_pDSListener->Release();
            m_pDSListener = nullptr;
        }
    }

    bool ZSoundListenerDS::SetEnvSize(float _size)
    {
        return m_pEaxListener && m_pEaxListener->SetEnvSize(_size);
    }

    void ZSoundListenerDS::SetEnvironment(int _environment)
    {
        if (m_pEaxListener)
            m_pEaxListener->SetRoom(_environment);
    }

    void ZSoundListenerDS::SetEnv(ZREF _environment)
    {
        ZSoundListener::SetEnv(_environment);
        if (m_pEaxListener)
            m_pEaxListener->SetEnv(_environment);
    }

    void ZSoundListenerDS::Update(SSynthCmdBase* _command)
    {
        if (m_pEaxListener && m_pSynth->m_lNumChains > 0)
        {
            SChain& chain = m_pSynth->m_Chains[0];
            for (int i = 0; i < chain.m_lNumFilters; ++i)
            {
                const auto& filter = chain.m_Filters[i];
                if (filter.m_lType == 2)
                {
                    ZSynthWintel::GetEaxProps(&m_pEaxListener->m_Properties,
                        static_cast<const ZWintelRoomReverb*>(filter.m_pFilter));
                }
                else if (filter.m_lType == 3)
                {
                    auto& properties = m_pEaxListener->m_Properties;
                    properties = {};
                    properties.m_lEnvironment = 17;
                    properties.m_fEnvironmentSize = 100.0f;
                    properties.m_fEnvironmentDiffusion = 0.27f;
                    properties.m_lRoom = -1000;
                    properties.m_lRoomHF = -2500;
                    properties.m_fDecayTime = 0.85f;
                    properties.m_fDecayHFRatio = 0.21f;
                    properties.m_fDecayLFRatio = 1.0f;
                    properties.m_lReflections = -2780;
                    properties.m_fReflectionsDelay = 0.3f;
                    properties.m_lReverb = -1434;
                    properties.m_fReverbDelay = 0.1f;
                    properties.m_fEchoTime = 0.25f;
                    properties.m_fEchoDepth = 1.0f;
                    properties.m_fModulationTime = 0.25f;
                    properties.m_fAirAbsorptionHF = -5.0f;
                    properties.m_fHFReference = 5000.0f;
                    properties.m_fLFReference = 250.0f;
                    properties.m_fRoomRolloffFactor = 1.0f;
                    properties.m_lFlags = 31;
                }
                else if (filter.m_lType == 5)
                {
                    const auto* reverb = static_cast<const SInterpolatedRoomReverb*>(filter.m_pFilter);
                    if (reverb && m_pEaxListener)
                    {
                        EaxListenerProperties from{};
                        EaxListenerProperties to{};
                        ZSynthWintel::GetEaxProps(&from, &reverb->m_From);
                        ZSynthWintel::GetEaxProps(&to, &reverb->m_To);
                        InterpolateEaxProperties(from, to, reverb->m_fFactor,
                            m_pEaxListener->m_Properties);
                    }
                }
            }
            m_pEaxListener->Update();
        }

        const auto* vectors = reinterpret_cast<const ZVector3*>(
            reinterpret_cast<const char*>(_command) + 0x08);
        m_vFront = vectors[0];
        m_vTop = vectors[1];
        m_vEar = vectors[2];
        m_vPosition = vectors[3];

        if (!m_pDSListener)
            return;

        DS3DLISTENER parameters{};
        parameters.dwSize = sizeof(parameters);
        parameters.vPosition = {m_vPosition.x, m_vPosition.y, m_vPosition.z};
        parameters.vVelocity = {vectors[4].x, vectors[4].y, vectors[4].z};
        parameters.vOrientFront = {m_vFront.x, m_vFront.y, m_vFront.z};
        parameters.vOrientTop = {m_vTop.x, m_vTop.y, m_vTop.z};
        parameters.flDistanceFactor = 0.01f;
        parameters.flRolloffFactor = 1.0f;
        parameters.flDopplerFactor = 1.0f;
        m_pDSListener->SetAllParameters(&parameters, DS3D_DEFERRED);
        m_pDSListener->CommitDeferredSettings();
    }

    void ZSoundListenerDS::SetPrimary(IDirectSoundBuffer* _primary)
    {
        m_pPrimary = _primary;
    }
}
