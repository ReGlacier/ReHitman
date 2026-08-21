#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZPlayer.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZSTL/ZArray.h>
#include <Glacier/Physics/SBodyGMR.h>

#include <BloodMoney/Game/LevelControls/ESecurityZone.h>
#include <BloodMoney/Game/Actions/EHM3Action.h>

namespace Hitman::BloodMoney
{
    // fwds
    class ZHitman3;
    class ZHM3Actor;
    class ZHM3ItemContainer;
    class ZHM3ItemTemplateWeapon;
    class ZHM3ItemWeapon;
    class ZHM3ItemBomb;
    class ZHM3HmAs;
    class ZTie;
    struct ZTimeMultEffectControl;
    struct ZHM3Camera;
    struct ZHitman3AutoAim;
    struct ZHM3MovementGuideToMatPos;
    struct ZHM3MovementGuideList;

    // enums
    enum eHideStuffState {
        eHideNone = 0,
        eHideFront = 1,
        eHideBehind = 2,
        eHideSide = 3,
    };

    struct SCombineItems
    {
        EHM3ItemType m_eItemType1;
        EHM3ItemType m_eItemType2;
        Glacier::ZGROUP* m_pCombineGroup;
    };
    RE_VERIFY_SIZE(SCombineItems, 0xC);

    struct SNearEnemy
    {
        ZHM3Actor* m_pActor;
        float      m_fDistance;
        float      m_fAngle;
        bool       m_bVisibleOnScreen;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(SNearEnemy, 0x10);

    struct SDualWeapons
    {
        ZHM3ItemTemplateWeapon* m_pTemplate;
        ZHM3ItemWeapon* m_pRight;
        ZHM3ItemWeapon* m_pLeft;
        int32_t m_nRightAmmo;
        int32_t m_nLeftAmmo;
    };
    RE_VERIFY_SIZE(SDualWeapons, 0x14);

    struct SRailInfo
    {
        Glacier::ZGEOM* pRailGeom;
        Glacier::ZVector3 vP1;
        Glacier::ZVector3 vP2;
        Glacier::ZVector3 vP3;
        Glacier::ZVector3 vColiPt;
    };
    RE_VERIFY_SIZE(SRailInfo, 0x34);

    // aliases
    using ZStackArrayLocalizedActionNames = Glacier::ZStackArray<61, Glacier::zstring>;

    using ZStackArrayCombineItemList = Glacier::ZStackArray<10, SCombineItems>; // 124
    RE_VERIFY_SIZE(ZStackArrayCombineItemList, 124);

    using ZStackArrayNearEnemiesList = Glacier::ZStackArrayInsert<5, SNearEnemy, float>; // 104
    RE_VERIFY_SIZE(ZStackArrayNearEnemiesList, 104);

    using ZStackArrayDualWeaponsTemplateList = Glacier::ZStackArray<10, SDualWeapons>; // 204
    RE_VERIFY_SIZE(ZStackArrayDualWeaponsTemplateList, 204);

    using ZStackArrayPlantedBombs = Glacier::ZStackArray<3, ZHM3ItemBomb*>;
    RE_VERIFY_SIZE(ZStackArrayPlantedBombs, 0x10);

	struct ZHM3Inventory
	{
        // members
		Glacier::CInventory* m_pInventoryEvent;
		Glacier::REFTAB* m_pInventory;
		Glacier::ZGROUP* m_pWeaponsGroup;
	};
	RE_VERIFY_SIZE(ZHM3Inventory, 0xC);

	class ZHM3InputControl
	{
    public:
        // types
        struct MoveFlags
        {
            uint16_t m_bLeanLeft : 1;
            uint16_t m_bLeanRight : 1;
            uint16_t m_bLeanLeftShoot : 1;
            uint16_t m_bLeanRightShoot : 1;
            uint16_t m_bRunToggle : 1;
            uint16_t m_bMovementIncrease : 1;
            uint16_t m_bMovementDecrease : 1;
            uint16_t m_bCrouchSneak : 1;
            uint16_t m_bRun : 1;
        };

        struct WeaponFlags
        {
            uint8_t m_bFire : 1;
            uint8_t m_bFire2 : 1;
            uint8_t m_bReload : 1;
        };

        struct CameraFlags
        {
            uint8_t m_bToggleCamera;
            uint8_t m_bCameraZoomIn;
            uint8_t m_bCameraZoomOut;
            uint8_t m_bCameraZoom;
        };

        // vtbl
        virtual void LoadObject(Glacier::IInputSerializerStream& stream);
        virtual void SaveObject(Glacier::IOutputSerializerStream& stream);

        // members
        MoveFlags m_Movements;
        WeaponFlags m_Weapons;
        RE_ADD_PADDING(1);
        CameraFlags m_Camera;
        bool m_bDropBody;
        bool m_bCancelExit;
        bool m_bDropItem;
        bool m_bThrowItem;
        bool m_bPickupItem;
        bool m_bUseAction;
        bool m_bActive;
        RE_ADD_PADDING(1);
        float m_fCutSeqTime;
        float m_fThrowTime;
	};
	RE_VERIFY_SIZE(ZHM3InputControl, 0x1C);

