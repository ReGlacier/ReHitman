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
        virtual bool ActivateRagdoll(bool bActive, bool bEnableTimeout, bool bUseDamping);
        virtual void CalcAnimRemapNames();
        virtual void EnableIK();
        virtual void DisableIK();
        virtual void DisableControls();
        virtual void EnableControls();
        virtual void ForceFacing(const ZVector3& vFacing);
        virtual void GetFocalPoint(ZVector3& vPoint);
        virtual void Reset();
        virtual void GetRootCenter(ZMat3x3& mMat, ZVector3& vPos) const;
        virtual SIKBoneCollision GetBoneCollision(const ZVector3& vPoint, const ZVector3& vDirection);
        virtual float GetOwnerMoveSpeed() const;
        virtual bool GetIKBoneMatPos(int lBoneNr, ZMat3x3& mMat, ZVector3& vPos) const;
        virtual bool GetIKBoneMat(int lBoneNr, ZMat3x3& mMat) const;
        virtual bool GetIKBonePos(int lBoneNr, ZVector3& vPos) const;
        virtual void SetHeadTarget(const ZVector3& vPos, float fSpeedScaleFactor);
        virtual void ResetHeadTarget();
        virtual int32_t HeadBoneIndex() const;
        virtual int32_t NeckBoneIndex() const;
        virtual int32_t PelvisBoneIndex() const;
        virtual int32_t LHandBoneIndex() const;
        virtual int32_t RHandBoneIndex() const;
        virtual int32_t LToeBoneIndex() const;
        virtual int32_t RToeBoneIndex() const;
        virtual int32_t IKCallBackToId(ZIKCALLBACK* pCallBack);
        virtual ZIKCALLBACK* IKCallBackFromId(int);
        virtual void SetFacingTarget(ZREF rGeom, float fTime, ZIKCALLBACK cb);
        virtual void RemoveFacingTarget(float fTime, ZIKCALLBACK callback);
        virtual void DisableFacing();
        virtual void EnableFacing();
        virtual void CreateActionDispatcher();
        virtual bool RunLnkAction(ZLnkAction* pAction);
        virtual void RemoveCurrentLnkAction();
        virtual void UpdateCurrentLnkAction();
        virtual void CallBackLnkActionTarget();
        virtual bool CallBackLnkActionBone(Animation::ActiveAnimation* pZBoneAnim, float fCallBackStartFrame, float fFrame, ZREF rGeomRef);
        virtual ZLnkAction* CreateLnkAction(uint32_t lActionId);
        virtual uint32_t CurrentLnkActionId() const;
        virtual ZLnkAction* GetCurrentLnkAction() const;
        virtual bool DisplayGround(bool bDisplay);
        virtual void EmitFootDustParticle(float fStartTime, int lBoneIndex);
        virtual void MakeFootPrint(bool bMake);
        virtual void UpdateHead();
        virtual void UpdateFacing();
        virtual void UpdateTargets();
        virtual void ResetTargets();
        virtual void RemoveTargets();

        // methods
        ZIKLNKOBJ(const char* psName, ZBaseGeom* pBaseGeom);

        bool CanPlayAnimSegment(Animation::Header* pAnimHeader, float fFrom, float fTo, bool bMirror);
        bool CanPlayAnimSegment(Animation::Header* pAnimHeader, float fFrom, float fTo, const float* pRootMat, const float* pRootPos, bool bMirror, float fHeight, float fDepth);

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
