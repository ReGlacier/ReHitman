#include <Glacier/Locomotion/ZIdle.h>

#include <Glacier/GameBase/ZActor.h>
#include <Glacier/GameBase/Boid/ZHumanBoid.h>


namespace Glacier::Locomotion
{
    bool ZIdle::TakeControl(ZActor *pActor, bool bNewControl)
    {
        if (bNewControl && !pActor->m_bPositionLock
            && (pActor->m_Locomotion.m_State == LOCOSTATE_ACTIVEANIM
                || pActor->m_Locomotion.m_State == LOCOSTATE_MANUALANIM))
        {
            pActor->RestartLocomotionSystem();
        }

        return bNewControl;
    }

    bool ZIdle::ReleaseControl(ZActor *, bool bNewControl)
    {
        return bNewControl;
    }

    void ZIdle::UpdatePosition(ZActor *pActor, float fDeltaTime)
    {
        if (pActor->m_bPositionLock)
            return;

        const bool bVisible = (pActor->BaseGeom()->m_lControl & 0x1000) != 0;
        ZMat3x3 mActor;
        ZVector3 vActorPosition;
        pActor->GetActorRootTM(mActor, vActorPosition);

        ZVector3 vDirection(-mActor.data[0], -mActor.data[1], -mActor.data[2]);
        ZVector3 vPosition = pActor->m_pkBoid->m_kPosition;
        if (vActorPosition.x == vPosition.x && vActorPosition.z == vPosition.z)
        {
            vPosition.y = vActorPosition.y;
        }
        else
        {
            pActor->ShootIntoGroundRegularly(vPosition, false, false, fDeltaTime);
        }
        pActor->m_pkBoid->m_kPosition.y = vPosition.y;

        if (bVisible)
        {
            pActor->LocoFromMotion(vActorPosition, vDirection,
                vPosition, vDirection, false, fDeltaTime);
        }
        else
        {
            pActor->LocoFromMotionOffScreen(fDeltaTime);
        }

        pActor->m_pkBoid->SetMode(ePassivePushable);
    }
}
