#pragma once

#include <Glacier/Audio/ZSynthWintel.h>

namespace Glacier
{
    class ZSynthOpenAL : public ZSynthWintel
    {
    public:
        // vtbl
        bool Create() override;
        _ZSoundBuffer* AllocateBuffer() override;
        ~ZSynthOpenAL() override;
        void Free() override;
        _ZSoundBuffer* Duplicate(_ZSoundBuffer* _buffer) override;
        void CreateFilterChains() override;
        bool CreateListener() override;
        SSynthCaps* GetCaps() override;
        int Probe() override;
        virtual void Destroy();
        virtual bool EndInit(void* _window);
        virtual bool SetMasterPan(int _pan);
        virtual bool SetMasterVolume(float _volume);
        virtual void PrintStatus();
        virtual bool EaxEnabled();
        virtual bool EnableHardWare(bool _enabled);
        virtual bool EnableEAX(bool _enabled);

        // methods
        ZSynthOpenAL();

        // members
        RE_ADD_PADDING(4);
        bool m_bEAXEnabled;
        bool m_bHardwareEnabled;
        RE_ADD_PADDING(2);
    };

    RE_VERIFY_OFFSET(ZSynthOpenAL, m_bEAXEnabled, 0x1BB38);
    RE_VERIFY_OFFSET(ZSynthOpenAL, m_bHardwareEnabled, 0x1BB39);
    RE_VERIFY_SIZE(ZSynthOpenAL, 0x1BB3C);
}
