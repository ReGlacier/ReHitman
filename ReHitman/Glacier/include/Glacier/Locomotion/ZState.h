#pragma once

#include <Glacier/ReGlacier.h>


namespace Glacier
{
    class ZActor;

    namespace Animation
    {
        struct ActiveAnimation;
    }
}

namespace Glacier::Locomotion
{
    class ZState
    {
    public:
        virtual void UpdatePosition(ZActor *, float);
        virtual bool TakeControl(ZActor *, bool);
        virtual bool ReleaseControl(ZActor *, bool);
        virtual float ApproachSpeed(ZActor *);
        virtual float ApproachDirection(ZActor *, float *);
        virtual bool Interruptable(ZActor *);
        virtual bool Breakable(ZActor *);
        virtual void AnimEnd(ZActor *, Animation::ActiveAnimation *);
        virtual void Initialize();
    };

    RE_VERIFY_SIZE(ZState, 0x4);
}
