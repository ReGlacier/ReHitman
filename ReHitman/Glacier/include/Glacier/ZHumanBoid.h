#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/PF4/ZLocation.h>
#include <Glacier/PF4/Fwd.h>

#include <vector>

namespace Glacier
{
    enum EBoidState 
    {
        eFollowPath = 0,
        eControlled = 1,
        eSoftObstacle = 2,
        eHardObstacle = 3,
        ePassivePushable = 4,
        eInActive = 5,
        eHero = 6,
    };

    class ZBoid
    {
        // every void* is std::vector<ZBoid*> cref
    public:
        // vtbl
        virtual void FrameUpdate(void*, float);
        virtual void Draw();
        virtual float GetSpeed();
        virtual void Move(void*, float);
        virtual void AvoidBoids(void*, float);
        virtual void HardAvoidBoids(void*, float);
        virtual void MovePosition(const ZVector3& position);
        virtual void TeleportPosition(const ZVector3& position);
        virtual void SetSpeed(float);
        virtual void LoadSave(ISerializerStream&, bool);

        // data
        EBoidState m_eState;
        float m_fSpeed;
        float m_TrackerDis;
        ZLNKWHANDS* m_pActor;
        int m_Mask;
        PF4::ZMetaNode* m_pMetaNode;
        ZVector3 m_kPosition;
        ZVector3 m_Tracker;
        float m_Weight;
        float m_PushRadius;
        int m_iID;
    };
    RE_VERIFY_SIZE(ZBoid, 0x40);

    struct ZTarget 
    {
        ZVector3 m_vPos;
        ZVector3 m_vNormal;
        bool m_bEndPoint;
        float m_fRemaining;
        PF4::ZLocation m_Loc;
    };
    RE_VERIFY_SIZE(ZTarget, 0x34);

    class ZHumanBoid : public ZBoid
    {
    public:
        //vftable (no changes)

        //api
        void SetTarget(Glacier::ZVector3* position, Glacier::Vector3* rotation, float a4, bool force);

        // data
        ZTarget m_Targets[2];
        Glacier::ZVector2 m_Displacement;
        float m_ActualSpeed;
        float m_fMaxSpeed;
        float m_fLookAheadDistance;
        PF4::ZInterface* m_pkPathfinder;
        float m_PathRemaining;
        Glacier::ZVector3 m_SubTarget;
        float m_fSpeedMultiplier;
        float m_fEndSpeed;
        Glacier::ZVector2 m_vEndDir;
        float m_fRemaining;
        float m_fPauseMovementAtDistanc;
        bool m_bFollowSubTarget;
        bool m_bSkipTargets;
        bool m_PathEndsInObstacle;
        int8_t m_AvoidSide;
    }; // Verified size 0xEC (alloc)
    RE_VERIFY_SIZE(ZHumanBoid, 0xEC); // Verified
}