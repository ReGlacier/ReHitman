#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZCTRLIKLNKOBJ.h>
#include <Glacier/IK/ZIKHAND.h>

namespace Glacier
{
    struct SIKBoneCollision;

    enum EFootSide : int {
        Type1 = 1
    };

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
        int m_field1A0;
        bool m_bInMotion; //0x1A4
        char field_1A5;
        char field_1A6;
        char field_1A7;
        int field_1A8;
        int field_1AC;
        int field_1B0;
        int field_1B4;
        int field_1B8;
        int field_1BC;
        int field_1C0;
        int field_1C4;
        int field_1C8;
        int field_1CC;
        int field_1D0;
        int field_1D4;
        int field_1D8;
        int field_1DC;
        int field_1E0;
        int field_1E4;
        int field_1E8;
        int field_1EC;
        int field_1F0;
        int field_1F4;
        int field_1F8;
        int field_1FC;
        int field_200;
        int field_204;
        int m_rSpeechSound;
        int field_20C;
        int field_210;
        int field_214;
        int field_218;
        int field_21C;
        int field_220;
        int field_224;
        int field_228;
        int field_22C;
        int field_230;
        int m_lFootStepEvent;
        int m_ContactMaterialDescID;
        int m_rMaterial;
        int field_240;
        float m_fRecoil;
        int m_hitAnimationName;
        uint16_t m_animPullGun_Right;
        uint16_t field_24E;
        uint16_t m_animPickPut;
        uint16_t field_252;
        uint16_t m_animReloadMagnum;
        uint16_t field_256;
        uint16_t m_animReloadHardballer;
        uint16_t field_25A;
        uint16_t m_animReloadMP5;
        uint16_t field_25E;
        uint16_t m_animReloadRuger;
        uint16_t field_262;
        uint16_t m_animReloadGun_OneHand;
        uint16_t field_266;
        int m_pTarget;
        int field_26C;
        int field_270;
        int field_274;
        int field_278;
        int field_27C;
        int field_280;
        int field_284;
        int field_288;
        int field_28C;
        int field_290;
        int field_294;
        int field_298;
        int field_29C;
        int field_2A0;
        int field_2A4;
        int field_2A8;
        int field_2AC;
        int field_2B0;
        int field_2B4;
        int field_2B8;
        int field_2BC;
        int field_2C0;
        int field_2C4;
        ZIKHAND m_leftHand;
        ZIKHAND m_rightHand;
        REFTAB *p_nearItems;
        int m_Mask1;
    };
}