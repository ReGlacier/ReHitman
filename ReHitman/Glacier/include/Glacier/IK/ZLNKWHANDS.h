#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/IK/ZCTRLIKLNKOBJ.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/Audio/ZSDOwner.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Items/ITEMSTATE.h>
#include <Glacier/Animation/ZBlendBone.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>
#include <Glacier/Materials/ZTypedef.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/Materials/ZTypedef.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    struct SIKBoneCollision;

    enum HANDSPICKUP
    {
        BP_NONE = 0,
        BP_LEFT = 1,
        BP_RIGHT = 2,
        BP_LEFTRIGHT = 3,
        BP_LEFT_HIDE = 4,
        BP_RIGHT_HIDE = 5,
        BP_RIGHT_SWAP = 6,
    };

    enum HANDPICKUP
    {
        HP_NONE = 0,
        HP_DROP = 1,
        HP_HIDE = 2
    };

    class ZLNKWHANDS : public ZCTRLIKLNKOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZLNKWHANDS, 0x200073u);

        // types
        enum EFootSide : int
        {
            eRIGHT = 0,
            LEFT = 1,
            CENTER = 2
        };

        // static
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMSGID, s_msgCutSequenceEnd);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMessageResolver, m_msgCalcMats);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMessageResolver, m_msgDropItem);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMessageResolver, m_msgUseItem);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMessageResolver, m_msgCanPickupItem);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMessageResolver, m_msgPickupItem);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMessageResolver, m_msgReloadWeapon);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMessageResolver, m_msgEnterItemRange);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMessageResolver, m_msgLeaveItemRange);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMessageResolver, m_msgSoundStartet);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZMessageResolver, m_msgSoundStopped);
        STATIC_CLASS_VAR(ZLNKWHANDS, float, m_fDmgMultDefault);
        STATIC_CLASS_VAR(ZLNKWHANDS, float, m_fDmgMultLeftArm);
        STATIC_CLASS_VAR(ZLNKWHANDS, float, m_fDmgMultRightArm);
        STATIC_CLASS_VAR(ZLNKWHANDS, float, m_fDmgMultLeftLeg);
        STATIC_CLASS_VAR(ZLNKWHANDS, float, m_fDmgMultRightLeg);
        STATIC_CLASS_VAR(ZLNKWHANDS, float, m_fDmgMultTorso);
        STATIC_CLASS_VAR(ZLNKWHANDS, float, m_fDmgMultHead);
        STATIC_CLASS_VAR(ZLNKWHANDS, float, m_fDmgMultFace);
        STATIC_CLASS_VAR(ZLNKWHANDS, float, m_fDmgMultLeftHand);
        STATIC_CLASS_VAR(ZLNKWHANDS, float, m_fDmgMultRightHand);
        STATIC_CLASS_VAR(ZLNKWHANDS, bool, bInitializedStaticAnimsZLNKWHANDS);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pPullGunRight);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pPutItemLeft);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pPutItemRight);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pPutGunLeft);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pPutGunRight);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pDropItemLeft);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pDropItemRight);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pAimRHandGuns);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberGunLeft);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberGunRight);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberRifle);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberShotgun);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberPumpgun);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberRPG);
        STATIC_CLASS_VAR(ZLNKWHANDS, ZAnimVariationHandle, m_pChamberGrenade);
        STATIC_CLASS_VAR(ZLNKWHANDS, TAudioPropertyID, m_MaterialProperty_SoundContact);
        STATIC_CLASS_VAR(ZLNKWHANDS, TAudioPropertyID, m_MaterialProperty_SoundMaterial);
        STATIC_CLASS_VAR_ARRAY(ZLNKWHANDS, ZAnimVariationHandle, m_pTalk, 2);
        STATIC_CLASS_VAR_ARRAY(ZLNKWHANDS, ZAnimVariationHandle, m_pListen, 2);

        // vtbl
        ~ZLNKWHANDS() override;

        // ZSerializable
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void ClassInit2() override;
        int32_t ClassCommand(ZMSGID Msg, void* pData) override;
        void OnMoving() override;
        void OnMoved() override;

        // ZIKLNKOBJ
        void CalcAnimRemapNames() override;
        void Reset() override;
        ZLnkAction* CreateLnkAction(uint32_t lActionId) override;
        void UpdateTargets() override;
        void ResetTargets() override;
        void RemoveTargets() override;

        // ZLNKWHANDS
        virtual void AddNearItem(ZREF rItem);
        virtual void RemoveNearItem(ZREF rItem);
        virtual void OnBoidPushing(ZLNKWHANDS* pBoid);
        virtual void OnBoidPushed(ZLNKWHANDS* pBoid);
        virtual bool CheckRHandFireTarget();
        virtual ZREF GetRHandFireTarget();
        virtual void SetAimInPosition(bool bValue);
        virtual void AddAction(int lType, ZREF rTarget);
        virtual const char* GetAnimNameFromCollision(SIKBoneCollision* pCol, bool bBigForce, ZItemTemplateWeapon* pITW);
        virtual void ReloadItem(ZItem* pItem);
        virtual void LetItemFall(ZItem* pItem);
        virtual bool ShowReloadAnim(ZItemWeapon* pWeapon) const;
        virtual bool ShowChamberAnim(ZItemWeapon* pWeapon) const;
        virtual HANDSPICKUP ControlPickup(ZItem* pItem);
        virtual void ControlUseOneExtra(ZREF rRef);
        virtual void ControlUseOneAlone(ZREF rRef);
        virtual void ControlUseTwo(ZREF rFirst, ZREF rSecond);
        virtual void ControlDrop(ZItem* pItem);
        virtual void ControlPushButton(ZGEOM* pGeom);
        virtual int32_t GetPickupAction(HANDPICKUP& lResLHand, HANDPICKUP& lResRHand, HANDSPICKUP& lResBoth, ITEMHANDS lLHand, ITEMHANDS lRHand, ITEMHANDS lPickup, bool bPickupFit) const;
        virtual void ModifyPickupAction(ITEMHANDS& lLHand, ITEMHANDS& lRHand, ITEMHANDS& lPickup, ZItemTemplate* pItemTemplate) const;
        virtual Animation::Header* GetAimAnim(Animation::Header** pRecoil, uint32_t& lExtraFlags) const;
        virtual void GetPoseEffects(float* poseWeights, int16_t* poseIdToIndex);
        virtual void SetHandTarget(int handIndex, Glacier::ZVector3& vWorldPos, float fWeight);
        virtual int DontAnimateAttachers();
        virtual void RemoveLHandIKTarget(float fTime, ZIKCALLBACK cb);
        virtual void RemoveRHandIKTarget(float fTime, ZIKCALLBACK cb);
        virtual bool UpdateAimTarget();
        virtual void SetAimTarget(ZREF rGeom, const ZVector3& vP0, float fTime, ZIKCALLBACK cb, int lBoneId);
        virtual void SetAimTarget(const ZVector3& vP0, float fTime, ZIKCALLBACK cb);
        virtual void RemoveAimTarget(float fTime, ZIKCALLBACK cb);
        virtual void FreezeAimTarget();
        virtual void DisableAim();
        virtual void EnableAim();
        virtual bool GetAimTarget(ZVector3& vTarget);
        virtual bool GetAimTarget(ZREF& rRef) const;
        virtual ZREF GetAimTarget() const;
        virtual float GetDrawWeaponSpeed();
        virtual bool IsAiming() const;
        virtual void SetAimAnimPrc(float, float, float);
        virtual void FireShotNotify();
        virtual void SetHitAnimation(char const* szAnimName);
        virtual void SetHitAnimHandle(ZAnimVariationHandle& hAnim);
        virtual void UseItem(ZItem* pItem, int lType);
        virtual void DropItem(ZItem* pItem);
        virtual void PickupItem(ZItem* pItem);
        virtual void Reload(ZItem* pItem);
        virtual void Chamber(ZItem* pItem);
        virtual void PerformDrop(ZItem* pItem, bool bAddRigidBody, bool bSilent);
        virtual ZIKHAND* GetRHand();
        virtual ZIKHAND* GetLHand();
        virtual ZItem* GetLHandItem() const;
        virtual ZItem* GetRHandItem() const;
        virtual ZItemWeapon* GetLHandWeapon() const;
        virtual ZItemWeapon* GetRHandWeapon() const;
        virtual ITEMHANDS GetRHandItemType() const;
        virtual ITEMHANDS GetLHandItemType() const;
        virtual void SetRHandItemState(ITEMSTATE lState);
        virtual void SetLHandItemState(ITEMSTATE lState);
        virtual ZREF AttachItemToHand(ZIKHAND* pHand, ZREF rItem);
        virtual ZREF AttachRHandItem(ZREF rItem);
        virtual ZREF AttachLHandItem(ZREF rItem);
        virtual REFTAB* GetNearItems() const;
        virtual REFTAB* GetNearItems();
        virtual void SetDialog(bool bEnable);
        virtual bool GetDialog() const;
        virtual void DialogListen(int lType);
        virtual void DialogTalk(int lType);
        virtual void SlipRHandItem();
        virtual void SlipLHandItem();
        virtual void ForceLHandItem(ZItem* pZItem, bool bBruteForce);
        virtual void ForceRHandItem(ZItem* pZItem, bool bBruteForce);
        virtual void DestroyRHandItem();
        virtual void DestroyLHandItem();
        virtual void EmptyHands();
        virtual int32_t GetAmmoFor(ZItemWeapon* pWeapon);
        virtual void SetAmmoFor(ZItemWeapon* pWeapon, int lAmount);
        virtual void SetLHandIKTarget(const ZMat3x3& m0, const ZVector3& v0, float fTime, ZIKCALLBACK cb);
        virtual void SetRHandIKTarget(const ZMat3x3& m0, const ZVector3& v0, float fTime, ZIKCALLBACK cb);
        virtual void UpdateItemActions();
        virtual void CallBackItemAction();
        virtual SREF PlaySpeech(uint32_t lDefinitionIndex, int lSoundEvent, bool bUseFilter);
        virtual SREF PlayFootStep(EFootSide PlayFootStep);
        virtual bool IsFirstPersonCamera() const;
        virtual float GetCombatStrength();

        // methods
        ZLNKWHANDS(const char* psName, ZBaseGeom* pBaseGeom);
        SREF PlaySpeechResource(uint32_t lSoundIndex, int lSoundEvent);

        // members
        float m_fLastUpdatedPosition;
        bool m_bInMotion;
        RE_ADD_PADDING(3);
        Animation::ZBlendBone m_AttachBones[2]; // I'm not sure, but looks right
        unsigned int m_rSpeechSound;
        ZSDOwner m_SoundDef;
        ZSDOwner m_MaterialDef;
        float m_fActorPitch;
        bool m_bFootStepSoundsEnabled;
        bool m_bFootStepEventsEnabled;
        bool m_padding[2];
        int m_lFootStepEvent;
        TMaterialDescID m_ContactMaterialDescID;
        int m_rMaterial;
        int m_rCutSequence;
        float m_fRecoil;
        const char* m_pszHitAnim;
        ZAnimVariationHandle m_HitAnimHandle;
        ZAnimVariationHandle m_pPullItemRight;
        ZAnimVariationHandle m_pPickupItemLeft;
        ZAnimVariationHandle m_pPickupItemRight;
        ZAnimVariationHandle m_pSwapItems;
        ZAnimVariationHandle m_pReloadRevolverLeft;
        ZAnimVariationHandle m_pReloadRevolverRight;
        ZAnimVariationHandle m_pReloadPistolRight;
        ZAnimVariationHandle m_pReloadRifle;
        ZAnimVariationHandle m_pReloadSubMachineGunRight;
        ZAnimVariationHandle m_pReloadShotgun;
        ZAnimVariationHandle m_pReloadPumpgun;
        ZAnimVariationHandle m_pReloadRPG;
        ZAnimVariationHandle m_pReloadGunOneHand;
        ZTARGET m_AimTarget;
        ZIKHAND m_LHand;
        ZIKHAND m_RHand;
        REFTAB* m_pNearItems;
        union {
            uint8_t m_Mask1;
            struct
            {
                bool m_bDialog : 1;
                bool m_bAimDisabled : 1;
                bool m_bAimInPosition : 1;
            };
        };
        RE_ADD_PADDING(3);;
    };
    RE_VERIFY_SIZE(ZLNKWHANDS, 0x3D0); // Verified
    RE_VERIFY_OFFSET(ZLNKWHANDS, m_SoundDef, 0x20C);
    RE_VERIFY_OFFSET(ZLNKWHANDS, m_MaterialDef, 0x21C);
    RE_VERIFY_OFFSET(ZLNKWHANDS, m_AimTarget, 0x268);
    RE_VERIFY_OFFSET(ZLNKWHANDS, m_LHand, 0x2C8);
    RE_VERIFY_OFFSET(ZLNKWHANDS, m_RHand, 0x348);
}
