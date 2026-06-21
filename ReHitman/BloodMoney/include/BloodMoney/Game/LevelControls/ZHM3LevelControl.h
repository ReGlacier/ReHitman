#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/Geom/ZBoxPrimitive.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/ZSDOwner.h>
#include <Glacier/ZSTL/ZHash.h>

#include <BloodMoney/Game/Items/EHM3ItemType.h>
#include <BloodMoney/Game/LevelControls/ESecurityZone.h>

namespace Hitman::BloodMoney
{
    class ZHM3Actor;
    class ZHM3HmAs;
    class ZHM3Item;
    class ZHM3ItemBomb;
    class ZHM3ItemWeapon;
    class ZHM3ClothBundle;

    class ZMusicController;

    class ZHitmanWeaponStorage;
    class ZPostFilterCollectionEvent;
    class ZPostFilterEvent;
    class ZPostFilterSetEvent;

    class ZHM3LevelControl : public Glacier::CBaseEvent<Glacier::ZBoxPrimitive>
    {
    public:
        // vftable
        virtual void MusicInit();
        virtual void CharacterKilled(Glacier::ZREF actorRef);
        virtual void CharacterHarmed(Glacier::ZREF actorRef);
        virtual void HitmanHideWeapon();
        virtual void HitmanDragBody(Glacier::ZGEOM *);
        virtual void HitmanDropDeadBody(unsigned int,int);
        virtual void HitmanPickupItem(Glacier::ZItem *);
        virtual void HitmanDropItem(Glacier::ZItem *);
        virtual void HitmanUseItem(Glacier::ZItem *);
        virtual void HitmanThrowItem(Glacier::ZItem *);
        virtual void HitmanGetCurrentUBAnim(unsigned int *);
        virtual void HitmanDied();
        virtual void HitmanNearDeath();
        virtual void OnHitmanMayAttackCloseCombat(ZHM3ItemWeapon *,ZHM3Actor *);
        virtual void OnHitmanMayStrangulateTarget(ZHM3Actor *);
        virtual void OnHitmanMayDisarmTarget(ZHM3Actor *);
        virtual void OnHitmanTakeHumanShield(ZHM3Actor *);
        virtual void OnHitmanUnpackSniper();
        virtual void OnHitmanCloseInventory();
        virtual void OnHitmanChangeClothes(ZHM3HmAs *, Glacier::ZGEOM *);
        virtual void OnDropHitmanClothBundle(ZHM3ClothBundle *);
        virtual void BombExploded(ZHM3ItemBomb *);
        virtual void OnBombPlaced(ZHM3ItemBomb *);
        virtual void BombAddedToHitman(Glacier::ZItem *);
        virtual void MissionFailed(char const* messageDialog);
        virtual void CalculateExtraStats();
        virtual void MissionCompleted();
        virtual void MissionCompleted2();
        virtual void TerminateScene();
        virtual void CompleteObjective(int, bool);
        virtual void UnCompleteObjective(int);
        virtual void HideObjective(int);
        virtual void UnHideObjective(int);
        virtual void IsHitmanAllowedHere(ESecurityZone,ZHM3HmAs *, Glacier::ZGEOM *, Glacier::ZItem *, Glacier::ZItem *);
        virtual void IsIgnoredWeapon(Glacier::ZItemWeapon *);
        virtual void IsWeaponFoundAtHitman(bool &);
        virtual void TransferHitmanWeaponsToActor(Glacier::ZREF itemRef, bool&);
        virtual void CanHMSyringeTarget(ZHM3Actor *, ZHM3ItemWeapon *);
        virtual void CanHMSyringeItem(ZHM3Item *,ZHM3ItemWeapon *);
        virtual void CanHitmanFiberWireTarget(ZHM3Actor *);
        virtual void CanHMPutItemIntoElevator(Glacier::ZGEOM *);
        virtual void OnObjectExplode(Glacier::ZGEOM *);
        virtual void CanHMOperateElevatorButton(Glacier::ZAction *,bool &);
        virtual void OnActorDropItemsInHand(ZHM3Actor *);
        virtual void DisguiseUsed(Glacier::ZREF);
        virtual void CanHMPlaceItemToContainer(Glacier::ZItemContainer *,bool &);
        virtual void CanHMRetrieveItemFromContainer(Glacier::ZItemContainer *,bool &);
        virtual void PlaceItemToContainer(Glacier::ZItemContainer *,Glacier::ZItem *);
        virtual void RetrieveItemFromContainer(Glacier::ZItemContainer *, Glacier::ZItem *);
        virtual void OnHitmanPlaceItemToContainer(Glacier::ZItemContainer *,Glacier::ZItem *);
        virtual void OnHitmanRetrieveItemFromContainer(Glacier::ZItemContainer *,Glacier::ZItem *);
        virtual void CanHitmanOperateGenericAction(Glacier::ZAction *,bool &);
        virtual void OnHitmanBreakUtilBox(Glacier::ZGEOM *);
        virtual void OnUtilBoxRepaired(Glacier::ZGEOM *);
        virtual void OnPickLockOpenDoor(Glacier::ZGEOM *);
        virtual void OnOpenedDoorWithKeycard(Glacier::ZGEOM *);
        virtual void OnHitmanReloadWeapon(Glacier::ZItem *);
        virtual void OnHitmanPickupWeapon(Glacier::ZItem *);
        virtual void OnHitmanPickupItem(Glacier::ZItem *);
        virtual void OnHitmanDisarmNPC(ZHM3Actor *);
        virtual void OnHitmanPutWeaponRightHandItem(Glacier::ZItem *);
        virtual void OnHitmanPlacedBomb();
        virtual void OnHitmanUseSyringeOnItem(ZHM3Item *);
        virtual void OnHitmanUseSyringe(EHM3ItemType);
        virtual void OnHitmanHideInCloset(Glacier::ZGEOM *);

