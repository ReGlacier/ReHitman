#include <Glacier/Audio/ZSynthOpenAL.h>

#include <Glacier/Audio/ZSoundBufferOpenAL.h>
#include <Glacier/Audio/ZSoundListenerOpenAL.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    namespace
    {
        SSynthCaps g_SynthCapsAL = {0, "OpenAL", "OpenAL", 0, false};
    }

    ZSynthOpenAL::ZSynthOpenAL() = default;

    ZSynthOpenAL::~ZSynthOpenAL() = default;

    bool ZSynthOpenAL::Create()
    {
        return false;
    }

    _ZSoundBuffer* ZSynthOpenAL::AllocateBuffer()
    {
        return ZUniMemory::New<ZSoundBufferOpenAL>(this);
    }

    void ZSynthOpenAL::Free()
    {
        ZSynth::Free();
        Destroy();
    }

    _ZSoundBuffer* ZSynthOpenAL::Duplicate(_ZSoundBuffer*)
    {
        return nullptr;
    }

    void ZSynthOpenAL::CreateFilterChains()
    {
    }

    bool ZSynthOpenAL::CreateListener()
    {
        auto* listener = ZUniMemory::New<ZSoundListenerOpenAL>();
        m_pListener = listener;
        listener->m_pSynth = this;
        return listener->Init();
    }

    SSynthCaps* ZSynthOpenAL::GetCaps()
    {
        return &g_SynthCapsAL;
    }

    int ZSynthOpenAL::Probe()
    {
        // TODO: Finish this place after OpenAL loader will be reversed
        return 0;
    }

    void ZSynthOpenAL::Destroy()
    {
        // TODO: Finish this place after OpenAL loader will be reversed
    }

    bool ZSynthOpenAL::EndInit(void*)
    {
        return false;
    }

    bool ZSynthOpenAL::SetMasterPan(int)
    {
        return true;
    }

    bool ZSynthOpenAL::SetMasterVolume(float)
    {
        return false;
    }

    void ZSynthOpenAL::PrintStatus()
    {
    }

    bool ZSynthOpenAL::EaxEnabled()
    {
        return m_bEAXEnabled;
    }

    bool ZSynthOpenAL::EnableHardWare(bool _enabled)
    {
        m_bHardwareEnabled = _enabled;
        return _enabled;
    }

    bool ZSynthOpenAL::EnableEAX(bool _enabled)
    {
        m_bEAXEnabled = _enabled;
        return _enabled;
    }
}
