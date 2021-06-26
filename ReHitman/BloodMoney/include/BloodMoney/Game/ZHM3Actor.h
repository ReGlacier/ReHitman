#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/ZHumanBoid.h>
#include <BloodMoney/Game/ZHM3HmAs.h>
#include <BloodMoney/Game/ZActor.h>

namespace Hitman::BloodMoney
{
    enum EActorMapRole : int {
        Civilian                 = 0,
        Enemy                    = 1,
        VIP                      = 2,
        Target                   = 3,
        UnknownRole_FilledYellow = 4,
        Police                   = 5,
        Optional                 = 6,
        Default                  = Civilian
    };

    enum EBloodSpurtType
    {
        BloodSpurt = 0x0,
        KnifeSlash = 0x1,
        KnifeSlit = 0x2,
        KnifeImpact = 0x3,
        BluntImpact = 0x4,
    };

    class ZHM3Actor : public ZActor
    {
    public:
        // vftable
        virtual void ActivateAnimSegmentWithCameraBone(Glacier::Animation::Header*, int, float, float, float);
        virtual void PlaySpeechResource(uint32_t, int);
        virtual void PlaySpeechResourceWithFilter(uint32_t, int, float, float, float);
        virtual void SetWeapon(Glacier::ZREF);
        virtual Glacier::ZREF GetWeapon();
        virtual void SetWeaponTemplate(Glacier::ZREF);
        virtual Glacier::ZREF GetWeaponTemplate();
        virtual void FireWeapon(Glacier::ZREF, bool, float, bool);
        virtual void Bite(int, float*, float*, float, float);
        virtual void Punch(bool);
        virtual void StunGunAttack();
        virtual void ThrowKnifeAtRef(Glacier::ZREF, Glacier::ZREF);
        virtual void ActivateRagdollOrAnim();
        virtual void ActivateRagdollNextFrame();
        virtual void InitializeHitpoints(float);
        virtual void SetHitpoints(float);
        virtual float GetHitpoints();
        virtual void TakeDamage(float);
        virtual int GetHealth();
        virtual float GetWeaponStrength();
        virtual bool IsDragEnabled();
        virtual Glacier::ZItemTemplate* GetItemTemplate(Glacier::ZREF);
        virtual Glacier::ZItem* CreateItemFromItemTemplate(Glacier::ZREF);
        virtual void ActivateItem(Glacier::ZREF, int);
        virtual void ConcealItem(Glacier::ZREF, bool);
        virtual void EnablePickupClothes();
        virtual void DisablePickupClothes();
        virtual bool IsWearingClothes();
        virtual void OnHitmanChangedClothes();
        virtual ZHM3HmAs* GetHitmanAs();
        virtual void* GetActorProperties();
        virtual void SetScriptUBAnim(Glacier::Animation::Header*, bool);
        virtual uint32_t GetDeadBodyFlags(Glacier::ZLNKOBJ*);
        virtual void StartDrag(uint32_t);
        virtual void EndDrag();
        virtual bool GetAutoAimTarget();
        virtual void SetAutoAimTarget(bool);
        virtual void StoreUBHoldAnims();
        virtual void MapUBHoldAnimsToIdx();
        virtual void GetItemAssignedUBAnim(Glacier::ZItem*);
        virtual void LoadSave(Glacier::ZPackedInput*);
        virtual void ActivateRigidBody(Glacier::SRigidBodyVelocity const*);

        // api
        void PreparePath();
        void ActivateBloodSpurt(const Glacier::Vector3* pInvDir, const Glacier::Vector3* pCollisionPos, EBloodSpurtType type);

