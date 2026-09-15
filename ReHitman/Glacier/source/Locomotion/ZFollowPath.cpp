#include <Glacier/Locomotion/ZFollowPath.h>
#include <Glacier/Locomotion/ZStates.h>

#include <Glacier/Animation/Model.h>
#include <Glacier/GameBase/ZActor.h>
#include <Glacier/GameBase/Boid/ZHumanBoid.h>


namespace Glacier::Locomotion
{
    void ZFollowPath::Initialize()
    {
    }

    bool ZFollowPath::ReleaseControl(ZActor *pActor, bool bNewControl)
    {
        return bNewControl || pActor->m_pkBoid->Remaining() < 0.5f;
    }

    bool ZFollowPath::TakeControl(ZActor *pActor, bool bNewControl)
    {
        if (!bNewControl)
            return false;

        if (Animation::Model* pModel = pActor->Model())
        {
            for (Animation::ActiveAnimation& animation : pModel->m_ActiveAnims)
            {
                if ((animation.mode & 0x2000) == 0
                    && ((animation.mode & 7) == 3 || (animation.mode & 7) == 1)
                    && (animation.mode & 0x400) != 0)
                {
                    animation.Destroy(true);
                    animation.mode = 0;
                }
            }
        }

        const int iQueueEntry = pActor->m_iQuePos;
        pActor->m_iNextPathTarget = pActor->m_ProgramQue[iQueueEntry].m_StartIndex;
        if (pActor->m_iNextPathTarget < pActor->m_ProgramQue[iQueueEntry].m_EndIndex)
        {
            ZVector3 vPosition;
            pActor->m_Path.GetPosition(pActor->m_iNextPathTarget, vPosition);
            const ZVector3& vBoidPosition = pActor->m_pkBoid->m_kPosition;
            const float fDeltaX = vPosition.x - vBoidPosition.x;
            const float fDeltaZ = vPosition.z - vBoidPosition.z;
            if (std::sqrt(fDeltaX * fDeltaX + fDeltaZ * fDeltaZ) <= 1.0f)
                ++pActor->m_iNextPathTarget;
        }

        SetBoidTargets(pActor, iQueueEntry + 1);
        pActor->m_pkBoid->SetMode(eFollowPath);
        pActor->m_fCurrentBaseMoveFramePrc = pActor->m_fCurrentBaseMoveDuration;
        if (pActor->m_iEndAction != 0 && pActor->m_iEndAction != 5)
        {
            pActor->m_Action[0] = -1;
            pActor->m_Action[1] = -1;
        }

        return true;
    }

    void ZFollowPath::UpdatePosition(ZActor *pActor, float fDeltaTime)
    {
        if (pActor->m_bPositionLock || pActor->m_Path.m_Size <= 0)
            return;

        if ((pActor->m_pkBoid->m_bFollowSubTarget & 4) != 0)
        {
            pActor->m_bCancelPathWhenPossible = true;
            return;
        }

        switch (pActor->m_pkBoid->TargetState())
        {
            case eSkipped:
                if (pActor->m_iNextPathTarget < pActor->m_ProgramQue[pActor->m_iQuePos].m_EndIndex)
                {
                    ++pActor->m_iNextPathTarget;
                    SetBoidTargets(pActor, pActor->m_iQuePos);
                }
                break;

            case eUnreachable:
                pActor->MoveToPosition(&pActor->m_vEndPos,
                    pActor->m_bUseEndDir ? &pActor->m_vEndDir : nullptr);
                break;

            case eReached:
            case eInRoute:
                break;
        }

        ZMat3x3 mActor;
        ZVector3 vActorPosition;
        pActor->GetActorRootTM(mActor, vActorPosition);

        ZVector3 vDirection(-mActor.data[0], -mActor.data[1], -mActor.data[2]);
        float fApproachSpeed = 0.0f;
        if (pActor->m_Action[1] != -1)
            fApproachSpeed = ZStates::GetState(pActor->m_Action[1])->ApproachSpeed(pActor);

        if (pActor->m_bDontStop)
            fApproachSpeed = pActor->m_MoveSetBindings.m_MoveSpeed * pActor->m_fMoveSpeedMultiplier;
        pActor->m_pkBoid->SetEndSpeed(fApproachSpeed);

        ZVector2 vEndDirection;
        if (pActor->m_bUseEndDir
            && pActor->m_ProgramQue[pActor->m_iQuePos].m_EndIndex == pActor->m_Path.m_Size - 1)
        {
            vEndDirection.x = pActor->m_vEndDir.x;
            vEndDirection.y = pActor->m_vEndDir.z;
        }
        pActor->m_pkBoid->SetEndDirection(&vEndDirection.x);

        pActor->m_pkBoid->SetMode(pActor->m_Path.m_Size > 0 ? eFollowPath : ePassivePushable);
        ZVector3 vPosition = pActor->m_pkBoid->m_kPosition;

        if ((pActor->BaseGeom()->m_lControl & 0x1000) != 0)
        {
            pActor->LocoFromMotion(vActorPosition, vDirection,
                vPosition, vDirection, fDeltaTime != 0.0f, -1.0f);
        }
        else
        {
            pActor->LocoFromMotionOffScreen(fDeltaTime);
        }
        (void)fApproachSpeed;
        (void)vEndDirection;
        (void)vPosition;
        (void)fDeltaTime;
    }

    void ZFollowPath::SetBoidTargets(ZActor *pActor, int iQueueEntry)
    {
        ZASSERT(iQueueEntry >= 0 && iQueueEntry < pActor->m_iProgramQueSize);

        const ProgramQueEntry& entry = pActor->m_ProgramQue[iQueueEntry];
        const int iEndIndex = entry.m_EndIndex;
        ZASSERT(entry.m_StartIndex <= pActor->m_iNextPathTarget
            && iEndIndex >= pActor->m_iNextPathTarget);

        for (int i = 0; i < 2; ++i)
        {
            const int iTarget = pActor->m_iNextPathTarget + i;
            ZVector3 vPosition;
            ZVector3 vNormal;
            pActor->m_Path.GetPosition(iTarget, vPosition);
            if (!pActor->m_Path.GetNormal(iTarget, vNormal))
                vNormal.Reset();

            const float fRemaining = pActor->m_Path.GetDistanceFromStart(iEndIndex)
                - pActor->m_Path.GetDistanceFromStart(iTarget);
            pActor->m_pkBoid->SetTarget(vPosition, vNormal, fRemaining, iTarget == iEndIndex);

            if (iTarget == iEndIndex)
            {
                if (i == 0)
                    pActor->m_pkBoid->SetTarget();
                break;
            }
        }

        pActor->m_pkBoid->SetPathRemaining(
            pActor->m_Path.m_Cost - pActor->m_Path.GetDistanceFromStart(iEndIndex));
    }
}
