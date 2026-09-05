#pragma once

#include <Glacier/Locomotion/ZState.h>


namespace Glacier::Locomotion
{
    class ZFollowPath : public ZState
    {
    public:
        void UpdatePosition(ZActor *, float) override;
        bool TakeControl(ZActor *, bool) override;
        bool ReleaseControl(ZActor *, bool) override;
        void Initialize() override;

        void SetBoidTargets(ZActor *, int);
    };

    RE_VERIFY_SIZE(ZFollowPath, 0x4);
}
