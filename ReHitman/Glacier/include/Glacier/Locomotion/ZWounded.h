#pragma once

#include <Glacier/Locomotion/ZState.h>


namespace Glacier::Locomotion
{
    class ZWounded : public ZState
    {
    public:
        void UpdatePosition(ZActor *, float) override;
    };

    RE_VERIFY_SIZE(ZWounded, 0x4);
}
