#include <Glacier/Audio/ZSoundListenerDS.h>

#include <Glacier/Audio/ZSoundBufferDS.h>
#include <Glacier/Audio/ZEaxListener.h>
#include <Glacier/Audio/ZSynthDS.h>
#include <Glacier/Audio/ZSynthWintel.h>
#include <Glacier/Audio/ZWintelRoomReverb.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>

#include <dsound.h>

namespace Glacier
{
    ZSoundListenerDS::ZSoundListenerDS()
        : m_pDSListener(nullptr)
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
                    ZASSERT(false);
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
