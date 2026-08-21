#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZCTRLIKLNKOBJ.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/Audio/ZSDOwner.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Items/ITEMSTATE.h>
#include <Glacier/Animation/ZBlendBone.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>

namespace Glacier
{
    struct SIKBoneCollision;

    enum EWHandsActionType : int {
        ZActionPickupItem_7 = 7,
        ZActionPickupItem_8 = 8,
        ZActionPickupItem_3 = 3,
        ZActionPickupItem_4 = 4,
        Type5 = 5,
        Type6 = 6,
        Type9 = 9,
        Type10 = 10,
        Type11 = 11,
        Type12 = 12,
        Type13 = 13,
        Type14 = 14,
        Type15 = 15,
        Unknown = 0xFF // default
    };

    class ZLNKWHANDS : public ZCTRLIKLNKOBJ
    {
    public:
        // types
        enum EFootSide : int
        {
            eRIGHT = 0,
            LEFT = 1,
            CENTER = 2
        };

        //vftable
        virtual void AddNearItem(Glacier::ZREF);
        virtual void RemoveNearItem(Glacier::ZREF);
        virtual void OnBoidPushing(ZLNKWHANDS*);
        virtual void OnBoidPushed(ZLNKWHANDS*);
        virtual void CheckRHandFireTarget();
        virtual void GetRHandFireTarget();
        virtual void SetAimInPosition(bool);
        virtual void AddAction(EWHandsActionType, Glacier::ZREF);
        virtual const char* GetAnimNameFromCollision(SIKBoneCollision*, bool, ZItemTemplateWeapon*);
        virtual void ReloadItem(ZItem*);
        virtual void LetItemFall(ZItem*);
        virtual void ShowReloadAnim(ZItemWeapon*);
        virtual void ShowChamberAnim(ZItemWeapon*);
        virtual void ControlPickup(ZItem*);
        virtual void ControlUseOneExtra(Glacier::ZREF);
        virtual void ControlUseOneAlone(Glacier::ZREF);
        virtual void ControlUseTwo(Glacier::ZREF, Glacier::ZREF);
        virtual void ControlDrop(ZItem*);
        virtual ZLnkAction* GetPickupAction(int&, int&, int&, int, int, int, bool); //unused
        virtual void ModifyPickupAction(int&, int&, int&, ZItemTemplate*); //unused
        virtual bool OnlyVerticalAim();
        virtual void GetAimAnim(Animation::Header **, unsigned int &);
        virtual void GetPoseEffects(float *,short *);
        virtual void SetHandTarget(int handIndex, Glacier::ZVector3* position, float unk);
        virtual void DontAnimateAttachers(); //Do nothing
        virtual void RemoveLHandIKTarget(float a1, IKCallBack_t callback, int a3, int a4, int a5); //a3, a4, a5 will be ignores, pass zeroes
        virtual void RemoveRHandIKTarget(float a1, IKCallBack_t callback, int a3, int a4, int a5); //a3, a4, a5 will be ignores, pass zeroes
        virtual void UpdateAimTarget();
        virtual void SetAimTarget(Glacier::ZREF, float const*, float, IKCallBack_t callBack, int);
        virtual void SetAimTarget(float const*, float, IKCallBack_t callBack);
        virtual void RemoveAimTarget(float, IKCallBack_t callBack);
        virtual void FreezeAimTarget();
        virtual void DisableAim();
        virtual void EnableAim();
        virtual void* GetAimTarget();
        virtual void GetAimTarget(Glacier::ZREF*);
        virtual void GetAimTarget(Glacier::ZVector3*);
        virtual float GetDrawWeaponSpeed();
        virtual bool IsAiming();
        virtual float SetAimAnimPrc(float, float, float); //do nothing
        virtual void FireShotNotify();
        virtual void SetHitAnimation(char const* animName);
        virtual void SetHitAnimHandle(ZAnimVariationHandle&);
        virtual void UseItem(ZItem *, int);
        virtual void DropItem(ZItem *);
        virtual void PickupItem(ZItem *);
        virtual void Reload(ZItem *);
        virtual void Chamber(ZItem *);
        virtual void PerformDrop(ZItem *,bool,bool);
        virtual ZIKHAND* GetRHand();
        virtual ZIKHAND* GetLHand();
        virtual ZItem* GetLHandItem();
        virtual ZItem* GetRHandItem();
        virtual ZItemWeapon* GetLHandWeapon();
        virtual ZItemWeapon* GetRHandWeapon();
        virtual ZItemTemplate* GetRHandItemType();
        virtual ZItemTemplate* GetLHandItemType();
        virtual void SetRHandItemState(ITEMSTATE);
        virtual void SetLHandItemState(ITEMSTATE);
        virtual void AttachItemToHand(ZIKHAND*, Glacier::ZREF);
        virtual void AttachRHandItem(Glacier::ZREF);
        virtual void AttachLHandItem(Glacier::ZREF);
        virtual void* GetNearItems();  //
        virtual void* GetNearItems2(); //Dup, in PC only
        virtual void SetDialog(bool);
        virtual bool GetDialog();
        virtual void DialogListen(int variant); //1 or 0
        virtual void DialogTalk(int variant); //1 or 0
        virtual void SlipRHandItem();
        virtual void SlipLHandItem();
        virtual void ForceLHandItem(ZItem*, bool);
        virtual void ForceRHandItem(ZItem*, bool);
        virtual void DestroyRHandItem();
        virtual void DestroyLHandItem();
        virtual bool EmptyHands();
        virtual int GetAmmoFor(ZItemWeapon*); //Do nothing
        virtual void SetAmmoFor(ZItemWeapon*, int); //Do nothing
        virtual void SetLHandIKTarget(float const*, float const*, float, IKCallBack_t callBack, int a6, int a7, int a8); //a6, a7, a8 will be ignored, pass zeroes
        virtual void SetRHandIKTarget(float const*, float const*, float, IKCallBack_t callBack, int a6, int a7, int a8); //a6, a7, a8 will be ignored, pass zeroes
        virtual void UpdateItemActions();
        virtual void CallBackItemAction(); //Do Nothing
        virtual void PlaySpeech(unsigned int speedSoundIndex, int unk, bool unused);
        virtual void PlayFootStep(EFootSide);
        virtual bool IsFirstPersonCamera();
        virtual float GetCombatStrength();