        /**
         *  In-game level controls size stats
         *
         *  Hideout     - 0x618
         *  M00         - 0x6B8
         *  M01         - 0x5EC
         *  M02         - 0x664
         *  M03         - 0x6E0
         *  M04         - 0x77C
         *  M05         - 0x734
         *  M06         - 0x88C
         *  M07         - 0x5E0 (Original size is here), M07 - cropped mission from game
         *  M08         - 0x664
         *  M09         - 0x65C
         *  M10         - 0x604
         *  M11         - 0x6C0
         *  M12         - 0x62C
         *  M13         - 0x668
         *
         *  So, total size of class is 0x5E0, ZEventBase size is 0x30
         * 
         * 21.06.26: Updated base size to 0x5D8 because base class data found in PS2 build.
         */
        float m_fHitmanDiedAt; //+0x30
        ZMusicController* m_pMusicController;
        Glacier::ZSDOwner m_Music;
        int m_iMusicDefinition;
        Glacier::REFTAB m_rHitmanChangeClothesSubscribers;
        Glacier::ZIntHash m_WeaponsUsedToKill;
        Glacier::ZIntHash m_WeaponsFired;
        int m_iNumWeaponsUsed;
        Glacier::ZREF m_pDisguisesUsed[64];
        int m_iNumDisguisesUsed;
        bool m_bTapeStolen;
        RE_ADD_PADDING(3);
        int m_iCustomWeaponsStart;
        int m_iActorCount;
        Glacier::ZREF m_arTargets[6];
        Glacier::ZREF m_arTargetsKilledWith[6];
        Glacier::REFTAB m_HarmedCharacters;
        Glacier::REFTAB32 m_rtAudioPauseObjects;
        float m_fTimeSinceLastOneliner;
        int m_lObjectiveHiddenFlags;
        int m_lObjectiveCompletedFlags;
        bool m_bShowNotoriety;
        RE_ADD_PADDING(3);
        ZHitmanWeaponStorage* m_pNormalWeaponStorage;
        ZHitmanWeaponStorage* m_pSmuggleWeaponStorage;
        Glacier::ZVector3 m_vP1;
        Glacier::ZVector3 m_vP2;
        Glacier::ZVector3 m_vP3;
        Glacier::ZVector3 m_vN;
        int m_iSyringeUsed_HeartAttack;
        int m_iSyringeUsed_Poison;
        int m_iSyringeUsed_Anastetic;
        ZPostFilterCollectionEvent* m_pPFCollection;
        ZPostFilterEvent* m_pPF;
        ZPostFilterSetEvent* m_pPFSet;
        float m_fPFBlend;
        int m_PFFadeDir;
        Glacier::REFTAB32 m_BeginningItems;
        Glacier::REFTAB32 m_rtClothlist;
        Glacier::REFTAB32 m_rtWitnesses;
        Glacier::ZREF m_rSecurityDevice;
        Glacier::REFTAB32 m_rtSuitcaseBoxes;
        Glacier::ZREF m_rBreathEmitter;
        Glacier::ZREF m_rWaterSplashParticleSystem;
    };
    RE_VERIFY_SIZE(ZHM3LevelControl, 0x5D8);
    RE_VERIFY_OFFSET(ZHM3LevelControl, m_fHitmanDiedAt, 0x30);
    RE_VERIFY_OFFSET(ZHM3LevelControl, m_pMusicController, 0x34);
}