	class ZHM3MovementControl
    {
    public:
        // vtbl
        virtual void LoadObject(Glacier::IInputSerializerStream& stream);
        virtual void SaveObject(Glacier::IOutputSerializerStream& stream);

        // members
        bool m_bDisableMovement;
        RE_ADD_PADDING(3);
        Glacier::ZVector2 m_vTurn;
        Glacier::ZVector2 m_vTurnMultiplier;
        Glacier::ZVector3 m_vMoveSpeed;
        float m_fMovementSpeedPrc;
        struct ZHitman3ControlBase* m_pCurrentControl;
        struct ZHitman3ControlMouseStrafe* m_pControlMouseStrafe;
        struct ZHitman3ControlMouse* m_pControlMouse;
        struct ZHitman3ControlConsoleStrafe* m_pControlConsoleStrafe;
        struct ZHitman3ControlConsole* m_pControlConsole;
        bool m_bStrafeModeActive;
        bool m_bNoTurnStrafeMode;
        bool m_bForceFaceCamera;
        RE_ADD_PADDING(1);
        float m_fTurnToWalkAngle;
        struct CEaseInOut* m_pTurnEaseInOut;
    };
    RE_VERIFY_SIZE(ZHM3MovementControl, 0x48);

    class ZHM3SegmentedLineColi
    {
    public:
        Glacier::TIMETYPE m_fColiUpdateTime;
        float m_fDynamicTestDistance;
        bool m_bWorkColiStatic;
        bool m_bWorkColiDynamic;
        bool m_bAlternateDynamicStatic;
        RE_ADD_PADDING(1);
        int m_lTryingToHitCount;
        Glacier::ZVector3 m_vStart;
        Glacier::ZVector3 m_vEnd;
        float m_fColiLength;
        Glacier::ZVector3 m_vColiPos;
        Glacier::ZVector3 m_vColiNormal;
        Glacier::ZBaseGeom* m_pColiGeom;
        Glacier::ZBaseGeom* m_pWorkColiGeom;
    };
    RE_VERIFY_SIZE(ZHM3SegmentedLineColi, 0x4C);

    class ZHM3WeaponsControl
    {
    public:
        // vtbl
        virtual void LoadObject(Glacier::IInputSerializerStream& stream);
        virtual void SaveObject(Glacier::IOutputSerializerStream& stream);

        // methods
        ZHitman3* Hitman() { return m_pHitman; }

        // types
        struct FireFlags {
            uint16_t m_bFireWeapon : 1;
            uint16_t m_bFireWeapon2 : 1;
            uint16_t m_bLastFireWeapon : 1;
            uint16_t m_bLastFireWeapon2 : 1;
            uint16_t m_bFireWeaponRelease : 1;
            uint16_t m_bFireWeapon2Release : 1;
            uint16_t m_bFireWeaponTap : 1;
            uint16_t m_bFireWeapon2Tap : 1;
            uint16_t m_bFireLock : 1;
            uint16_t m_bFireTargetCallBackWait : 1;
            uint16_t m_bReloadWeapon : 1;
            uint16_t m_bReloadCausedByEmptyWeapon : 1;
            uint16_t m_bHumanShieldAimModeActive : 1;
            uint16_t m_bEnableWeaponUse : 1;
        };

        struct DualActionFlags
        {
            unsigned char m_bDualActionEnabled : 1;
            unsigned char m_bDualActionAutoEnabled : 1;
        };

        struct WeaponInfo
        {
            Glacier::ZLNKOBJ* m_pGround;
            Glacier::Animation::Header* m_pAnimReloadWeapon;
            Glacier::Animation::Header* m_pAnimChamberWeapon;
            Glacier::Animation::Header* m_pAnimReloadOneHand;
        };

        // members
        ZHitman3* m_pHitman;
        Glacier::TIMETYPE m_ttLastAttackTime;
        Glacier::ZMSGID m_msgGetInventoryListPtr;
        FireFlags m_Flags;
        WeaponInfo m_RHandWeaponInfo;
        WeaponInfo m_LHandWeaponInfo;
        int32_t m_iNumberOfAmmoClips;
        int32_t m_iProjectilesInMagazine;
        int32_t m_iPrevProjectilesInMagazine;
        int32_t m_iNrAmmoProjectiles;
        Glacier::ZGEOM* m_pAimLastTarget;
        float m_fTargetDistance;
        ZHM3SegmentedLineColi* m_pSegAimColi;
        DualActionFlags m_DualFlags;
    };
    RE_VERIFY_SIZE(ZHM3WeaponsControl, 0x50);

    class ZHM3ActionControl
    {
    public:
        // vtbl
        virtual void LoadObject(Glacier::IInputSerializerStream& stream);
        virtual void SaveObject(Glacier::IOutputSerializerStream& stream);

