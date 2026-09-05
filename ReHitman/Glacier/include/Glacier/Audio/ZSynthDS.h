#pragma once

#include <Glacier/Audio/ZSynthWintel.h>

#include <dsound.h>

namespace Glacier
{
    class ZSynthDS : public ZSynthWintel
    {
    public:
        // vtbl
        bool Create() override;
        _ZSoundBuffer* AllocateBuffer() override;
        ~ZSynthDS() override;
        void Free() override;
        void CommitFrame() override;
        void Render() override;
        _ZSoundBuffer* Duplicate(_ZSoundBuffer* _buffer) override;
        void CreateFilterChains() override;
        bool CreateListener() override;
        SSynthCaps* GetCaps() override;
        int Probe() override;
        virtual bool CreatePrimaryBuffer();
        virtual void Destroy();
        virtual bool EndInit(void* _window);
        virtual bool SetMasterPan(int _pan);
        virtual bool SetMasterVolume(float _volume);
        virtual void PrintStatus();
        virtual IDirectSound8* GetDS();
        virtual bool EaxEnabled();
        virtual bool EnableHardWare(bool _enabled);
        virtual bool EnableEAX(bool _enabled);
        virtual bool IsProbeActive();

        // methods
        ZSynthDS();

        // members
        IDirectSound8* m_pDirectSound;
        IDirectSoundBuffer* m_pPrimaryBuffer;
        bool m_bProbeActive;
        bool m_bEnableHardware;
        RE_ADD_PADDING(2);
        DSCAPS m_DSCaps;
    };

    RE_VERIFY_OFFSET(ZSynthDS, m_pDirectSound, 0x1BB34);
    RE_VERIFY_OFFSET(ZSynthDS, m_pPrimaryBuffer, 0x1BB38);
    RE_VERIFY_OFFSET(ZSynthDS, m_bProbeActive, 0x1BB3C);
    RE_VERIFY_OFFSET(ZSynthDS, m_bEnableHardware, 0x1BB3D);
    RE_VERIFY_OFFSET(ZSynthDS, m_DSCaps, 0x1BB40);
    RE_VERIFY_SIZE(ZSynthDS, 0x1BBA0);
}
