#pragma once

#include <Glacier/Audio/ZSynthWintel.h>

namespace Glacier
{
    struct IDieselPower;

    class ZSynthDP : public ZSynthWintel
    {
    public:
        // vtbl
        bool Create() override;
        _ZSoundBuffer* AllocateBuffer() override;
        ~ZSynthDP() override;
        void Free() override;
        bool CreateListener() override;
        SSynthCaps* GetCaps() override;
        int Probe() override;
        virtual void Destroy();

        // methods
        ZSynthDP();

        // members
        IDieselPower* m_pDieselPower;
    };

    RE_VERIFY_OFFSET(ZSynthDP, m_pDieselPower, 0x1BB34);
    RE_VERIFY_SIZE(ZSynthDP, 0x1BB38);
}
