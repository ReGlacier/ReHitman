#pragma once

#include <Glacier/Audio/ZSoundListener3D.h>

namespace Glacier
{
    class ZSoundListenerDP : public ZSoundListener3D
    {
    public:
        // vtbl
        ~ZSoundListenerDP() override;
        bool Init() override;
        void Free() override;
        void Update(SSynthCmdBase* _command) override;
    };

    RE_VERIFY_SIZE(ZSoundListenerDP, 0x44);
}
