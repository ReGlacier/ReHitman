#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZEntityLocator.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/ZHumanBoid.h>
#include <Glacier/ZAnimVariationHandle.h>
#include <BloodMoney/Game/ZHM3HmAs.h>
#include <BloodMoney/Game/ZActor.h>

namespace Hitman::BloodMoney
{
    enum EActorMapRole : int 
    {
        Civilian                 = 0,
        Enemy                    = 1,
        VIP                      = 2,
        Target                   = 3,
        UnknownRole_FilledYellow = 4,
        Police                   = 5,
        Optional                 = 6,
        Default                  = Civilian
    };

    enum eCharacterType {
        eCharacterType_Civilian = 0,
        eCharacterType_Enemy = 1,
        eCharacterType_VIP = 2,
        eCharacterType_Target = 3,
        eCharacterType_Animal = 4,
        eCharacterType_Policeman = 5,
        eCharacterType_OptionalTarget = 6,
    };

    enum eInterestLevel {
        IL_Neutral = 0,
        IL_Interesting = 1,
        IL_Strange = 2,
        IL_Suspicious = 3,
    };

    enum eActorElevatorState {
        AE_NONE = 0,
        AE_WAITING_FORELEVATOR = 1,
        AE_MOVE_INSIDEELEVATOR = 2,
        AE_INSIDE_ELEVATOR = 3,
        AE_MOVE_OUTSIDEELEVATOR = 4,
    };



    enum EBloodSpurtType
    {
        BloodSpurt = 0x0,
        KnifeSlash = 0x1,
        KnifeSlit = 0x2,
        KnifeImpact = 0x3,
        BluntImpact = 0x4,
    };

    enum EActorBodyPosition : int
    {
        eNormal = 0,
        eSittingDown = 1,
        eSittingDownBar = 2,
        eLyingDown = 3,
        eUnknown = 4,
        eSittingDownBed = 5,
    };

    enum class ESittingDetails : int
    {
        eNone = 0,
        eLeft = 1,
        eCenter = 2,
        eRight = 3,
    };

    enum EActorSpecies  : uint8_t
    {
        eHuman = 0,
        eDog = 1,
        eRat = 2,
        eCorpse = 3,
        eWheelChair = 4,
    };

    class ZHM3HmAs;
    class ZHM3ItemWeapon;

    struct ZDecaying
    {
        float value;
        float m_fTim;
        float m_fMax;
        float m_fMin;
    };
    RE_VERIFY_SIZE(ZDecaying, 0x10);

    struct SBodyInfo 
    {
        Glacier::ZGEOM* pBody;
        int16_t oldpos[3];
        bool bNude;
    };
    RE_VERIFY_SIZE(SBodyInfo, 0xC);

    struct sUseElevatorInfo
    {
        Glacier::ZREF rFrom;
        Glacier::ZREF rTo;
        Glacier::ZREF rElevator;
        eActorElevatorState eAEState;
        Glacier::ZVector3 vLinkEndPos;
        Glacier::TIMETYPE tEndPosReqTime;
        int nCellIdx;
        bool bMoving;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(sUseElevatorInfo, 0x28);

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
        virtual void LoadSave_2(Glacier::ZPackedInput*); // TODO: Make properly
        virtual void ActivateRigidBody(Glacier::SRigidBodyVelocity const*);

        // api
        void PreparePath();
        void ActivateBloodSpurt(const Glacier::Vector3* pInvDir, const Glacier::Vector3* pCollisionPos, EBloodSpurtType type);
        void InitMapIcon(bool);

        // types
        struct UseDoorInfo
        {
            Glacier::ZGEOM* m_CurrentDoorGeom;
            int m_LastDoorPing;
            int m_LastDirectPathQuery;
            float m_HasPausedSecs;
            Glacier::ZVector3 m_DoorNormal;
            
