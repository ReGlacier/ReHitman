#include <Glacier/Locomotion/ZDead.h>

#include <Glacier/GameBase/ZActor.h>
#include <Glacier/GameBase/Boid/ZHumanBoid.h>


namespace Glacier::Locomotion
{
    void ZDead::UpdatePosition(ZActor *pActor, float)
    {
        pActor->m_pkBoid->SetMode(eSoftObstacle);
    }

    bool ZDead::TakeControl(ZActor *pActor, bool bNewControl)
    {
        if (bNewControl)
        {
            pActor->m_pkBoid->SetMode(eSoftObstacle);
            pActor->m_Locomotion.m_CurrentAnim = nullptr;
        }

        return bNewControl;
    }
}
