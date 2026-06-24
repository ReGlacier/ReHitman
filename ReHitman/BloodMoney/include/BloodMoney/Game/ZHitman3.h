#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZPlayer.h>
#include <Glacier/Glacier.h>
#include <Glacier/ZSTL/zstring.h>

#include <BloodMoney/Game/Actions/EHM3Action.h>

namespace Hitman::BloodMoney
{
    // fwds
    class ZHM3Actor;

    // aliases
    using ZStackArrayLocalizedActionNames = Glacier::ZStackArray<61, Glacier::zstring>;

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
        class ZHitman3* m_pHitman;
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
        class ZHitman3* m_pHitman;
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

	/*
		Inheritance tree:
		ZHitman3: 
			ZPlayer -> ZLNKWHANDS -> ZCTRLIKLNKOBJ -> ZIKLNKOBJ -> ZLNKOBJ -> ZSTDOBJ -> ZGEOM -> RTP::cBase -> ZSerializable -> ZSerializableBase
			ZHM3InputControl
			ZHM3MovementControl 
			ZHM3WeaponsControl
			ZHM3Inventory
			ZHM3ActionControl

        Not finished, test only
	*/
	class ZHitman3 : public Glacier::ZPlayer, public ZHM3InputControl, public ZHM3MovementControl, public ZHM3WeaponsControl, public ZHM3Inventory, public ZHM3ActionControl
	{
        // Test only
        RE_ADD_PADDING(0x92C);
	};
    RE_VERIFY_SIZE(ZHitman3, 0x1478);
}