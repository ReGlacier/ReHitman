#pragma once

#include <Glacier/Audio/ZSoundListener3D.h>

namespace Glacier
{
    class ZSoundListenerWintel : public ZSoundListener3D
    {
    };

    RE_VERIFY_SIZE(ZSoundListenerWintel, 0x44);
}
