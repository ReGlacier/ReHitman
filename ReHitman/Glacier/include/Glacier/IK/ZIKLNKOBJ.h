#pragma once

#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/Materials/ZTypedef.h>
#include <Glacier/IK/ZActionDispatcher.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/IK/IK.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    class ZIKLNKOBJ : public ZLNKOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZIKLNKOBJ, 0x200027u);

        // static
        STATIC_CLASS_VAR(ZIKLNKOBJ, TScenePropertyID, ContactDepris);

        // types
        struct SIKBoneCollision
        {
            uint8_t cBodyPart;
            bool bFront;
        };
        RE_VERIFY_SIZE(SIKBoneCollision, 0x2);

        // vtbl
        ~ZIKLNKOBJ() override;

        // ZSerializable
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        int AnimCallBackToId(ActiveAnimCB pCallback) override;
        ActiveAnimCB AnimCallBackFromId(int) override;
        void ClassInit() override;
        void ClassInit2() override;
        void ClassFrameUpdate() override;
        void Invisible() override;
        void PushState() override;

        // ZLNKOBJ
        void GetDefaultBones(ZBone *pBones, uint32_t lFirstBoneNum) const override;
        void SetDefaultBones(const ZBone* pBones, const SBoneDefinition* pDef) override;
        void CopyGeometryFrom(ZREF rGeom) override;
        void CalcShadowProjectPlane(float* vTans, const float* mObjectToLight, const float* pObjectToLight) const override;
        void AnimEnd(Animation::ActiveAnimation* pAnim, int lControl) override;

        // ZIKLNKOBJ
        virtual void ActivateRagdoll(bool bActive, bool bEnableTimeout, bool bUseDamping);
        virtual void CalcAnimRemapNames();
        virtual void EnableIK();
        virtual void DisableIK();
        virtual void DisableControls();
        virtual void EnableControls();
        virtual void ForceFacing(const ZVector3& vFacing);
        virtual void GetFocalPoint(ZVector3& vPoint);
        virtual void Reset();
        virtual void GetRootCenter(ZMat3x3& mMat, ZVector3& vPos);
        virtual void GetBoneCollision(ZMat3x3& mMat, ZVector3& vPos);
        virtual void GetOwnerMoveSpeed();
        virtual void GetIKBoneMatPos(int lBoneNr, ZMat3x3& mMat, ZVector3& vPos);
        virtual void GetIKBoneMat(int lBoneNr, ZMat3x3& mMat);
        virtual void GetIKBonePos(int lBoneNr, ZVector3& vPos);
        virtual void SetHeadTarget(const ZVector3& vPos, float fSpeedScaleFactor);
        virtual void ResetHeadTarget();
        virtual uint32_t HeadBoneIndex();
        virtual uint32_t NeckBoneIndex();
        virtual uint32_t PelvisBoneIndex();
        virtual uint32_t LHandBoneIndex();
        virtual uint32_t RHandBoneIndex();
        virtual uint32_t LToeBoneIndex();
        virtual uint32_t RToeBoneIndex();
        virtual int32_t IKCallBackToId(ZIKCALLBACK* pCallBack);
        virtual ZIKCALLBACK* IKCallBackFromId(int);
        virtual void SetFacingTarget(ZREF rGeom, float fTime, ZIKCALLBACK cb);
        virtual void RemoveFacingTarget(float fTime, ZIKCALLBACK callback);
        virtual void DisableFacing();
        virtual void EnableFacing();
        virtual void CreateActionDispatcher();
        virtual void RunLnkAction(ZLnkAction* pAction);
        virtual void RemoveCurrentLnkAction();
        virtual void UpdateCurrentLnkAction();
        virtual void CallBackLnkActionTarget();
        virtual void CallBackLnkActionBone(Animation::ActiveAnimation* pZBoneAnim, float fCallBackStartFrame, float fFrame, ZREF rGeomRef);
        virtual ZLnkAction* CreateLnkAction(uint32_t lActionId);
        virtual uint32_t CurrentLnkActionId() const;
        virtual ZLnkAction* GetCurrentLnkAction() const;
        virtual void DisplayGround(bool bDisplay);
        virtual void EmitFootDustParticle(float fStartTime, int lBoneIndex);
        virtual void MakeFootPrint(bool bMake);
        virtual void UpdateHead();
        virtual void UpdateFacing();
        virtual void UpdateTargets();
        virtual void ResetTargets();
        virtual void RemoveTargets();

        // methods
        ZIKLNKOBJ(const char* psName, ZBaseGeom* pBaseGeom);

        bool CanPlayAnimSegment(Animation::Header* pAnimHeader, float fFrom, float fTo, bool bMirror) const;

        // members
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
    };
    RE_VERIFY_SIZE(ZIKLNKOBJ, 0x190); // Verified PC alloc
}
