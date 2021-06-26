#pragma once

#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/ZMath.h>

namespace Glacier
{
    class ZIKLNKOBJ : public ZLNKOBJ
    {
    public:
        using IKCallBack_t = void(ZIKLNKOBJ::*)();

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
        virtual void HeadBoneIndex();
        virtual void NeckBoneIndex();
        virtual void PelvisBoneIndex();
        virtual void LHandBoneIndex();
        virtual void RHandBoneIndex();
        virtual void LToeBoneIndex();
        virtual void RToeBoneIndex();
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
        int m_fieldF4;
        int m_fieldF8;
        int m_fieldFC;
        int m_field100;
        int m_field104;
        int m_field108;
        int m_field10C;
        int m_field110;
        int m_field114;
        int m_field118;
        int m_field11C;
        int m_field120;
        int m_field124;
        int m_field128;
        int m_field12C;
        int m_field130;
        int m_field134;
        int m_field138;
        int m_field13C;
        int m_field140;
        int m_field144;
        int m_field148;
        int m_field14C;
        int m_field150;
        int m_field154;
        int m_field158;
        int m_field15C;
        int m_field160;
        int m_field164;
        int m_field168;
        int m_field16C;
        int m_field170;
        int m_field174;
        int m_field178;
        int m_field17C;
        int m_field180;
        int m_field184;
        int m_field188;
        int m_field18C;
    };
}