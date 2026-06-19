#pragma once

#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZActionDispatcher.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZIKLNKOBJ : public ZLNKOBJ
    {
    public:
        // types
        using IKCallBack_t = void(ZIKLNKOBJ::*)();

        struct SIKBoneCollision
        {
            uint8_t cBodyPart;
            bool bFront;
        };
        RE_VERIFY_SIZE(SIKBoneCollision, 0x2);

        //vftable
        virtual void ActivateRagdoll(bool, bool, bool);
        virtual void CalcAnimRemapNames();
        virtual void EnableIK();
        virtual void DisableIK();
        virtual void DisableControls();
        virtual void EnableControls();
        virtual void ForceFacing(const ZVector3*);
        virtual void GetFocalPoint(ZVector3*);
        virtual void Reset();
        virtual void GetRootCenter(ZMat3x3*, ZVector3*);
        virtual void GetBoneCollision(ZMat3x3*, ZVector3*);
        virtual void GetOwnerMoveSpeed();
        virtual void GetIKBoneMatPos(int, ZMat3x3*, ZVector3*);
        virtual void GetIKBoneMat(int, ZMat3x3*);
        virtual void GetIKBonePos(int,ZVector3*);
        virtual void SetHeadTarget(const ZVector3*,float);
        virtual void ResetHeadTarget();
        virtual unsigned int HeadBoneIndex();
        virtual unsigned int NeckBoneIndex();
        virtual unsigned int PelvisBoneIndex();
        virtual unsigned int LHandBoneIndex();
        virtual unsigned int RHandBoneIndex();
        virtual unsigned int LToeBoneIndex();
        virtual unsigned int RToeBoneIndex();
        virtual void IKCallBackToId(IKCallBack_t callback);
        virtual IKCallBack_t IKCallBackFromId(int);
        virtual void SetFacingTarget(unsigned int, float, IKCallBack_t callback);
        virtual void RemoveFacingTarget(float, IKCallBack_t callback);
        virtual void DisableFacing();
        virtual void EnableFacing();
        virtual void CreateActionDispatcher();
        virtual void RunLnkAction(ZLnkAction*);
        virtual void RemoveCurrentLnkAction();
        virtual void UpdateCurrentLnkAction();
        virtual void CallBackLnkActionTarget();
        virtual void CallBackLnkActionBone(Animation::ActiveAnimation*, float, float, unsigned int);
        virtual ZLnkAction* CreateLnkAction(unsigned int);
        virtual unsigned int CurrentLnkActionId();
        virtual ZLnkAction* GetCurrentLnkAction();
        virtual void DisplayGround(bool);
        virtual void EmitFootDustParticle(float, int);
        virtual void MakeFootPrint(bool);
        virtual void UpdateHead();
        virtual void UpdateFacing();
        virtual void UpdateTargets();
        virtual void ResetTargets();
        virtual void RemoveTargets();

        //data (total size is 0x190, base size is 0xF4)
        uint32_t m_Active;
        bool m_bRunning;
        bool m_bFacingDisabled;
        bool m_pad13D[2];
        int m_lPushedActive;
        ZGEOM* m_pFootDustTemplate;
        ZGEOM* m_pFootPrints;
        ZTARGET m_BodyFacingTarget;
        ZVector3 m_vHeadTarget;
        float m_fHeadTargetPrc;
        float m_fHeadHeight;
        ZActionDispatcher* m_pActionDispatcher;
        ZActionDispatcher m_StdActionDisplatcher;
        uint32_t m_pad;
    }; // Total size is 0x190 (reg code at 0x0073CD70)
    RE_VERIFY_SIZE(ZIKLNKOBJ, 0x190);
}