            union
            {
                uint8_t m_LoadSaveMask;
                struct 
                {
                    uint8_t m_bOpenDoorAnimStarted : 1;
                    uint8_t m_bDoorIsDouble : 1;
                    uint8_t m_iMoveThroughDirection : 1;
                    uint8_t m_bMovingThroughDoor : 1;
                    uint8_t m_iIsDoorOpen : 2;
                };
            };
        };
        RE_VERIFY_SIZE(UseDoorInfo, 0x20);
 
        union HitmanInfo
        {
            uint16_t m_LoadSaveMask;
            struct {
                uint16_t m_bCheckIfHitmanIsArmed : 1;
                uint16_t m_bHitManVisiblyArmed : 1;
                uint16_t m_bPreFrameUpdateDie : 1;
                uint16_t m_bHitmanRunningTooLong : 1;
                uint16_t m_bHitmanStealthyTooLong : 1;
                uint16_t m_bHitmanIsNear : 1;
                uint16_t m_bHitmanIntrudedFriskBox : 1;
                uint16_t m_bDressPropertyBloodyDress : 1;
                uint16_t m_bCheckLockpickingHitman : 1;
                uint16_t m_bHitmanVisiblyLockpicking : 1;
                uint16_t m_IsHumanShield : 1;
                uint16_t m_bBulletWentThroughMe : 1;
            };
        };
        RE_VERIFY_SIZE(HitmanInfo, sizeof(uint16_t));

        struct ExtraFlags 
        {
            union
            {
                uint32_t loadSaveMask2;
                struct 
                {
                    uint32_t m_iTimeToNextElevatorUpdate : 8;
                    uint32_t m_bDieWithoutRagdollOrAnim : 1;
                    uint32_t m_bDieWithoutRagdollOneshot : 1;
                    uint32_t m_bForceDisableDragAction : 1;
                    uint32_t m_bUpdateElevatorState : 1;
                    uint32_t m_bDoNotAddActorToVisionSystem : 1;
                    uint32_t m_bDoNotMoveHipOutOfGeometry : 1;
                    uint32_t m_bDisableRagdollForShots : 1;
                    uint32_t m_bDragActionShown : 1;
                    uint32_t m_bClothActionShown : 1;
                    uint32_t m_bInContainer : 1;
                    uint32_t m_bRigidBodyInUse : 1;
                    uint32_t m_iLookAtTime : 3;
                    uint32_t m_bIsHoldingShootableItemL : 1;
                    uint32_t m_bIsHoldingShootableItemR : 1;
                    uint32_t m_bWantBloodOnHit : 1;
                };
            };
        };
        RE_VERIFY_SIZE(ExtraFlags, sizeof(uint32_t));

        struct Flags
        {
            union {
                unsigned int m_LoadSaveMask;
                struct {
                    unsigned int m_bScriptUbAnimActive : 1;
                    unsigned int m_bBloodStainDropped : 1;
                    unsigned int m_bWearingClothes : 1;
                    unsigned int m_bTakeClothesActionActive : 1;
                    unsigned int m_bDrageActionActive : 1;
                    unsigned int m_bIsAutoaimTarget : 1;
                    unsigned int m_bLockUbAnim : 1;
                    unsigned int m_bMayDisable : 1;
                    unsigned int m_bBodyReported : 1;
                    unsigned int m_bUnconscious : 1;
                    unsigned int m_bWeaponCreated : 1;
                    unsigned int m_iRagdollAtNextFrame : 2;
                    unsigned int m_bUseUBRagdoll : 1;
                    unsigned int m_bRelayShootIntoGroundCB : 1;
                    unsigned int m_bUseRigidBodyOnPush : 1;
                    unsigned int m_bNearDeath : 1;
                    unsigned int m_bMayBeBodyBagged : 1;
                    unsigned int m_bIsBodyBag : 1;
                    unsigned int m_bIgnoreHelpRequest : 1;
                    unsigned int m_bHitmanVisible : 1;
                    unsigned int m_bIgnoreDisguise : 1;
                    unsigned int m_bIsInInterruptableState : 1;
                    unsigned int m_bLeaveBloodTrail : 1;
                    unsigned int m_bIsBeingKilled : 1;
                    unsigned int m_bAttachToElevator : 1;
                    unsigned int m_bDetachFromElevator : 1;
                    unsigned int m_bAccidentKilled : 1;
                    unsigned int m_bIsSearching : 1;
                    unsigned int m_bMovingRagdoll : 1;
                    unsigned int m_bCanOpenDoors : 1;
                    unsigned int m_bLastDamageWeaponOwnerIsHitman: 1;
                };
            };
        };
        RE_VERIFY_SIZE(Flags, sizeof(uint32_t));

