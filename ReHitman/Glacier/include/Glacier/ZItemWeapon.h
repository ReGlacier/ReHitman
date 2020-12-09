#pragma once

#include <Glacier/ZItem.h>
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
        Glacier::ZVector3 m_targetPosition;
        Glacier::ZREF m_ammoTemplateREF;
        int m_projectilesPerMagazine;
        bool m_isBulletInChamber;
        char m_field99;
        char m_field9A;
        char m_field9B;
        int m_field9C;
        int m_fieldA0;
        int m_fieldA4;
        int m_nextOperation;
        int m_fieldAC;
        int m_fieldB0;
        int m_fieldB4;
        int m_fieldB8;
        int m_fieldBC;
        int m_fieldC0;
        int m_fieldC4;
        int m_fieldC8;
        int m_fieldCC;
        Glacier::ZREF m_fieldD0;
        int m_fieldD4;
        bool m_useBulletsFromMagazine;
    };

}