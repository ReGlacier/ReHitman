#include <Glacier/Locomotion/ZSpecialLink.h>


namespace Glacier::Locomotion
{
    void ZSpecialLink::UpdatePosition(ZActor *, float)
    {
    }

    bool ZSpecialLink::TakeControl(ZActor *, bool)
    {
        return false;
    }

    bool ZSpecialLink::ReleaseControl(ZActor *, bool bNewControl)
    {
        return bNewControl;
    }
}
