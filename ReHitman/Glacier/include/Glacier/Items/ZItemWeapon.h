#pragma once

#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/EWeaponOperation.h>
#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    class ZItemWeapon : public ZItem
    {
    public:
        //vftable
        virtual void DestroyItem();
        virtual void SetAmmoTemplate(ZItemTemplateAmmo*);
        virtual ZItemTemplateAmmo* GetAmmoTemplate();
        virtual void SetTarget(const Glacier::ZVector3* target);
        virtual bool GetTarget(Glacier::ZVector3* targetPos); // return true is result vector isn't zeroed
        virtual void* Target();
        virtual WEAPONOPERATION GetWeaponOperation();
        virtual void SetWeaponOperation(WEAPONOPERATION weaponOperation);
        virtual void SelectNextWeaponOperation();
        virtual void GetFirePosition(ZMat3x3* mat, ZVector3* pos); //TODO: Recognize math types here
        virtual void* GetMuzzleExitPos();
        virtual void GetRootMuzzleExitPos(const Glacier::ZVector3* result);
        virtual ZItemWeaponTemplate* GetWeaponTemplate();
        virtual bool GetBulletInChamber();
        virtual void SetBulletInChamber(bool value);
        virtual int GetProjectilesInMagazine(); //Could be overridden by cheat 'Inf Ammo'
        virtual void SetProjectilesInMagazine(int value);
        virtual int GetProjectilesPerMagazine();
        virtual bool WeaponReady();
        virtual void SetUseBulletsFromMagazine(bool);
        virtual int GetNumOfBulletsPerReloadCycle();
        virtual void ReloadStart();
        virtual void ReloadEnd();
        virtual void ReloadStartActivateAnimation();
        virtual void ChamberStart();
        virtual void ChamberEnd();
        virtual void FireStart();
        virtual void FireEnd();
        virtual void FireRound();
        virtual void EmptyFire();
        virtual void FireRoundActivateAnimation();
        virtual void OnActivateProjectile(ZGEOM*);
        virtual void CopyGeom(ZGEOM* from, ZGEOM* unused, ZGROUP* inGroup, bool makeActive);

        //data (total size is 0xDC, ZItem size is 0x84)
        Glacier::ZVector3 m_vTarget;
        Glacier::ZREF m_rAmmoTemplate;
        int m_lProjectilesInMagazine;
        bool m_bBulletInChamber;
        RE_ADD_PADDING(3);
        bool m_lBurstCount;
        bool m_bRequestFireRelease;
        bool m_bReloading;
        bool m_bChambering;
        bool m_bTriggerHeld;
        RE_ADD_PADDING(3);
        int m_fTimeLastShot;
        EWeaponOperation m_eWeaponOperation;
        REFTAB* m_prtWeaponParts;
        int m_rParticleController;
        int m_lMuzzleFireIndex;
        int m_lMuzzleSmokeIndex;
        int m_lCartridgeIndex;
        Glacier::ZREF m_rMuzzleLight;
        Glacier::ZVector3 m_vMuzzleLightAlign;
        Glacier::ZREF m_rSlide;
        Glacier::ZREF m_rClip;
        bool m_useBulletsFromMagazine;
        bool m_bWantSoundEvent;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(ZItemWeapon, 0xDC); // Verified

}