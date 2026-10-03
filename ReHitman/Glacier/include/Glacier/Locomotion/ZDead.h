#pragma once

#include <Glacier/Locomotion/ZState.h>


namespace Glacier::Locomotion
{
    class ZDead : public ZState
    {
    public:
        void UpdatePosition(ZActor *, float) override;
        bool TakeControl(ZActor *, bool) override;
    };

    RE_VERIFY_SIZE(ZDead, 0x4);
}
