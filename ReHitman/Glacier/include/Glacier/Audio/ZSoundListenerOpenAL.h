#pragma once

#include <Glacier/Audio/ZSoundListener3D.h>

namespace Glacier
{
    class ZSoundListenerOpenAL : public ZSoundListener3D
    {
    public:
        // vtbl
        ~ZSoundListenerOpenAL() override;
        bool Init() override;
        void Free() override;
        void Update(SSynthCmdBase* _command) override;
    };

    RE_VERIFY_SIZE(ZSoundListenerOpenAL, 0x44);
}
