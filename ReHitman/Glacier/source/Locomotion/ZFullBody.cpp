#include <Glacier/Locomotion/ZFullBody.h>

#include <Glacier/GameBase/ZActor.h>
#include <Glacier/GameBase/Boid/ZHumanBoid.h>
#include <Glacier/IK/ZBoneModifyBase.h>


namespace Glacier::Locomotion
{
    bool ZFullBody::TakeControl(ZActor *pActor, bool bNewControl)
    {
        if (bNewControl)
        {
            pActor->m_bPositionLock = true;
            if (pActor->m_bShootIntoGround && !pActor->m_bCutSequence)
            {
                ZVector3 vPosition = pActor->m_pkBoid->m_kPosition;
                const float fPositionHeight = vPosition.y;
                pActor->ShootIntoGround(vPosition, true, true);
                pActor->SetActorWorldPosition(vPosition);
                pActor->m_pkBoid->m_kPosition.y = fPositionHeight;
            }

            pActor->m_Locomotion.m_CurrentAnim = nullptr;
        }

        return bNewControl;
    }

    bool ZFullBody::ReleaseControl(ZActor *pActor, bool bNewControl)
    {
        if (bNewControl)
        {
            const bool bPerformFullUpdate = Glacier::g_PerformFullUpdate;
            Glacier::g_PerformFullUpdate = true;
            UpdatePosition(pActor, 0.0f);
            Glacier::g_PerformFullUpdate = bPerformFullUpdate;
        }

        return bNewControl;
    }

    void ZFullBody::UpdatePosition(ZActor *pActor, float)
    {
        ZMat3x3 mActor;
        ZVector3 vActorPosition;
        pActor->GetActorRootTM(mActor, vActorPosition);

        ZVector3 vPosition = vActorPosition;
        ZVector3 vDirection;
        if (Glacier::g_PerformFullUpdate)
        {
            vmmul(vPosition, pActor->m_Ground._Pos, mActor);
            vPosition += vActorPosition;
            mmmul(mActor, pActor->m_Ground._Mat);
            vDirection = {mActor.data[0], mActor.data[1], mActor.data[2]};
        }

        const bool bVisible = (pActor->BaseGeom()->m_lControl & 0x1000) != 0;
        bool bPositionAdjusted = false;
        if ((pActor->m_bShootIntoGroundInFullbody || bVisible)
            && !pActor->GetBoneModifier()->IsRagdollActive())
        {
            pActor->ShootIntoGround(vPosition, true, true);
            bPositionAdjusted = true;
        }

        if (Glacier::g_PerformFullUpdate)
            pActor->m_pkBoid->MovePosition(vPosition);
        else
            pActor->m_pkBoid->m_kPosition = vPosition;

        if (pActor->m_pkBoid->GetMode() != eInActive)
        {
            pActor->m_pkBoid->SetMode(pActor->GetBoneModifier()->IsRagdollActive()
                ? eSoftObstacle
                : eControlled);
            pActor->m_pkBoid->SetSpeed(0.0f);
        }

        if (Glacier::g_PerformFullUpdate)
        {
            vDirection.y = 0.0f;
            vnorm(vDirection);
            ZMat3x3 mDirection;
            createmat(mDirection, vDirection, nullptr);
            pActor->SetActorRootTM(mDirection, vPosition);
        }
        else if (bPositionAdjusted)
        {
            pActor->SetActorRootTM(mActor, vPosition);
        }
    }
}
