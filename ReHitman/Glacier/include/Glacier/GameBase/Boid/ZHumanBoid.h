#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/GameBase/Boid/EBoidState.h>
#include <Glacier/GameBase/Boid/ZBoid.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/PF4/ZLocation.h>
#include <Glacier/PF4/Fwd.h>


namespace Glacier
{
    enum ETargetState
    {
        eUnreachable = 0,
        eReached = 1,
        eSkipped = 2,
        eInRoute = 3,
    };

    class ZHumanBoid : public ZBoid
    {
    public:
        // types
        struct ZTarget
        {
            // members
            ZVector3 m_vPos;
            ZVector3 m_vNormal;
            bool m_bEndPoint;
            float m_fRemaining;
            PF4::ZLocation m_Loc;
        };
        RE_VERIFY_SIZE(ZTarget, 0x34);

        // vtbl
        void FrameUpdate(const stlp::vector<ZBoid*>& aBoids, float) override;
        void Move(const stlp::vector<ZBoid*>& aBoids, float) override;
        void MovePosition(const ZVector3& vPos) override;
        bool TeleportPosition(const ZVector3& vPos) override;
        void SetSpeed(float fSpeed) override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;
        virtual ZVector3 AvoidBoids(const stlp::vector<ZBoid*>&, float, float&);
        virtual void LoadSaveSubTarget(ISerializerStream& stream, bool bSaving);

        // methods
        ZHumanBoid(int lID, PF4::ZInterface* pPathFinder, PF4::ZMetaNode* pMetaNode, ZLNKWHANDS* pActor);
        void Initialize(int lID, PF4::ZInterface* pPathFinder, PF4::ZMetaNode* pMetaNode, ZLNKWHANDS* pActor);
        void SetTarget(ZVector3& vPosition, Vector3& vRotation, float a4, bool bForced);
        void SetTarget();
        void GetLocomotionInfo(float& fSpeed, float* pPosition, float* pTracker, float& fTrackerDistance, float& fRemaining, float* pDirection);
        void Collision(const stlp::vector<ZBoid*>&, float);
        ZVector3 AvoidWallIntersections(const ZVector3& vPosition, float fDistance, float& fResult);
        uint32_t AvoidDynamicObstacles();
        void AvoidObstacles();
        float MovingPointCircleIntersection(ZVector3 vCirclePosition, ZVector3 vCircleVelocity, ZVector3 vPointPosition, ZVector3 vPointVelocity, float fRadius);
        void SetMode(EBoidState eMode);
        EBoidState GetMode() const;
        void CheckMemberSanity(const ZVector3& vPosition);
        PF4::ZMetaNode* Entity() const;
        ETargetState TargetState();
        void TargetDirection(float* pDirection) const;
        void TrackerDirection(float* pDirection) const;
        float TrackerDistance() const;
        void SetEndSpeed(float fEndSpeed);
        float GetEndSpeed() const;
        void SetEndDirection(const float* pEndDir);
        float Remaining() const;
        float TotalRemaining() const;
        float BoidRemaining() const;
        void SetPathRemaining(float fRem);
        void SetMaxSpeed(float fMaxSpeed);
        float GetMaxSpeed() const;
        float GetSpeedMultiplier() const;
        bool IsFollowingSubTarget() const;
        void PauseMovementAt(float);
        void UnpauseMovement();
        bool IsMovementPausing() const;
        bool IsMovementPaused() const;
        bool PathEndsInObstacle() const;
        bool HasDirectPathTo(const PF4::ZLocation& rLoc);
        void MovePool(const stlp::vector<ZBoid*>& aBoids, float fTimeDelta);
        ZVector3 HardKeepInsideWalls(const ZVector3& vPosition);
        void TeleportPositionBoidOnly(const ZVector3& vPos);
        void TeleportToPFLocation(const PF4::ZLocation& rLoc);
        void MoveTracker(float fSpeed, float fDeltaX, float fDeltaZ, float fTimeDelta);
        void MoveTrackerAndSetSpeed(float fSpeed, float fDistance, float fTimeDelta);
        void SetEndDirection(const ZVector3& vEndDir);
        void GetEndDirection(ZVector3& vEndDir);

    private:
        void Draw() override;

    protected:
        void FollowPath(float fTimeDelta);
        bool CalcPathSide(const float* pDirection);
        bool CheckPathSide(const float* pDirection, bool bSide);

    public:
        // members
        ZTarget m_Targets[2];
        ZVector2 m_Displacement;
        float m_ActualSpeed;
        float m_fMaxSpeed;
        float m_fLookAheadDistance;
        PF4::ZInterface* m_pkPathfinder;
        float m_PathRamaining;
        ZVector3 m_SubTarget;
        float m_fSpeedMultiplier;
        float m_fEndSpeed;
        ZVector2 m_vEndDir;
        float m_fRemaining;
        float m_fPauseMovementAtDistanc;
        bool m_bFollowSubTarget;
        bool m_bSkipTargets;
        bool m_PathEndsInObstacle;
        int8_t m_AvoidSide;
    };
    RE_VERIFY_SIZE(ZHumanBoid, 0xEC); // Verified
}