        // members
        ZHitman3* m_pHitman;
        Glacier::ZREF m_rHitman;
        EHM3Action m_eCurrentAction;
        Glacier::ZGEOM* m_pCurrentActionObject;
        Glacier::ZAction* m_pCurrentZAction;
        Glacier::ZItem* m_pRightHandItem;
        Glacier::ZItem* m_pLeftHandItem;
        Glacier::ZItem* m_pCombineItem1;
        Glacier::ZItem* m_pCombineItem2;
        Glacier::ZGEOM* m_pCombineMaster;
        Glacier::ZItemContainer* m_pItemContainer;
        Glacier::ZItem* m_pPlaceRetrieveItem;
        ZStackArrayLocalizedActionNames m_LocalizedActionNames;
        Glacier::ZREF m_rNearestFrontEnemy;
        ZHM3Actor* m_pNearestFrontEnemy;
        float m_fNearestEnemyAngle;
        float m_fNearestEnemyFaceHitmanAngle;
    };
    RE_VERIFY_SIZE(ZHM3ActionControl, 0x324);

    struct SAccessoryGeom
    {
        uint32_t rGeom;
        uint32_t iBoneId;
    };
    RE_VERIFY_SIZE(SAccessoryGeom, 0x8);

    /*
		Inheritance tree:
		ZHitman3:
			ZPlayer -> ZLNKWHANDS -> ZCTRLIKLNKOBJ -> ZIKLNKOBJ -> ZLNKOBJ -> ZSTDOBJ -> ZGEOM -> RTP::cBase -> ZSerializable -> ZSerializableBase
			ZHM3InputControl
			ZHM3MovementControl
			ZHM3WeaponsControl
			ZHM3Inventory
			ZHM3ActionControl
	*/
	class ZHitman3 : public Glacier::ZPlayer, public ZHM3InputControl, public ZHM3MovementControl, public ZHM3WeaponsControl, public ZHM3Inventory, public ZHM3ActionControl
	{
    public:
        // types
        struct Actions
        {
            bool m_bChangingClothes;
            bool m_bChangingClothesAllowed;
            bool m_bSneaking;
            bool m_bStrangulating;
            bool m_bInjecting;
            bool m_bPunching;
            bool m_bPushing;
            bool m_bClimbingHatch;
            float m_fInjectingTime;
            bool m_bLockPicking;
            char pad_0D[3];
        };
        RE_VERIFY_SIZE(Actions, 0x10);

        struct Enabled
        {
            bool m_bNightVisionEnabled;
            bool m_bBinocularsEnabled;
            bool m_bFacingEnabled;
        };
        RE_VERIFY_SIZE(Enabled, 0x3);

        struct SMovementLast
        {
            bool m_bIsInMotion;
            bool m_bIsWalking;
            bool m_bIsRunning;
            bool m_bIsCrouching;
            bool m_bIsSneaking;
            bool m_bIsCrouchSneaking;
            bool m_bIsOnLadder;
            bool m_bIsOnDrainPipe;
            bool m_bIsOnGuide;
        };
        RE_VERIFY_SIZE(SMovementLast, 0x9);

        struct SForceFlags
        {
            bool m_bForceCrouch;
            bool m_bDragPosForced;
        };
        RE_VERIFY_SIZE(SForceFlags, 0x2);

        // vtbl (valid & reconstructed)
        virtual bool GetHideInThisView() const;
        virtual void Die(bool bUnknown1, bool bUnknown2);
        virtual void ActivateAnimSegmentWithCameraBone(Glacier::Animation::Header* pHeader, int iBoneIdx, float fParam1, float fParam2, float fParam3);
        virtual void GetAnims();
        virtual float GetItemAssignedUBAnim(Glacier::ZItem* pItem, Glacier::Animation::Header** ppOutHeader, unsigned int* pOutIdx, float* pOutFloat);
        virtual void StoreUBHoldAnims();
        virtual void MapUBHoldAnimsToIdx();
        virtual void DisableControls(bool cMode);
        virtual Glacier::ZGEOM* GetNearestActionObject();
        virtual bool IsInCutsequence();
        virtual void DetachItemFromRightHand(Glacier::ZItem* pItem);
        virtual void DetachItemFromLeftHand(Glacier::ZItem* pItem);
        virtual void DetachItemFromHitman(Glacier::ZItem* pItem);
        virtual void AttachItemToRightHand(Glacier::ZItem* pItem);
        virtual void AttachItemToLeftHand(Glacier::ZItem* pItem);
        virtual void AttachItemToHitman(Glacier::ZItem* pItem);

