#include <Glacier/Locomotion/ZWounded.h>

#include <Glacier/GameBase/ZActor.h>
#include <Glacier/GameBase/Boid/ZHumanBoid.h>


namespace Glacier::Locomotion
{
    void ZWounded::UpdatePosition(ZActor *pActor, float)
    {
        pActor->m_Locomotion.m_CurrentAnim = nullptr;
        pActor->m_pkBoid->SetMode(eSoftObstacle);
    }
}