        //data (total size is 0x3D0, base size is 0x1A0)
        float m_fLastUpdatedPosition;
        bool m_bInMotion;
        bool m_pad1E5[3];
        Animation::ZBlendBone m_AttachBones[2]; // I'm not sure, but looks right
        unsigned int m_rSpeechSound;
        ZSDOwner m_SoundDef;
        ZSDOwner m_MaterialDef;
        float m_fActorPitch;
        bool m_bFootStepSoundsEnabled;
        bool m_bFootStepEventsEnabled;
        bool m_padding[2];
        int m_lFootStepEvent;
        int m_ContactMaterialDescID; //  ZTypedef<int,enum EHardTypedef_TMaterialDescID>
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
        uint8_t m_pad[3];
    };
    RE_VERIFY_SIZE(ZLNKWHANDS, 0x3D0); // Verified
    RE_VERIFY_OFFSET(ZLNKWHANDS, m_SoundDef, 0x20C);
    RE_VERIFY_OFFSET(ZLNKWHANDS, m_MaterialDef, 0x21C);
    RE_VERIFY_OFFSET(ZLNKWHANDS, m_AimTarget, 0x268);
    RE_VERIFY_OFFSET(ZLNKWHANDS, m_LHand, 0x2C8);
    RE_VERIFY_OFFSET(ZLNKWHANDS, m_RHand, 0x348);
}