        // Test only
        ZTimeMultEffectControl* m_pTimeMultEffectControl;
        ZHM3Camera* m_pCameraControl;
        ZHitman3AutoAim* m_pAutoAim;
        bool m_bAutoAimEnabled;
        char _pad1[3];
        struct ZHM3MovementGuideToMatPos* m_pMovementGuideToMatPos;
        struct ZHM3MovementGuideList* m_pMovementGuideList;
        unsigned int m_CurrentStatusExtended;
        unsigned int m_LastStatusExtended;
        unsigned int m_LastStatus;
        bool m_bFakeScopeMode;
        char _pad2[3];
        unsigned int m_rThis;
        bool bSwapBack;
        char _pad3[3];
        unsigned int m_iObjectVisibleFlags[2];
        unsigned int m_iLastObjectVisibleFlags[2];
        bool m_bIsInWater;
        char _pad4[3];
        Glacier::ZVector3 m_fPelvisUpDown;
        Glacier::Animation::Header* m_pAnimReloadShotgun1;
        Glacier::Animation::Header* m_pAnimReloadFranchi;
        Glacier::Animation::Header* m_pAnimHolsterGunSuitcase;
        Glacier::Animation::Header* m_pAnimPullItemRightSuitcase;
        Glacier::Animation::Header* m_pPullItemRight;
        Glacier::Animation::Header* m_pAnimFirstPersonAimRifle;
        Glacier::Animation::Header* m_pAnimFirstPersonAimGun;
        Glacier::Animation::Header* m_pAnimFirstPersonAim2Guns;
        Glacier::Animation::Header* m_pAnimAimAnimHumanShield;
        Glacier::Animation::Header* m_pAnimAimAnimHumanShieldRecoil;
        Glacier::Animation::Header* m_pAnimUpperBodyCutThroat;
        Glacier::Animation::Header* m_pAnimGarrot_01;
        Glacier::Animation::Header* m_pAnimGarrot_02;
        Glacier::Animation::Header* m_pAnimGarrot_03;
        Glacier::Animation::Header* m_pAnimGarrot_Costume;
        Glacier::Animation::Header* m_pAnimStrangleQuick;
        Glacier::Animation::Header* m_pAnimStrangleQuickHigh;
        Glacier::Animation::Header* m_pAnimGarrot_Chair;
        Glacier::Animation::Header* m_pAnimDieStomac;
        Glacier::Animation::Header* m_pAnimStabAxe;
        Glacier::Animation::Header* m_pAnimPunchDeep;
        Glacier::Animation::Header* m_pAnimPunchTighten;
        Glacier::Animation::Header* m_pAnimDisarm_Gun;
        Glacier::Animation::Header* m_pAnimDisarm_DualGun;
        Glacier::Animation::Header* m_pAnimDisarm_Riffle;
        Glacier::Animation::Header* m_pAnimKnifeTighten[2];
        Glacier::Animation::Header* m_pAnimKnifeHold[2];
        Glacier::Animation::Header* m_pAnimKnifeHit01[2];
        Glacier::Animation::Header* m_pAnimKnifeHit02[2];
        Glacier::Animation::Header* m_pAnimKnifeThrowTighten;
        Glacier::Animation::Header* m_pAnimKnifeMiss;
        Glacier::Animation::Header* m_pAnimKnifeKill01;
        Glacier::Animation::Header* m_pAnimKnifeKill02;
        Glacier::Animation::Header* m_pAnimKnifeKill03;
        Glacier::Animation::Header* m_pAnimKnifeKillCostume;
        Glacier::Animation::Header* m_pAnimKnifeKillSitting;
        Glacier::Animation::Header* m_pAnimHedgeCutterTighten[2];
        Glacier::Animation::Header* m_pAnimHedgeCutterHit[2];
        Glacier::Animation::Header* m_pAnimHedgeCutterHold[2];
        Glacier::Animation::Header* m_pAnimHedgeCutterMiss;
        Glacier::Animation::Header* m_pAnimHedgeCutterKill;
        Glacier::Animation::Header* m_pAnimHedgeCutterTightenBehind;
        Glacier::Animation::Header* m_pAnimPickAxeTighten[2];
        Glacier::Animation::Header* m_pAnimPickAxeHit[2];
        Glacier::Animation::Header* m_pAnimPickAxeHold[2];
        Glacier::Animation::Header* m_pAnimPickAxeKill;
        Glacier::Animation::Header* m_pAnimFireExtinguisherTighten[2];
        Glacier::Animation::Header* m_pAnimFireExtinguisherHit[2];
        Glacier::Animation::Header* m_pAnimFireExtinguisherHold[2];
        Glacier::Animation::Header* m_pAnimFireExtinguisherKill;
        Glacier::Animation::Header* m_pAnimHammerTighten[2];
        Glacier::Animation::Header* m_pAnimHammerHit[2];
        Glacier::Animation::Header* m_pAnimHammerHold[2];
        Glacier::Animation::Header* m_pAnimHammerKill;
        Glacier::Animation::Header* m_pAnimSyringeHold[2];
        Glacier::Animation::Header* m_pAnimBaseballBatTighten[2];
        Glacier::Animation::Header* m_pAnimBaseballBatHit[2];
        Glacier::Animation::Header* m_pAnimUBBaseballBatHold[2];
        Glacier::Animation::Header* m_pAnimBaseballBatKill;
        Glacier::Animation::Header* m_pAnimPitchforkTighten[2];
        Glacier::Animation::Header* m_pAnimPitchforkHit[2];
        Glacier::Animation::Header* m_pAnimPitchforkHold[2];
        Glacier::Animation::Header* m_pAnimPitchforkKill;
        Glacier::Animation::Header* m_pAnimScrewdriverTighten[2];
        Glacier::Animation::Header* m_pAnimScrewdriverTightenBehind;
        Glacier::Animation::Header* m_pAnimScrewdriverHit[2];
        Glacier::Animation::Header* m_pAnimScrewdriverHold[2];
        Glacier::Animation::Header* m_pAnimScrewdriverKill;
        Glacier::Animation::Header* m_pAnimStungunTighten[2];
        Glacier::Animation::Header* m_pAnimStungunHit[2];
        Glacier::Animation::Header* m_pAnimHoldStunGun[2];
        Glacier::Animation::Header* m_pAnimStunGunTightenBehind;
        Glacier::Animation::Header* m_pAnimStungunKill;
        Glacier::Animation::Header* m_pAnimShovelTighten[2];
        Glacier::Animation::Header* m_pAnimShovelHit[2];
        Glacier::Animation::Header* m_pAnimShovelHold[2];
        Glacier::Animation::Header* m_pAnimShovelKill;
        Glacier::Animation::Header* m_pAnimCaneSwordTighten;
        Glacier::Animation::Header* m_pAnimCaneSwordHit;
        Glacier::Animation::Header* m_pAnimCaneSwordHold;
        Glacier::Animation::Header* m_pAnimCaneSwordKill;
        Glacier::Animation::Header* m_pAnimDragBodyLeftHand;
        Glacier::Animation::Header* m_pAnimStartDragBody;
        Glacier::Animation::Header* m_pAnimPickLockStart;
        Glacier::Animation::Header* m_pAnimPickLockStartSuitcase;
        Glacier::Animation::Header* m_pAnimPickLockLoop;
        Glacier::Animation::Header* m_pAnimPickLockEnd;
        Glacier::Animation::Header* m_pAnimPickLockEndSuitcase;
        Glacier::Animation::Header* m_pAnimHMUnlockDoor;
        Glacier::Animation::Header* m_pAnimHoldSuitcase;
        Glacier::Animation::Header* m_pAnimHoldSuitcaseReload;
        Glacier::Animation::Header* m_pAnimHoldSuitcaseRun;
        Glacier::Animation::Header* m_pAnimHoldSuitcaseDrainPipe;
        Glacier::Animation::Header* m_pAnimHoldSuitcaseCrouch;
        Glacier::Animation::Header* m_pAnimHoldSuitcaseSneak;
        Glacier::Animation::Header* m_pAnimStunFront;
        Glacier::Animation::Header* m_pAnimDisarmTakeGun;
        Glacier::Animation::Header* m_pAnimDisarmTakeGunFast;
        Glacier::Animation::Header* m_pAnimChangeClothes;
        Glacier::Animation::Header* m_pAnimUpperBodyMissAxe;
        Glacier::Animation::Header* m_pAnimUpperBodyUseClub;
        Glacier::Animation::Header* m_pAnimUpperBodyHitClub;
        Glacier::Animation::Header* m_pAnimDieSequence;
        Glacier::Animation::Header* m_pAnimUpperBodyHoldGun;
        Glacier::Animation::Header* m_pAnimUpperBodyHold2Guns;
        Glacier::Animation::Header* m_pAnimUpperBodyHoldRifle;
        Glacier::Animation::Header* m_pAnimUpperBodyHoldRifleLefthand;
        Glacier::Animation::Header* m_pAnimUpperBodyHoldMP5;
        Glacier::Animation::Header* m_pAnimUpperBodyHoldSMGHorizontalHandle;
        Glacier::Animation::Header* m_pAnimUpperBodyHoldCase;
        Glacier::Animation::Header* m_pAnimUpperBodyUseGarrot;
        Glacier::Animation::Header* m_pAnimUpperBodyTightenGarrot01;
        Glacier::Animation::Header* m_pAnimUpperBodyTightenGarrotCrouch;
        Glacier::Animation::Header* m_pAnimUpperBodyMissGarrot;
        Glacier::Animation::Header* m_pAnimUpperBodyHoldFN2000;
        Glacier::Animation::Header* m_pAnim1StHoldCustomSMGNoUpgrade;
        Glacier::Animation::Header* m_pAnim1StRecoilCustomSMGNoUpgrade;
        Glacier::Animation::Header* m_pAnim1StHoldCustomAssaultUpgrade;
        Glacier::Animation::Header* m_pAnim1StRecoilCustomAssaultUpgrade;
        Glacier::Animation::Header* m_pAnimUBStrechArmWhileAiming;
        Glacier::Animation::Header* m_pAnim1StCHBSingleRecoilAuto;
        Glacier::Animation::Header* m_pAnim1StCHBDualRecoilAuto;
        Glacier::Animation::Header* m_pAnimDisposeBody1;
        Glacier::Animation::Header* m_pAnimDisposeBody2;
        Glacier::Animation::Header* m_pAnimDisposeBodyTable;
        Glacier::Animation::Header* m_pAnimHideBodyInBed;
        Glacier::Animation::Header* m_pAnimPickupDeadBody;
        Glacier::Animation::Header* m_pAnimDisposeBody;
        Glacier::Animation::Header* m_pAnimHoldGunBehind;
        Glacier::Animation::Header* m_pAnimHoldGunFront;
        Glacier::Animation::Header* m_pAnimHoldGunSide;
        Glacier::Animation::Header* m_pAnimHoldKnifeBehind;
        Glacier::Animation::Header* m_pAnimHoldKnifeFront;
        Glacier::Animation::Header* m_pAnimHoldKnifeSide;
        Glacier::Animation::Header* m_pAnimTransKnifeBehindToSide;
        Glacier::Animation::Header* m_pAnimTransKnifeFrontToSide;
        Glacier::Animation::Header* m_pAnimUpperBodyCarryBody;
        Glacier::Animation::Header* m_pAnimThrow;
        Glacier::Animation::Header* m_pAnimThrowLow;
        Glacier::Animation::Header* m_pAnimThrowTighten;
        Glacier::Animation::Header* m_pAnimThrowTightenBigRightHand;
        Glacier::Animation::Header* m_pAnimThrowSuitcaseTighten;
        Glacier::Animation::Header* m_pAnimThrowSuitcase;
        Glacier::Animation::Header* m_pAnimPush;
        Glacier::Animation::Header* m_pAnimSuitcaseRetrieve;
        Glacier::Animation::Header* m_pAnimSuitcasePack;
        Glacier::Animation::Header* m_pAnimHumanShieldGrab;
        Glacier::Animation::Header* m_pAnimHumanShieldRelease;
        Glacier::Animation::Header* m_pAnimHumanShieldLeftArmPose;
        Glacier::Animation::Header* m_pAnimPoisonActor;
        Glacier::Animation::Header* m_pAnimPoisonActorCostume;
        Glacier::Animation::Header* m_pAnimPoisonActorQuick;
        Glacier::Animation::Header* m_pAnimPoisonActorChair;
        Glacier::Animation::Header* m_pAnimPoisonActorBed;
        Glacier::Animation::Header* m_pAnimPoisonItemGround;
        Glacier::Animation::Header* m_pAnimPoisonItemTable;
        Glacier::Animation::Header* m_pAnimPoisonItem100cm;
        Glacier::Animation::Header* m_pAnimPoisonItem120cm;
        Glacier::Animation::Header* m_pAnimUseKeycard;
        Glacier::Animation::Header* m_pAnimClimbHatch;
        Glacier::Animation::Header* m_pAnimStartStrangleInElev;
        Glacier::Animation::Header* m_pAnimUseSwitch;
        Glacier::Animation::Header* m_pAnimUseFirealarm;
        Glacier::Animation::Header* m_pAnimJumpBalcony;
        Glacier::Animation::Header* m_pSedateCIAAgent;
        Glacier::Animation::Header* m_pPutDownSuitcase;
        Glacier::Animation::Header* m_pPickupSuitcaseFront;
        Glacier::Animation::Header* m_pPickupSuitcaseLeft;
        Glacier::Animation::Header* m_pPickupSuitcaseRight;
        Glacier::Animation::Header* m_pReviveCIAAgent;
        Glacier::Animation::Header* m_pDropWeigth;
        Glacier::Animation::Header* m_pBreakUtilBox;
        Glacier::Animation::Header* m_pPourFluidOnDevice;
        Glacier::Animation::Header* m_pPoisonBottle;
        Glacier::Animation::Header* m_pOpenGasAnim;
        Glacier::Animation::Header* m_pLaptopAnim;
        Glacier::Animation::Header* m_pHMPhoning;
        Glacier::Animation::Header* m_pPickUpBox;
        Glacier::Animation::Header* m_pPickUpBox75;
        Glacier::Animation::Header* m_pPutDownBox;
        Glacier::Animation::Header* m_pPickUpBurgers;
        Glacier::Animation::Header* m_pPutDownBurgers;
        Glacier::Animation::Header* m_pBoxFromElevator;
        Glacier::Animation::Header* m_pBoxToElevator;
        Glacier::Animation::Header* m_pAnimTurnRight;
        Glacier::Animation::Header* m_pAnimTurnLeft;
        Glacier::Animation::Header* m_pStandDoNothing;
        Glacier::Animation::Header* m_pTriggerBombAnim;
        Glacier::Animation::Header* m_pDualWeaponAimAnim;
        Glacier::Animation::Header* m_pDualWeaponRecoilAnim;
        Glacier::Animation::Header* m_pPackRifle;
        Glacier::Animation::Header* m_pUnPackRifle;
        Glacier::Animation::Header* m_pHoldCrate;
        Glacier::Animation::Header* m_pHoldCake;
        Glacier::Animation::Header* m_pHoldToolBox;
        Glacier::Animation::Header* m_pHoldTray;
        Glacier::Animation::Header* m_pHoldBottle;
        Glacier::Animation::Header* m_pHoldGlass;
        Glacier::Animation::Header* m_pHoldBible;
        Glacier::Animation::Header* m_pHoldMobile;
        Glacier::Animation::Header* m_pHoldBinoculars;
        Glacier::Animation::Header* m_pHoldSausage;
        Glacier::ZArray<Glacier::Animation::Header*> m_UBAnimArray;
        struct ZFootPrints* m_pFootPrints;
        struct ZBloodTrails* m_pBloodTrails;
        Glacier::ZMSGID m_msgProjectileHit;
        Glacier::ZMSGID m_msgBite;
        Glacier::ZMSGID m_msgDoorLocked;
        Glacier::ZMSGID m_msgOperateObject;
        Glacier::ZMSGID m_msgReloadWeapon;
        Glacier::ZMSGID m_msgCanOperateObject;
        Glacier::ZMSGID m_msgAction_GetActionList;
        Glacier::ZMSGID m_msgRequestKeyholePos;
        Glacier::ZMSGID m_msgPickLockReqeust;
        Glacier::ZMSGID m_msgGetCombineWithItemTemplate;
        Glacier::ZMSGID m_msgCombineItems;
        Glacier::ZMSGID m_msgRequestDeltaY;
        Glacier::ZMSGID m_msgForceDie;
        Glacier::ZMSGID m_msgActivateObject;
        Glacier::ZMSGID m_msgGrab;
        Glacier::ZMSGID m_msgRelease;
        float m_fMapClosedTime;
        Glacier::TIMETYPE m_ttDamageGraceTime;
        Glacier::TIMETYPE m_ttDeathCutsequenceTime;
        Glacier::TIMETYPE m_ttCrouchSneakStartTime;
        Glacier::TIMETYPE m_ttLastHitAnimTime;
        Glacier::TIMETYPE m_ttLastUpdateActionControl;
        float m_fTurnSpeed;
        float m_fHitpoints;
        float m_fHitpointsMax;
        float m_fAdrenalineHitpoints;
        float m_fAdrenalineTimer;
        bool m_bPostInit;
        uint8_t m_pad469[3];
        uint32_t m_iKnifeCount;
        uint32_t m_iStrangleCount;
        float m_fFallingDamageMultiplier;
        float m_fLockPickTimeMultiplier;
        int m_iHitmanHitpoints;
        bool m_bAllowHeadShotsAgainstHitman;
        uint8_t m_pad47D[3];
        struct ZHM3HmAs* m_pCurrentSuit;
        struct ZHM3HmAs* m_pHMAsHM;
        struct ZTie* m_pTie;
        SAccessoryGeom m_aAccessoryGeoms[3];
        int8_t m_iNumAccessoryGeoms;
        int8_t m_iOnFirstPersonModeForceThirdPersonModeCount;
        int8_t m_DontAnimateAttachers;
        int8_t m_pad4AB;
        ZGEOM* m_pGrab;
        Glacier::SBodyGMR m_GrabData;
        int32_t m_BoneGrabIndex;
        ZHM3Actor* m_pEscort;
        bool m_bFireTargetCallBackWait;
        bool m_bUseItemUBAnim;
        bool m_bOneHandReloading;
        bool m_bReloadIng;
        Actions m_Actions;
        Enabled m_Enabled;
        SMovementLast m_MovementLast;
        SForceFlags m_ForceFlags;
        bool m_bLastCutsequence;
        bool m_bVisibleToCameraUpdate;
        bool m_bFirstPersonModeBones;
        bool m_bCameraToggleAllowed;
        bool m_pad502[2];
        ZHM3Actor* m_pHumanShieldTarget;
        ZHM3Actor* m_pCurrentVictim;
        ZStackArrayCombineItemList m_CombineItems;
        ZStackArrayNearEnemiesList m_NearEnemiesList;
        ZHM3ItemWeapon* m_pDualWeaponsPickupItem;
        ZStackArrayDualWeaponsTemplateList m_DualWeaponsTemplateList;
        bool m_bSuitcasePackUnpackActive;
        bool m_bSuitcasePackUnpackInitialPosReached;
        bool m_pad6C2[2];
        ZHM3ItemWeapon* m_pSuitcasePackUnpackItemSniper;
        ZHM3ItemContainer* m_pSuitcasePackUnpackItemSuitcase;
        bool m_bPackSuitcase;
        bool m_pad6CD[3];
        uint32_t m_CurrentStatusBuffer[15];
        uint32_t m_CurrentStatusExtendedBuffer[15];
        Glacier::ZGROUP* m_pSuitcaseUnpackSniper;
        Glacier::ZGROUP* m_pSuitcasePackSniper;
        Glacier::ZGROUP* m_pPackFlightCase;
        Glacier::ZGROUP* m_pUnPackFlightCase;
        Glacier::TIMETYPE m_ttStartBreathTime;
        float m_fBreathTime;
        bool m_bBreathIn;
        bool m_pad761[3];
        struct ZParticleEmitter* m_pBreathEmitter;
        bool m_bIsInOutsideLocation;
        bool m_pad769[3];
        ZStackArrayPlantedBombs m_PlantedBombs;
        Glacier::ZGEOM* m_pLastPlaceBombGeom;
        Glacier::ZItem* m_pAttachedRHandItem;
        Glacier::ZROOM* m_pCurrentRoom;
        ESecurityZone m_eRoomZone;
        ESecurityZone m_eCustomZone;
        bool m_bZoneClear;
        bool m_bRequestZoneValidate;
        bool m_pad792[2];
        Glacier::ZGEOM* m_pRequiredHeadwear;
        Glacier::ZGEOM* m_pRequiredAttachedItem;
        Glacier::ZGEOM* m_pZoneControl;
        Glacier::ZROOM* m_pBackdropRoom;
        Glacier::ZGEOM* m_pChuteColiGeom;
        Glacier::ZGEOM* m_pBedGeom;
        Glacier::ZGEOM* m_pCloset;
        ZHM3Actor* m_pLastCloseCombatStunTarget;
        int* m_pLastUseAnims[2];
        bool m_bPlayedReverse;
        bool m_bAnimSoundStartet;
        bool m_pad7BE[2];
        Glacier::ZVector3 m_vLastValidPFPos;
        bool m_bIsInValidComponent;
        bool m_pad7CD[3];
        ZGEOM* m_pLastDoor;
        Glacier::ZVector3 m_ActorAimPoint;
        Glacier::ZVector3 m_ActorAimOffset;
        float m_fLightMultiplier;
        float m_fVisionRangeMultiplier;
        ZGEOM* m_pPickLockDoor;
        bool m_bBloodyDress;
        bool m_bRightDualWeaponUnequipInProgress;
        bool m_bLeftDualWeaponUnequipInProgress;
        bool m_pad7FB;
        ZGEOM* m_pHeadWear;
        ZGEOM* m_pAttachedItem;
        uint32_t m_lCurrentDefIndex;
        bool m_BloodTrailStartFlag;
        bool m_pad809[3];
        Glacier::ZVector3 m_BloodTrailStartPos;
        Glacier::TIMETYPE m_LastUpdateBloodTrails;
        float m_NextWaitTimeBloodTrails;
        int m_iNumTrailsInCurrentTrail;
        float m_fCurrentTrailSize;
        uint32_t m_rContainingElevator;
        ZHM3Actor* m_pElevatorStrangleTarget;
        bool m_bHMIsOnTopOfElevator;
        bool m_pad831[3];
        float m_fOldMovePoolWeight;
        float m_fLastEnterClosetTime;
        float m_fLastExitClosetTime;
        SRailInfo m_RailInfo;
        bool m_bMatPosForced;
        bool m_pad875[3];
        Glacier::ZVector3 m_vForcePos;
        Glacier::ZMat3x3 m_mForceMat;
        bool m_bDeathSequenceActive;
        bool m_pad8A9[3];
        Glacier::TIMETYPE m_ttDeathSequenceStartTime;
        Glacier::ZGROUP* m_pHitmanDeathSequenceIngame;
        float m_fDeathSequencePostFilterNrFrames;
        float m_fDeathSequenceTimeMultiplier;
        float m_fDeathSequenceTimeToDie;
        int32_t m_iDeathSequenceNrKills;
        float m_fDeathSequenceNrKillsAbortSequence;
        struct ZPostFilterAnimEvent* m_pPostFilterAnimDeathSequence;
        bool m_bDiePostfilterEnabled;
        bool m_pad8CD[3];
        uint32_t m_AudioEffect_Dying;
        uint32_t m_AudioEffect_Sneaking;
        bool m_bControlUseOneAloneDisabled;
        bool m_bEnableControlUseOneAloneNextFrame;
        bool m_pad8DA[2];
        struct ZSoundObject* m_pSoundObject_Sneaking;
        Glacier::ZVector3 m_vForceDragpos;
        bool m_bLandingOnLedge;
        bool m_bTighteningFiberWire;
        bool m_bMountingWallFromBelow;
        bool m_bAutoCrouchWhenTighteningFiberWire;
        float m_fLastHeadShotTime;
        eHideStuffState m_eHideState;
        eHideStuffState m_eHideAnimState;
        float fLastTransTime;

        // This part is unknown and not declared in PS2/iOS builds
        // I guess it's PC specific part
        uint8_t m_aUnknownBytes[44];
	};
    RE_VERIFY_SIZE(ZHitman3, 0x1478);
}
