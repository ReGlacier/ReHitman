#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZEventBase.h>

namespace Hitman::BloodMoney
{
    class ZHM3HmAs;
    class ZHM3Item;
    class ZHM3ItemBomb;
    class ZHM3ItemWeapon;
    class ZHM3ClothBundle;

    enum class ESecurityZone : int {};
    enum class EHM3ItemType : int {};

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
        // data

    };
}