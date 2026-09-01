#pragma once

#include <Glacier/Locomotion/ZState.h>


namespace Glacier::Locomotion
{
    class ZIdle : public ZState
    {
    public:
        void UpdatePosition(ZActor *, float) override;
        bool TakeControl(ZActor *, bool) override;
        bool ReleaseControl(ZActor *, bool) override;
    };

    RE_VERIFY_SIZE(ZIdle, 0x4);
}
