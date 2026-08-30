#include <Glacier/Audio/ZSynthDP.h>

#include <Glacier/Audio/ZSoundBufferDP.h>
#include <Glacier/Audio/ZSoundListenerDP.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>

namespace Glacier
{
    namespace
    {
        SSynthCaps g_SynthCapsDP{};
    }

    ZSynthDP::ZSynthDP()
        : m_pDieselPower(nullptr)
    {
    }

    ZSynthDP::~ZSynthDP() = default;

    bool ZSynthDP::Create()
    {
        // TODO: Finish this place after IDieselPower will be reversed
        return false;
    }

    _ZSoundBuffer* ZSynthDP::AllocateBuffer()
    {
        return ZUniMemory::New<ZSoundBufferDP>(this);
    }

    void ZSynthDP::Free()
    {
        ZSynth::Free();
        Destroy();
    }

    bool ZSynthDP::CreateListener()
    {
        auto* listener = ZUniMemory::New<ZSoundListenerDP>();
        m_pListener = listener;
        listener->m_pSynth = this;
        return listener->Init();
    }

    SSynthCaps* ZSynthDP::GetCaps()
    {
        return &g_SynthCapsDP;
    }

    int ZSynthDP::Probe()
    {
        std::strcpy(g_SynthCapsDP.m_szDescription, "AM:3D DieselPower");
        std::strcpy(g_SynthCapsDP.m_szName, "DieselPower");
        if (!Create())
            return 0;
        g_SynthCapsDP.m_bEAX = false;
        g_SynthCapsDP.m_lMaxBuffers = 32;
        Destroy();
        return 1;
    }

    void ZSynthDP::Destroy()
    {
        // TODO: Finish this place after IDieselPower will be reversed
        m_pDieselPower = nullptr;
    }
}
