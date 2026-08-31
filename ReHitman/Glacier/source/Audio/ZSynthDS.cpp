#include <Glacier/Audio/ZSynthDS.h>

#include <Glacier/Audio/ZSoundBufferDS.h>
#include <Glacier/Audio/ZSoundListenerDS.h>
#include <Glacier/Audio/ZEaxSource.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>

namespace Glacier
{
    namespace
    {
        SSynthCaps g_SynthCapsDS{};

        BOOL CALLBACK EnumerateSoundDevices(LPGUID, LPCSTR, LPCSTR, LPVOID)
        {
            return TRUE;
        }
    }

    ZSynthDS::ZSynthDS()
        : m_pDirectSound(nullptr)
        , m_pPrimaryBuffer(nullptr)
        , m_bProbeActive(false)
        , m_bEnableHardware(true)
    {
        m_bUseEAX = false;
    }

    ZSynthDS::~ZSynthDS() = default;

    bool ZSynthDS::Create()
    {
        if (FAILED(DirectSoundEnumerateA(EnumerateSoundDevices, nullptr)))
            return false;
        if (FAILED(DirectSoundCreate8(nullptr, &m_pDirectSound, nullptr)))
            return false;
        if (FAILED(m_pDirectSound->SetCooperativeLevel(static_cast<HWND>(GetWindowHandle()), DSSCL_PRIORITY)))
            return false;

        m_DSCaps.dwSize = sizeof(m_DSCaps);
        if (FAILED(m_pDirectSound->GetCaps(&m_DSCaps)))
            return false;
        return CreatePrimaryBuffer();
    }

    _ZSoundBuffer* ZSynthDS::AllocateBuffer()
    {
        return ZUniMemory::New<_ZSoundBufferDS>(this);
    }

    void ZSynthDS::Free()
    {
        ZSynth::Free();
        Destroy();
    }

    void ZSynthDS::CommitFrame()
    {
        ZSynthWintel::CommitFrame();
    }

    void ZSynthDS::Render()
    {
        ZSynthWintel::Render();
    }

    _ZSoundBuffer* ZSynthDS::Duplicate(_ZSoundBuffer* _buffer)
    {
        auto* source = static_cast<_ZSoundBufferDS*>(_buffer);
        auto* destination = ZUniMemory::New<_ZSoundBufferDS>(this);
        destination->m_rWave = source->m_rWave;
        destination->m_dwBufferType = source->m_dwBufferType;

        if (FAILED(m_pDirectSound->DuplicateSoundBuffer(source->m_pDSBuffer,
                &destination->m_pDSBuffer)))
        {
            ZUniMemory::Delete(destination);
            return nullptr;
        }
        destination->m_pDSBuffer->SetVolume(DSBVOLUME_MIN);

        if (source->m_dwBufferType >= 1 && source->m_dwBufferType <= 11)
            return destination;

        if (FAILED(destination->m_pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer,
                reinterpret_cast<void**>(&destination->m_pDS3DBuffer))))
        {
            ZUniMemory::Delete(destination);
            return nullptr;
        }

