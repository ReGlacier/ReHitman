#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZSDOwner.h>

#include <BloodMoney/Game/Items/EHM3ItemType.h>
#include <BloodMoney/Game/LevelControls/ESecurityZone.h>

namespace Hitman::BloodMoney
{
    class ZHM3HmAs;
    class ZHM3Item;
    class ZHM3ItemBomb;
    class ZHM3ItemWeapon;
    class ZHM3ClothBundle;

    class ZMusicController;

    class ZHM3LevelControl : public Glacier::ZEventBase
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
         */
        int m_field30;
        ZMusicController* m_musicController;
        Glacier::ZSDOwner m_sdOwner;
        int m_field48;
        Glacier::REFTAB m_reftab4C;
        int m_hashTab68;
        int m_field6C;
        int m_field70;
        int m_field74;
        int m_field78;
        int m_field7C;
        int m_field80;
        int m_field84;
        int m_hashTab88;
        int m_field8C;
        int m_field90;
        int m_field94;
        int m_field98;
        int m_field9C;
        int m_fieldA0;
        int m_fieldA4;
        int m_fieldA8;
        int m_fieldAC;
        int m_fieldB0;
        int m_fieldB4;
        int m_fieldB8;
        int m_fieldBC;
        int m_fieldC0;
        int m_fieldC4;
        int m_fieldC8;
        int m_fieldCC;
        int m_fieldD0;
        int m_fieldD4;
        int m_fieldD8;
        int m_fieldDC;
        int m_fieldE0;
        int m_fieldE4;
        int m_fieldE8;
        int m_fieldEC;
        int m_fieldF0;
        int m_fieldF4;
        int m_fieldF8;
        int m_fieldFC;
        int m_field100;
        int m_field104;
        int m_field108;
        int m_field10C;
        int m_field110;
        int m_field114;
        int m_field118;
        int m_field11C;
        int m_field120;
        int m_field124;
        int m_field128;
        int m_field12C;
        int m_field130;
        int m_field134;
        int m_field138;
        int m_field13C;
        int m_field140;
        int m_field144;
        int m_field148;
        int m_field14C;
        int m_field150;
        int m_field154;
        int m_field158;
        int m_field15C;
        int m_field160;
        int m_field164;
        int m_field168;
        int m_field16C;
        int m_field170;
        int m_field174;
        int m_field178;
        int m_field17C;
        int m_field180;
        int m_field184;
        int m_field188;
        int m_field18C;
        int m_field190;
        int m_field194;
        int m_field198;
        int m_field19C;
        int m_field1A0;
        int m_field1A4;
        int m_field1A8;
        int m_field1AC;
        char m_field1B0;
        char field_1B1;
        char field_1B2;
        char field_1B3;
        int m_field1B4;
        int m_field1B8;
        int m_field1BC;
        int m_field1C0;
        int m_field1C4;
        int m_field1C8;
        int m_field1CC;
        int m_field1D0;
        int m_field1D4;
        int m_field1D8;
        int m_field1DC;
        int m_field1E0;
        int m_field1E4;
        int m_field1E8;
        Glacier::REFTAB m_reftab1EC;
        Glacier::REFTAB32 m_reftab32_208;
        int m_field2B4;
        int m_field2B8;
        int m_field2BC;
        char m_field2C0;
        char field_2C1;
        char field_2C2;
        char field_2C3;
        int m_field2C4;
        int m_field2C8;
        int m_field2CC;
        int m_field2D0;
        int m_field2D4;
        int m_field2D8;
        int m_field2DC;
        int m_field2E0;
        int m_field2E4;
        int m_field2E8;
        int m_field2EC;
        int m_field2F0;
        int m_field2F4;
        int m_field2F8;
        int m_field2FC;
        int m_field300;
        int m_field304;
        int m_field308;
        int m_field30C;
        int m_field310;
        int m_field314;
        int m_field318;
        Glacier::REFTAB32 m_reftab32_31C;
        Glacier::REFTAB32 m_reftab32_3C8;
        Glacier::REFTAB32 m_reftab32_474;
        int m_field520;
        Glacier::REFTAB32 m_reftab32_524;
        int m_field5D0;
        int m_field5D4;
        int m_field5D8;
        int m_field5DC;
    };
}