        // data (total size is 0xB60, base size is 0x900)
        EActorMapRole m_mapIconType; //+0x900
        Glacier::ZREF m_HmAs; //+0x904
        int m_field908; //+0x908
        int m_field90C; //+0x90C
        int m_field910; //+0x910
        int m_field914; //+0x914
        int m_field918; //+0x918
        int m_field91C; //+0x91C
        int m_field920; //+0x920
        int m_field924; //+0x924
        int m_field928; //+0x928
        int m_field92C; //+0x92C
        int m_field930; //+0x930
        int m_field934; //+0x934
        int m_field938; //+0x938
        int m_field93C; //+0x93C
        int m_field940; //+0x940
        int m_field944; //+0x944
        int m_field948; //+0x948
        int m_field94C; //+0x94C
        int m_field950; //+0x950
        int m_field954; //+0x954
        int m_field958; //+0x958
        int m_field95C; //+0x95C
        int m_field960; //+0x960
        int m_field964; //+0x964
        int m_field968; //+0x968
        int m_field96C; //+0x96C
        int m_field970; //+0x970
        int m_field974; //+0x974
        int m_field978; //+0x978
        int m_field97C; //+0x97C
        int m_field980; //+0x980
        int m_field984; //+0x984
        int m_field988; //+0x988
        int m_field98C; //+0x98C
        int m_field990; //+0x990
        int m_field994; //+0x994
        int m_field998; //+0x998
        int m_field99C; //+0x99C
        int m_field9A0; //+0x9A0
        int m_field9A4; //+0x9A4
        int m_field9A8; //+0x9A8
        int m_field9AC; //+0x9AC
        int m_field9B0; //+0x9B0
        int m_field9B4; //+0x9B4
        int m_field9B8; //+0x9B8
        int m_field9BC; //+0x9BC
        int m_field9C0; //+0x9C0
        int m_field9C4; //+0x9C4
        int m_field9C8; //+0x9C8
        int m_field9CC; //+0x9CC
        int m_field9D0; //+0x9D0
        int m_field9D4; //+0x9D4
        int m_field9D8; //+0x9D8
        int m_field9DC; //+0x9DC
        int m_field9E0; //+0x9E0
        int m_field9E4; //+0x9E4
        int m_field9E8; //+0x9E8
        int m_field9EC; //+0x9EC
        int m_animHoldToolbox; //+0x9F0
        int m_animHoldBriefcase; //+0x9F4
        int m_field9F8; //+0x9F8
        int m_animHoldCoffee; //+0x9FC
        int m_fieldA00; //+0xA00
        int m_fieldA04; //+0xA04
        int m_fieldA08; //+0xA08
        int m_animHoldBeer; //+0xA0C
        int m_fieldA10; //+0xA10
        int m_fieldA14; //+0xA14
        int m_fieldA18; //+0xA18
        int m_fieldA1C; //+0xA1C
        int m_fieldA20; //+0xA20
        int m_fieldA24; //+0xA24
        int m_fieldA28; //+0xA28
        int m_fieldA2C; //+0xA2C
        int m_fieldA30; //+0xA30
        int m_fieldA34; //+0xA34
        int m_fieldA38; //+0xA38
        int m_fieldA3C; //+0xA3C
        int m_fieldA40; //+0xA40
        int m_fieldA44; //+0xA44
        int m_fieldA48; //+0xA48
        int m_fieldA4C; //+0xA4C
        int m_fieldA50; //+0xA50
        int m_fieldA54; //+0xA54
        int m_fieldA58; //+0xA58
        int m_fieldA5C; //+0xA5C
        int m_fieldA60; //+0xA60
        int m_fieldA64; //+0xA64
        int m_fieldA68; //+0xA68
        int m_fieldA6C; //+0xA6C
        int m_fieldA70; //+0xA70
        int m_fieldA74; //+0xA74
        int m_fieldA78; //+0xA78
        int m_fieldA7C; //+0xA7C
        int m_fieldA80; //+0xA80
        int m_fieldA84; //+0xA84
        int m_fieldA88; //+0xA88
        int m_fieldA8C; //+0xA8C
        int m_fieldA90; //+0xA90
        int m_fieldA94; //+0xA94
        int m_fieldA98; //+0xA98
        int m_fieldA9C; //+0xA9C
        int m_fieldAA0; //+0xAA0
        int m_fieldAA4; //+0xAA4
        int m_fieldAA8; //+0xAA8
        int m_fieldAAC; //+0xAAC
        int m_fieldAB0; //+0xAB0
        int m_fieldAB4; //+0xAB4
        int m_fieldAB8; //+0xAB8
        int m_fieldABC; //+0xABC
        int m_fieldAC0; //+0xAC0
        int m_fieldAC4; //+0xAC4
        Glacier::ZROOM* m_room; // +0xAC8
        int m_fieldACC; //+0xACC
        int m_fieldAD0; //+0xAD0
        int m_fieldAD4; //+0xAD4
        int32_t m_lastLogicUpdateTick; //+0xAD8
        int m_fieldADC; //+0xADC
        int m_fieldAE0; //+0xAE0
        int m_fieldAE4; //+0xAE4
        int m_fieldAE8; //+0xAE8
        int m_fieldAEC; //+0xAEC
        int m_fieldAF0; //+0xAF0
        int m_fieldAF4; //+0xAF4
        int m_fieldAF8; //+0xAF8
        int m_fieldAFC; //+0xAFC
        int m_fieldB00; //+0xB00
        int m_fieldB04; //+0xB04
        int m_fieldB08; //+0xB08
        int m_fieldB0C; //+0xB0C
        int m_fieldB10; //+0xB10
        int m_fieldB14; //+0xB14
        int m_fieldB18; //+0xB18
        int m_fieldB1C; //+0xB1C
        int m_fieldB20; //+0xB20
        int m_fieldB24; //+0xB24
        int m_fieldB28; //+0xB28
        int m_fieldB2C; //+0xB2C
        int m_fieldB30; //+0xB30
        int m_fieldB34; //+0xB34
        int m_fieldB38; //+0xB38
        int m_fieldB3C; //+0xB3C
        int m_fieldB40; //+0xB40
        int m_fieldB44; //+0xB44
        int m_fieldB48; //+0xB48
        int m_fieldB4C; //+0xB4C
        int m_fieldB50; //+0xB50
        int m_fieldB54; //+0xB54
        int m_fieldB58; //+0xB58
        int m_fieldB5C; //+0xB5C
    };
}