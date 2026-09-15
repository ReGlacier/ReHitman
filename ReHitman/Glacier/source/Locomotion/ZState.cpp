#include <Glacier/Locomotion/ZState.h>


namespace Glacier::Locomotion
{
    void ZState::UpdatePosition(ZActor *, float)
    {
    }

    bool ZState::TakeControl(ZActor *, bool bNewControl)
    {
        return bNewControl;
    }

    bool ZState::ReleaseControl(ZActor *, bool bNewControl)
    {
        return bNewControl;
    }

    float ZState::ApproachSpeed(ZActor *)
    {
        return 0.0f;
    }

    float ZState::ApproachDirection(ZActor *, float *)
    {
        return 0.0f;
    }

    bool ZState::Interruptable(ZActor *)
    {
        return true;
    }

    bool ZState::Breakable(ZActor *)
    {
        return true;
    }

    void ZState::AnimEnd(ZActor *, Animation::ActiveAnimation *)
    {
    }

    void ZState::Initialize()
    {
    }

}
