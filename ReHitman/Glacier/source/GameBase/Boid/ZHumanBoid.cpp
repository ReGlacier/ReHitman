#include <Glacier/GameBase/Boid/ZHumanBoid.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/PF4/ZInterface.h>
#include <cmath>


namespace Glacier
{
    namespace
    {
        float Distance2D(const ZVector3& a, const ZVector3& b)
        {
            const float dx = a.x - b.x;
            const float dz = a.z - b.z;
            return sqrt(dx * dx + dz * dz);
        }

        float Distance2DSq(const ZVector3& a, const ZVector3& b)
        {
            const float dx = a.x - b.x;
            const float dz = a.z - b.z;
            return dx * dx + dz * dz;
        }
    }

    ZHumanBoid::ZHumanBoid(int lID, PF4::ZInterface* pPathFinder, PF4::ZMetaNode* pMetaNode, ZLNKWHANDS* pActor)
        : ZBoid(lID, ZVector3(0.f))
    {
        Initialize(lID, pPathFinder, pMetaNode, pActor);
    }

    void ZHumanBoid::Initialize(int lID, PF4::ZInterface* pPathFinder, PF4::ZMetaNode* pMetaNode, ZLNKWHANDS* pActor)
    {
        m_iID = lID;
        m_pActor = pActor;
        m_pMetaNode = pMetaNode;
        m_pkPathfinder = pPathFinder;

        m_Displacement.x = 0.0f;
        m_Displacement.y = 0.0f;
        m_ActualSpeed = 0.0f;
        m_fMaxSpeed = 170.0f;
        m_fLookAheadDistance = 100.0f;
        m_PathRamaining = 0.0f;
        m_SubTarget = ZVector3(0.0f);
        m_fSpeedMultiplier = 1.0f;
        m_fEndSpeed = 0.0f;
        m_vEndDir.x = 0.0f;
        m_vEndDir.y = 0.0f;
        m_fRemaining = 0.0f;
        m_fPauseMovementAtDistanc = -1.0f;
        m_bFollowSubTarget = false;
        m_bSkipTargets = false;
        m_PathEndsInObstacle = false;
        m_AvoidSide = 0;
        m_eState = ePassivePushable;

        m_Targets[0] = {};
        m_Targets[1] = {};

        ZVector3 vTarget(0.0f);
        ZVector3 vNormal(0.0f);
        SetTarget(vTarget, vNormal, 0.0f, true);
        SetTarget(vTarget, vNormal, 0.0f, true);
    }

    void ZHumanBoid::FrameUpdate(const stlp::vector<ZBoid*>& aBoids, float)
    {
        // TODO: Finish me
    }

    void ZHumanBoid::Move(const stlp::vector<ZBoid*>& aBoids, float)
    {
        // TODO: Finish me
    }

    void ZHumanBoid::MovePosition(const ZVector3& vPos)
    {
        if (vPos == m_kPosition)
            return;

        m_kPosition = vPos;
        m_Tracker = vPos;

        if (m_pkPathfinder && m_pMetaNode)
        {
            m_pkPathfinder->MoveNodeConstrained(m_pMetaNode, m_kPosition.Get());
            m_kPosition = m_pMetaNode->m_Location.m_vPos;
        }
    }

    bool ZHumanBoid::TeleportPosition(const ZVector3& vPos)
    {
        if (!(vPos == m_kPosition))
        {
            m_kPosition = vPos;

            if (m_pkPathfinder && m_pMetaNode)
            {
                m_pkPathfinder->TeleportNode(m_pMetaNode, m_kPosition.Get());
                m_kPosition = m_pMetaNode->m_Location.m_vPos;
            }

            m_Tracker = m_kPosition;
        }

        ZVector3 vRot(0.0f);
        SetTarget(m_kPosition, vRot, 0.0f, true);
        SetTarget(m_kPosition, vRot, 0.0f, true);

        return m_pMetaNode ? m_pMetaNode->m_Location.Inside() != 0 : true;
    }

    void ZHumanBoid::SetSpeed(float fSpeed)
    {
        ZASSERT(!isnan(fSpeed) && isfinite(fSpeed));

        m_fSpeed = fSpeed;
        m_ActualSpeed = fSpeed;
    }

    void ZHumanBoid::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZBoid::LoadSave(stream, bSaving);

        stream.Exchange("m_ActualSpeed", m_ActualSpeed);
        stream.Exchange("m_fMaxSpeed", m_fMaxSpeed);
        stream.Exchange("m_PathRemaining", m_PathRamaining);
        stream.Exchange("m_fSpeedMultiplier", m_fSpeedMultiplier);
        stream.Exchange("m_fEndSpeed", m_fEndSpeed);
        stream.ExchangeArray("m_vEndDir", &m_vEndDir.x, 2);
        stream.Exchange("m_PathEndsInObstacle", m_PathEndsInObstacle);
        stream.Exchange("m_fPauseMovementAtDistance", m_fPauseMovementAtDistanc);