        // data (total size is 0xB60, base size is 0x900)
        eCharacterType m_eCharacterType; //+0x900
        Glacier::ZREF m_rHitmanAs; //+0x904
        Glacier::ZREF m_rWeapon; //+0x908
        Glacier::ZREF m_rWeaponTemplate; //+0x90C
        bool m_bIsInOutsideLocation; //+0x910
        bool m_bIsInWater;
        RE_ADD_PADDING(2);
        ZHM3ItemWeapon* m_pLastDamageFromWeapon; //+0x914
        Glacier::ZREF m_rDragTarget; //+0x918
        Glacier::ZIKLNKOBJ* m_pDragPerson; //+0x91C
        int8_t m_iDragBone; //+0x920
        int8_t m_iDragParticle;
        RE_ADD_PADDING(2);
        float m_fStartingHitpoints; //+0x924
        float m_fHitpoints; //+0x928
        float m_fTension; //+0x92C
        bool m_bDecayTension; //+0x930
        RE_ADD_PADDING(3);
        float m_fRelaxedVisionRange; //+0x934
        float m_fRelaxedFOV; //+0x938
        float m_fAlertVisionRange; //+0x93C
        float m_fAlertFOV; //+0x940
        ZHM3HmAs* m_pSuspectedSuit; //+0x944
        uint32_t m_lOriginalVariantId; //+0x948
        uint32_t m_lOriginalPrim; //+0x94C
        float m_fHunger; //+0x950
        float m_fHungerTime; //+0x954
        float m_fThirst; //+0x958
        float m_fThirstTime; //+0x95C
        float m_fToilet; //+0x960
        float m_fToiletTime; //+0x964
        float m_fInterest; //+0x968
        float m_fInterestTime; //+0x96C
        ZDecaying m_PushAnnoyance; //+0x970 < APPROVED
        Glacier::ZVector3 m_vLastHitPos; //+0x980 [APPROVED]
        Glacier::ZVector3 m_vLastHitDir; //+0x98C [APPROVED]
        float m_fLastHitForce; //+0x998 [APPROVED]
        Glacier::TIMETYPE m_tBeganSeeingHitman; //+0x99C
        Glacier::TIMETYPE m_tBeganSeeingHitmanRunning; //+0x9A0
        Glacier::TIMETYPE m_tFriskBoxIntrusionStartTime; //+0x9A4
        Glacier::TIMETYPE m_tBeganSeeingHitmanStealthy; //+0x9A8
        UseDoorInfo m_UseDoorInfo; //+0x9AC + internal padding up 4 bytes (+3 actually)
        HitmanInfo m_HitmanInfo; // +0x9CC - use only 2 bytes
        RE_ADD_PADDING(2); // ^ because this [u16->u32]
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        uint32_t m_field9D0; // Divided by uint16x2
        uint32_t m_field9D4;