        if (EaxEnabled())
        {
            destination->m_pEaxSource = ZUniMemory::New<ZEaxSource>();
            destination->m_pEaxSource->Init(destination->m_pDSBuffer, true);
        }
        return destination;
    }

    void ZSynthDS::CreateFilterChains()
    {
    }

    bool ZSynthDS::CreateListener()
    {
        auto* listener = ZUniMemory::New<ZSoundListenerDS>();
        m_pListener = listener;
        listener->SetPrimary(m_pPrimaryBuffer);
        listener->m_pSynth = this;
        return listener->Init();
    }

    SSynthCaps* ZSynthDS::GetCaps()
    {
        return &g_SynthCapsDS;
    }

    int ZSynthDS::Probe()
    {
        if (!Create())
            return 0;

        std::strcpy(g_SynthCapsDS.m_szName, "DirectSound");
        std::strcpy(g_SynthCapsDS.m_szDescription, "Direct Sound");
        m_bEnableHardware = true;
        m_bProbeActive = true;

        _ZSoundBufferDS* buffers[128]{};
        int count = 0;
        SWaveHeader wave{};
        wave.m_iDataType = 1;
        wave.m_lSampleRate = 22050;
        wave.m_lBitsPerSample = 16;
        wave.m_lDataSize = 32;
        wave.m_lNumChannels = 1;
        wave.m_lNumSamples = 16;
        wave.m_lBlockAlign = 2;
        wave.m_lSamplesPerBlock = 2;

        while (count < 128)
        {
            auto* buffer = static_cast<_ZSoundBufferDS*>(AllocateBuffer());
            if (!buffer->Create(&wave, 0, 0))
            {
                ZUniMemory::Delete(buffer);
                break;
            }
            buffers[count++] = buffer;
        }

        if (!count)
        {
            std::strcat(g_SynthCapsDS.m_szDescription, " (SW)");
            m_bEnableHardware = false;
            while (count < 64)
            {
                auto* buffer = static_cast<_ZSoundBufferDS*>(AllocateBuffer());
                if (!buffer->Create(&wave, 0, 0))
                {
                    ZUniMemory::Delete(buffer);
                    break;
                }
                buffers[count++] = buffer;
            }
        }

        g_SynthCapsDS.m_lMaxBuffers = count;
        for (int i = 0; i < count; ++i)
            ZUniMemory::Delete(buffers[i]);

        g_SynthCapsDS.m_bEAX = false;
        if (m_bEnableHardware)
        {
            auto* buffer = static_cast<_ZSoundBufferDS*>(AllocateBuffer());
            if (buffer->Create(&wave, 0, 0))
            {
                ZEaxSource eax;
                g_SynthCapsDS.m_bEAX = eax.Init(buffer->m_pDSBuffer, false) && eax.QueryEAXSupport();
            }
            ZUniMemory::Delete(buffer);
        }

        m_bProbeActive = false;
        Destroy();
        return 1;
    }

    bool ZSynthDS::CreatePrimaryBuffer()
    {
        DSBUFFERDESC description{};
        description.dwSize = sizeof(description);
        description.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
        if (FAILED(m_pDirectSound->CreateSoundBuffer(&description, &m_pPrimaryBuffer, nullptr)))
        {
            description.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;
            if (FAILED(m_pDirectSound->CreateSoundBuffer(&description, &m_pPrimaryBuffer, nullptr)))
                return false;
        }

        WAVEFORMATEX format{};
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = 2;
        format.nSamplesPerSec = 44100;
        format.nAvgBytesPerSec = 176400;
        format.nBlockAlign = 4;
        format.wBitsPerSample = 16;
        format.cbSize = 18;
        if (FAILED(m_pPrimaryBuffer->SetFormat(&format)))
            return false;
        return SUCCEEDED(m_pPrimaryBuffer->Play(0, 0, DSBPLAY_LOOPING));
    }

    void ZSynthDS::Destroy()
    {
        if (m_pPrimaryBuffer)
        {
            m_pPrimaryBuffer->Stop();
            m_pPrimaryBuffer->Release();
            m_pPrimaryBuffer = nullptr;
        }
        if (m_pDirectSound)
        {
            m_pDirectSound->Release();
            m_pDirectSound = nullptr;
        }
    }

    bool ZSynthDS::EndInit(void*) { return true; }
    bool ZSynthDS::SetMasterPan(int) { return true; }
    bool ZSynthDS::SetMasterVolume(float) { return true; }
    void ZSynthDS::PrintStatus()
    {
        m_DSCaps.dwSize = sizeof(m_DSCaps);
        m_pDirectSound->GetCaps(&m_DSCaps);
    }
    IDirectSound8* ZSynthDS::GetDS() { return m_pDirectSound; }
    bool ZSynthDS::EaxEnabled() { return m_bUseEAX; }
    bool ZSynthDS::EnableHardWare(bool _enabled) { return m_bEnableHardware = _enabled; }
    bool ZSynthDS::EnableEAX(bool _enabled) { return m_bUseEAX = _enabled; }
    bool ZSynthDS::IsProbeActive() { return m_bProbeActive; }
}