        if (!bSaving)
        {
            ZVector3 vRot(0.0f);
            SetTarget(m_kPosition, vRot, 0.0f, true);
            SetTarget(m_kPosition, vRot, 0.0f, true);
        }
    }

    ZVector3 ZHumanBoid::AvoidBoids(const stlp::vector<ZBoid*>&, float, float&)
    {
        // TODO: Finish me
        return {};
    }

    void ZHumanBoid::LoadSaveSubTarget(ISerializerStream& stream, bool bSaving)
    {
        bool bFollowSubTarget = m_bFollowSubTarget;
        stream.Exchange("m_bFollowSubTarget", m_bFollowSubTarget);
        stream.ExchangeArray("m_SubTarget", m_SubTarget, 3);

        m_bFollowSubTarget = bFollowSubTarget;
    }

    void ZHumanBoid::SetTarget(ZVector3& vPosition, Vector3& vRotation, float a4, bool bForced)
    {
        SetTarget();

        if (bForced)
            m_Mask &= ~2;

        m_Targets[1].m_vPos = vPosition;
        m_Targets[1].m_vNormal = vRotation;
        m_Targets[1].m_bEndPoint = bForced;
        m_Targets[1].m_fRemaining = a4;

        if (m_pkPathfinder)
            m_pkPathfinder->MapLocation(vPosition.Get(), m_Targets[1].m_Loc);
    }

    void ZHumanBoid::SetTarget()
    {
        m_Targets[0] = m_Targets[1];
        m_bFollowSubTarget = false;
        m_PathEndsInObstacle = false;
    }

    void ZHumanBoid::GetLocomotionInfo(float& fSpeed, float* pPosition, float* pTracker, float& fTrackerDistance, float& fRemaining, float* pDirection)
    {
        fSpeed = m_ActualSpeed;
        pPosition[0] = m_kPosition.x;
        pPosition[1] = m_kPosition.y;
        pPosition[2] = m_kPosition.z;
        pTracker[0] = m_Tracker.x;
        pTracker[1] = m_Tracker.y;
        pTracker[2] = m_Tracker.z;
        fTrackerDistance = TrackerDistance();
        fRemaining = Remaining();
        TargetDirection(pDirection);
    }

    void ZHumanBoid::Collision(const stlp::vector<ZBoid*>&, float)
    {
        // TODO: Finish me
    }

    EBoidState ZHumanBoid::GetMode() const
    {
        return m_eState;
    }

    void ZHumanBoid::SetMode(EBoidState eMode)
    {
        m_eState = eMode;
    }

    void ZHumanBoid::CheckMemberSanity(const ZVector3& vPosition)
    {
        ZASSERT(!isnan(vPosition.x) && isfinite(vPosition.x));
        ZASSERT(!isnan(vPosition.y) && isfinite(vPosition.y));
        ZASSERT(!isnan(vPosition.z) && isfinite(vPosition.z));
    }

    PF4::ZMetaNode* ZHumanBoid::Entity() const
    {
        return m_pMetaNode;
    }

    ETargetState ZHumanBoid::TargetState()
    {
        if (Distance2D(m_Targets[0].m_vPos, m_kPosition) < 1.0f)
        {
            if (!m_Targets[0].m_bEndPoint)
                return eSkipped;

            return Distance2D(m_Targets[0].m_vPos, m_Tracker) < 1.0f ? eReached : eInRoute;
        }

        if (!m_Targets[0].m_bEndPoint)
            return eInRoute;

        const ZVector3 vDirection = m_Targets[0].m_vPos - m_kPosition;
        const float fDistance = sqrt(vDirection.x * vDirection.x + vDirection.z * vDirection.z);

        if (fDistance <= 200.0f && (m_bFollowSubTarget || m_bSkipTargets) && HasDirectPathTo(m_Targets[0].m_Loc))
            return eSkipped;

        m_bSkipTargets = false;
        return eInRoute;
    }

    void ZHumanBoid::TargetDirection(float* pDirection) const
    {
        const ZVector3& vTarget = m_bFollowSubTarget ? m_SubTarget : m_Targets[0].m_vPos;
        pDirection[0] = vTarget.x - m_kPosition.x;
        pDirection[1] = vTarget.z - m_kPosition.z;
    }

    void ZHumanBoid::TrackerDirection(float* pDirection) const
    {
        pDirection[0] = m_kPosition.x - m_Tracker.x;
        pDirection[1] = m_kPosition.z - m_Tracker.z;
    }

    float ZHumanBoid::TrackerDistance() const
    {
        return Distance2DSq(m_Tracker, m_kPosition);
    }

    float ZHumanBoid::Remaining() const
    {
        if (m_eState)
            return 0.0f;

        if (!m_bFollowSubTarget)
            return Distance2D(m_Tracker, m_Targets[0].m_vPos) + m_Targets[0].m_fRemaining;

        float fD0 = Distance2D(m_Tracker, m_SubTarget);
        return Distance2D(m_SubTarget, m_Targets[0].m_vPos) + fD0 + m_Targets[0].m_fRemaining;
    }

    float ZHumanBoid::TotalRemaining() const
    {
        return Remaining() + m_PathRamaining;
    }

    float ZHumanBoid::BoidRemaining() const
    {
        if (m_eState)
            return 0.0f;

        float fRemaining;

        if (m_bFollowSubTarget)
        {
            fRemaining = Distance2D(m_kPosition, m_SubTarget) + Distance2D(m_SubTarget, m_Targets[0].m_vPos);
        }
        else
        {
            fRemaining = Distance2D(m_kPosition, m_Targets[0].m_vPos);
        }

        return fRemaining + m_Targets[0].m_fRemaining;
    }

    void ZHumanBoid::SetPathRemaining(float fRem)
    {
        m_PathRamaining = fRem;
    }

    void ZHumanBoid::SetMaxSpeed(float fMaxSpeed)
    {
        m_fMaxSpeed = fMaxSpeed;
    }

    float ZHumanBoid::GetMaxSpeed() const
    {
        return m_fMaxSpeed;
    }

    void ZHumanBoid::SetEndSpeed(float fEndSpeed)
    {
        m_fEndSpeed = fEndSpeed;
    }

    float ZHumanBoid::GetEndSpeed() const
    {
        return m_fEndSpeed;
    }

    void ZHumanBoid::SetEndDirection(const float* pEndDir)
    {
        m_vEndDir.x = pEndDir[0];
        m_vEndDir.y = pEndDir[1];
    }

    float ZHumanBoid::GetSpeedMultiplier() const
    {
        return m_fSpeedMultiplier;
    }

    bool ZHumanBoid::IsFollowingSubTarget() const
    {
        return m_bFollowSubTarget;
    }

    void ZHumanBoid::PauseMovementAt(float fMoment)
    {
        m_fPauseMovementAtDistanc = fMoment;
    }

    void ZHumanBoid::UnpauseMovement()
    {
        m_fPauseMovementAtDistanc = -1.0f;
    }

    bool ZHumanBoid::IsMovementPausing() const
    {
        return m_fPauseMovementAtDistanc >= 0.0f;
    }

    bool ZHumanBoid::IsMovementPaused() const
    {
        if (IsMovementPausing())
            return m_ActualSpeed <= 0.0f;

        return false;
    }

    bool ZHumanBoid::PathEndsInObstacle() const
    {
        return m_PathEndsInObstacle;
    }

    bool ZHumanBoid::HasDirectPathTo(const PF4::ZLocation& rLoc)
    {
        // TODO: Finish me
        return false;
    }

    void ZHumanBoid::FollowPath(float)
    {
        // TODO: Finish me
    }

    void ZHumanBoid::SetEndDirection(const ZVector3& vEndDir)
    {
        m_vEndDir.x = vEndDir.x;
        m_vEndDir.y = vEndDir.z;
    }

    void ZHumanBoid::GetEndDirection(ZVector3& vEndDir)
    {
        vEndDir.x = m_vEndDir.x;
        vEndDir.y = 0.0f;
        vEndDir.z = m_vEndDir.y;
    }

    ZVector3 ZHumanBoid::AvoidWallIntersections(const ZVector3& vPosition, float fDistance, float& fResult)
    {
        // TODO: Finish this place after PF4::ZInterface will be reversed
        fResult = fDistance;
        return vPosition;
    }

    uint32_t ZHumanBoid::AvoidDynamicObstacles()
    {
        // TODO: Finish this place after PF4::ZInterface will be reversed
        return 0;
    }

    void ZHumanBoid::AvoidObstacles()
    {
        // TODO: Finish this place after PF4::ZInterface will be reversed
    }

    float ZHumanBoid::MovingPointCircleIntersection(ZVector3 vCirclePosition, ZVector3 vCircleVelocity, ZVector3 vPointPosition, ZVector3 vPointVelocity, float fRadius)
    {
        const float relPosX = vCirclePosition.x - vPointPosition.x;
        const float relPosZ = vCirclePosition.z - vPointPosition.z;
        const float relVelX = vCircleVelocity.x - vPointVelocity.x;
        const float relVelZ = vCircleVelocity.z - vPointVelocity.z;
        const float relPosSq = relPosX * relPosX + relPosZ * relPosZ;
        const float radiusSq = fRadius * fRadius;

        if (relPosSq <= radiusSq)
            return 0.0f;

        const float relVelSq = relVelX * relVelX + relVelZ * relVelZ;
        const float dot = 2.0f * (relPosX * relVelX + relPosZ * relVelZ);
        const float c = relPosSq - radiusSq;
        const float discriminant = dot * dot - 4.0f * relVelSq * c;

        if (discriminant <= 0.0f || relVelSq == 0.0f)
            return -1.0f;

        const float root = sqrt(discriminant);
        const float t0 = (root - dot) * 0.5f / relVelSq;
        const float t1 = (-root - dot) * 0.5f / relVelSq;

        if (t0 <= t1 && t0 >= 0.0f)
            return t0;

        if (t1 > t0 || t1 < 0.0f)
            return -1.0f;

        return t1;
    }

    void ZHumanBoid::MovePool(const stlp::vector<ZBoid*>& aBoids, float fTimeDelta)
    {
        Move(aBoids, fTimeDelta);
    }

    ZVector3 ZHumanBoid::HardKeepInsideWalls(const ZVector3& vPosition)
    {
        // TODO: Finish this place after PF4::ZInterface will be reversed
        return vPosition;
    }

    void ZHumanBoid::TeleportPositionBoidOnly(const ZVector3& vPos)
    {
        if (!(vPos == m_kPosition))
        {
            m_kPosition = vPos;

            if (m_pkPathfinder && m_pMetaNode)
            {
                m_pkPathfinder->TeleportNode(m_pMetaNode, m_kPosition.Get());
                m_kPosition = m_pMetaNode->m_Location.m_vPos;
            }
        }

        ZVector3 vRot(0.0f);
        SetTarget(m_kPosition, vRot, 0.0f, true);
        SetTarget(m_kPosition, vRot, 0.0f, true);
    }

    void ZHumanBoid::TeleportToPFLocation(const PF4::ZLocation& rLoc)
    {
        TeleportPosition(rLoc.m_vPos);
    }

    void ZHumanBoid::MoveTracker(float fSpeed, float fDeltaX, float fDeltaZ, float fTimeDelta)
    {
        if (fTimeDelta <= 0.0f)
            return;

        if (fSpeed == -1.0f)
        {
            m_Tracker.x += fDeltaX;
            m_Tracker.z += fDeltaZ;
            m_fSpeed = sqrt(fDeltaX * fDeltaX + fDeltaZ * fDeltaZ) / fTimeDelta;
            m_ActualSpeed = m_fSpeed;
            return;
        }

        const float dx = m_kPosition.x - m_Tracker.x;
        const float dz = m_kPosition.z - m_Tracker.z;
        const float distanceSq = dx * dx + dz * dz;

        if (distanceSq <= 0.0f)
        {
            m_fSpeed = 0.0f;
            m_ActualSpeed = 0.0f;
            return;
        }

        const float distance = sqrt(distanceSq);
        float moveDistance = fSpeed * fTimeDelta;

        if (moveDistance > distance)
            moveDistance = distance;

        const float factor = moveDistance / distance;
        m_Tracker.x += dx * factor;
        m_Tracker.z += dz * factor;
        m_fSpeed = factor * distance / fTimeDelta;
        m_ActualSpeed = m_fSpeed;
    }

    void ZHumanBoid::MoveTrackerAndSetSpeed(float fSpeed, float fDistance, float fTimeDelta)
    {
        if (fTimeDelta <= 0.0f)
            return;

        const float dx = m_kPosition.x - m_Tracker.x;
        const float dz = m_kPosition.z - m_Tracker.z;
        const float distanceSq = dx * dx + dz * dz;

        if (distanceSq <= 0.0f)
        {
            m_ActualSpeed = 0.0f;
        }
        else
        {
            const float distance = sqrt(distanceSq);

            if (distance < fDistance)
                fDistance = distance;

            const float factor = fDistance / distance;
            m_Tracker.x += dx * factor;
            m_Tracker.z += dz * factor;
            m_ActualSpeed = fDistance / fTimeDelta;
        }

        m_fSpeed = fSpeed;
    }

    void ZHumanBoid::Draw()
    {
        // TODO: Finish this place after ZHumanBoid drawing dependencies will be reversed
    }

    bool ZHumanBoid::CalcPathSide(const float* pDirection)
    {
        // TODO: Finish this place after PF4::ZInterface will be reversed
        return pDirection && pDirection[0] >= 0.0f;
    }

    bool ZHumanBoid::CheckPathSide(const float* pDirection, bool bSide)
    {
        // TODO: Finish this place after PF4::ZInterface will be reversed
        return CalcPathSide(pDirection) == bSide;
    }
}