        uint16_t m_LoadSaveMask; //+0x9D8
        RE_ADD_PADDING(2);
        Glacier::ZREF m_rActorProperties; // +0x9DC
        float m_HumanShieldWeight; // +0x9E0 Approved by in-game code
        Glacier::Animation::Header* m_pScriptUbAnim; // +0x9E4 [APPROVED]
        Glacier::Animation::Header* m_pSpecialPushAnim; // +0x9E8 [APPROVED]
        Glacier::ZAnimVariationHandle m_pPutBodyOnTable;
        RE_ADD_PADDING(2);
        Glacier::Animation::Header* m_pUB_Hold_Toolbox;    // +0x9F0
        Glacier::Animation::Header* m_pUB_Hold_Suitcase;   // +0x9F4
        Glacier::Animation::Header* m_pUB_CarryBox;        // +0x9F8
        Glacier::Animation::Header* m_pUB_HoldCoffeeCup;   // +0x9FC
        Glacier::Animation::Header* m_pUB_Hold_Tray;       // +0xA00
        Glacier::Animation::Header* m_pUB_Hold_Camera;     // +0xA04
        Glacier::Animation::Header* m_pUB_Hold_Glass;      // +0xA08
        Glacier::Animation::Header* m_pUB_Hold_Bottle;     // +0xA0C
        Glacier::Animation::Header* m_pUB_Hold_Crate;      // +0xA10
        Glacier::Animation::Header* m_pUB_Carry_Bucket;    // +0xA14
        Glacier::Animation::Header* m_pUB_Carry_Bible;     // +0xA18
        Glacier::Animation::Header* m_pUB_Hold_Camera_Spec;// +0xA1C
        Glacier::Animation::Header* m_pUB_Hold_Cane;       // +0xA20
        Glacier::Animation::Header* m_pUB_Hold_Mobile;     // +0xA24
        Glacier::Animation::Header* m_pUB_Hold_Miv;        // +0xA28
        Glacier::Animation::Header* m_pUB_Hold_Champagne;  // +0xA2C
        Glacier::Animation::Header* m_unknownAnimA30; // 0xA30
        EActorBodyPosition m_eBodyPosition; //+0xA34
        ESittingDetails m_eSittingDetails; //+0xA38
        Flags m_Flags; // +0xA3C
        float m_fBaseForce;
        Glacier::Animation::ActiveAnimation* m_pDeathAnim;
        EActorSpecies m_eActorSpecies; //+0xA48 < CONFIRMED
        SBodyInfo m_BodyArray[10];
        int8_t m_iMaxBody;
        int8_t m_iStoreBody;
        RE_ADD_PADDING(2);
        Glacier::ZROOM* m_pCurrentRoom; // +0xAC8
        float m_BlinkCycle; //+0xACC
        int8_t m_Mood; //+0xAD0
        RE_ADD_PADDING(3);
        float m_MoodScale; //+0xAD4
        int8_t m_iScriptPriorityUpdate; //+0xAD8
        RE_ADD_PADDING(3);
        Glacier::ZGEOM* m_pChair; //+0xADC
        Glacier::ZQuat m_qUseFurniture;
        Glacier::ZVector3 m_vUseFurniture;
        bool m_bOnScreenLastFrame; //+0xAFC
        RE_ADD_PADDING(3);
        float m_fLeaveScreenTime; //+0xB00
        Glacier::ZParticleEmitter* m_pBreathEmitter; //+0xB04
        Glacier::REFTAB* m_pMouthEmitters; //+0xB08
        float m_fTimeSinceLastOneliner; //+0xB0C
        bool m_bBlinkingDisabled;
        bool m_bUBHoldAnimsEnabled;
        RE_ADD_PADDING(2);
        eInterestLevel m_eLastPushedLevel; //+0xB14
        sUseElevatorInfo m_sActorElevatorInfo; //+0xB18
        Glacier::ZREF m_rContainingElevator; //+0xB40
        Glacier::ZREF m_rOldParent; //+0xB44
        ExtraFlags m_ExtraFlags; // +0xB48
        int m_iNumVariants; // +0xB4C
        bool m_bHidePrimFromCamera; // +0xB50
        RE_ADD_PADDING(3);
        float m_fLastClothActionStatusChange; //+0xB54
        float m_fLastDragActionStatusChange; //+0xB58
        int m_fieldB5C; //+0xB5C
    };
    RE_VERIFY_SIZE(ZHM3Actor, 0xB60);  // Verified
    RE_VERIFY_OFFSET(ZHM3Actor, m_rHitmanAs, 0x904); // Approved by ZHM3Actor__getAvailableSuitObjectID
    RE_VERIFY_OFFSET(ZHM3Actor, m_rWeapon, 0x908); // Approved by ZHM3Actor::DieByForce
    RE_VERIFY_OFFSET(ZHM3Actor, m_rWeaponTemplate, 0x90C); // Approved by ZHM3Actor::CreateAndHideWeapon
    RE_VERIFY_OFFSET(ZHM3Actor, m_bIsInOutsideLocation, 0x910); // Approved by code
    RE_VERIFY_OFFSET(ZHM3Actor, m_pLastDamageFromWeapon, 0x914); // Approved by ZHM3Actor::ShouldActorBleed
    RE_VERIFY_OFFSET(ZHM3Actor, m_pDragPerson, 0x91C); // Approved by Zhm3Actor__Ishitmandraggingactor
    RE_VERIFY_OFFSET(ZHM3Actor, m_fStartingHitpoints, 0x924); // Used by ZHM3Actor
    RE_VERIFY_OFFSET(ZHM3Actor, m_fHitpoints, 0x928); // Approved by ZHM3Actor::OverrideHitpoints
    RE_VERIFY_OFFSET(ZHM3Actor, m_lOriginalVariantId, 0x948); // Approved by  ZHM3Actor::OnChangeNude (store org prim variation)
    RE_VERIFY_OFFSET(ZHM3Actor, m_fHunger, 0x950); // Verified by ZHM3Actor::UpdateNeeds
    RE_VERIFY_OFFSET(ZHM3Actor, m_fHungerTime, 0x954); // Verified by ZHM3Actor::UpdateNeeds
    RE_VERIFY_OFFSET(ZHM3Actor, m_fThirst, 0x958); // Verified by ZHM3Actor::UpdateNeeds
    RE_VERIFY_OFFSET(ZHM3Actor, m_fThirstTime, 0x95C); // Verified by ZHM3Actor::UpdateNeeds
    RE_VERIFY_OFFSET(ZHM3Actor, m_fToilet, 0x960); // Verified by ZHM3Actor::UpdateNeeds
    RE_VERIFY_OFFSET(ZHM3Actor, m_fToiletTime, 0x964); // Verified by ZHM3Actor::UpdateNeeds
    RE_VERIFY_OFFSET(ZHM3Actor, m_fInterest, 0x968); // Verified by ZHM3Actor::UpdateNeeds
    RE_VERIFY_OFFSET(ZHM3Actor, m_fInterestTime, 0x96C); // Verified by ZHM3Actor::UpdateNeeds
    RE_VERIFY_OFFSET(ZHM3Actor, m_PushAnnoyance, 0x970); // Verified by ZHM3Actor::LoadSave 
    RE_VERIFY_OFFSET(ZHM3Actor, m_vLastHitPos, 0x980); // Verified by ZHM3Actor::OnProjectileDamage
    RE_VERIFY_OFFSET(ZHM3Actor, m_vLastHitDir, 0x98C); // Verified by ZHM3Actor::OnProjectileDamage 
    RE_VERIFY_OFFSET(ZHM3Actor, m_fLastHitForce, 0x998); // Verified by ZHM3Actor::Initialize
    RE_VERIFY_OFFSET(ZHM3Actor, m_UseDoorInfo, 0x9AC); // Verified by ZHM3Actor::LoadSave 
    RE_VERIFY_OFFSET(ZHM3Actor, m_HitmanInfo, 0x9CC); // runtime flags
    RE_VERIFY_OFFSET(ZHM3Actor, m_rActorProperties, 0x9DC); // Verified by ZHM3Actor::ClassFrameUpdate 
    RE_VERIFY_OFFSET(ZHM3Actor, m_HumanShieldWeight, 0x9E0); // Verified by ZHM3Actor::sub_63BCC0 code logic
    RE_VERIFY_OFFSET(ZHM3Actor, m_pScriptUbAnim, 0x9E4); // Verified by ZHM3Actor::SetScriptUBAnim
    RE_VERIFY_OFFSET(ZHM3Actor, m_pSpecialPushAnim, 0x9E8); // Verified by ZHM3Actor::OnPushed
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Toolbox, 0x9F0); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Suitcase, 0x9F4); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_CarryBox, 0x9F8); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_HoldCoffeeCup, 0x9FC); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Tray, 0xA00); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Camera, 0xA04); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Glass, 0xA08); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Bottle, 0xA0C); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Crate, 0xA10); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Carry_Bucket, 0xA14); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Carry_Bible, 0xA18); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Camera_Spec, 0xA1C); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Cane, 0xA20); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Mobile, 0xA24); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Miv, 0xA28); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_pUB_Hold_Champagne, 0xA2C); // Verified by ZHM3Actor::StoreUBHoldAnims
    RE_VERIFY_OFFSET(ZHM3Actor, m_eBodyPosition, 0xA34); // Verified by ZHM3Actor::LoadSave
    RE_VERIFY_OFFSET(ZHM3Actor, m_eSittingDetails, 0xA38); // Verified by ZHM3Actor::LoadSave
    RE_VERIFY_OFFSET(ZHM3Actor, m_Flags, 0xA3C); // Verified by ZHM3Actor::LoadSave
    RE_VERIFY_OFFSET(ZHM3Actor, m_eActorSpecies, 0xA48); // Verified by ZHM3Actor::LoadSave
    RE_VERIFY_OFFSET(ZHM3Actor, m_pCurrentRoom, 0xAC8); // Verified by previous R&D & ZHM3Actor::breakMovement
    RE_VERIFY_OFFSET(ZHM3Actor, m_eLastPushedLevel, 0xB14); // Verified by ZHM3Actor::LoadSave
    RE_VERIFY_OFFSET(ZHM3Actor, m_sActorElevatorInfo, 0xB18); // Verified by ZHM3Actor::LoadSave
    RE_VERIFY_OFFSET(ZHM3Actor, m_ExtraFlags, 0xB48); // Verified by ZHM3Actor::LoadSave, ZHM3Actor::EnableDrag, ZHM3Actor::ActivateRagdoll
    RE_VERIFY_OFFSET(ZHM3Actor, m_LoadSaveMask, 0x9D8); // Verified by ZHM3Actor::LoadSave
    RE_VERIFY_OFFSET(ZHM3Actor, m_pBreathEmitter, 0xB04); // Verified by ZHM3Actor::AllocBreathEmitter
    RE_VERIFY_OFFSET(ZHM3Actor, m_pMouthEmitters, 0xB08); // Verified by ZHM3Actor::AddMouthEmitter
    RE_VERIFY_OFFSET(ZHM3Actor, m_rContainingElevator, 0xB40); // Verified by ZHM3Actor::GetElevatorDeltaY & ZHM3Actor::IsInElevator
    RE_VERIFY_OFFSET(ZHM3Actor, m_iNumVariants, 0xB4C); // Verified by ZHM3Actor::OnChangeNormalCloth, ZHM3Actor::OnChangeNude
    RE_VERIFY_OFFSET(ZHM3Actor, m_bHidePrimFromCamera, 0xB50); // Verified by ZHM3Actor::Initialize (set to false)
    RE_VERIFY_OFFSET(ZHM3Actor, m_fLastClothActionStatusChange, 0xB54); // Verified by ZHM3Actor::HandleClothDragActions
    RE_VERIFY_OFFSET(ZHM3Actor, m_fLastDragActionStatusChange, 0xB58); // Verified by ZHM3Actor::HandleClothDragActions

    /*
    Unknown fields:
        uint32_t m_field9D0; // Divided by uint16x2
        uint32_t m_field9D4; // idk
        m_fieldB58 - Not found at all

    from ZHM3Actor::Initialize
        v10 = pThis->field_9D0 & 0xFE;
        LOBYTE(pThis->field_9C8) &= 0xFCu;
        v11 = pThis->field_B49;
        LOBYTE(pThis->field_9D0) = v10;
        pThis->field_9D4 = pThis->field_9D4 & 0xFFFFFFFC | 2;
#endif
    */
}