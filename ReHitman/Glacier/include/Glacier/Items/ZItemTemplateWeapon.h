#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Items/EWeaponOperation.h>

namespace Glacier
{
    class ZItemTemplateWeapon : public ZItemTemplate
    {
    public:
        // vftable
        virtual void CreateItemAndActuallyUseDestinationParameter(ZGROUP*, unsigned int);
        virtual void DestroyItem(ZItem*); //actually not destroying anything :sadpeppe:
        virtual bool GetRecoil();
        virtual bool CanHaveMagazines(); //returns same to CanFireProjectiles
        virtual bool CanFireProjectiles();
        virtual void SetCanFireProjectiles(bool);
        virtual bool HasSniperMode();
        virtual int GetWeaponType();
        virtual EWeaponOperation GetWeaponOperations();
        virtual int GetTimeBetweenShots();
        virtual ZItemTemplateAmmo* GetAmmoTemplate(int ammoIndex);
        virtual float GetMuzzleVelocity();
        virtual float GetNearRange();
        virtual float GetFarRange();
        virtual float GetPrecisionDegrees();
        virtual int GetDefaultProjectilesPerMagazine();
        virtual float GetCartridgeSpeed();
        virtual void SelectNextWeaponOperation(WEAPONOPERATION weaponOperation);
        virtual double CalcDamage(const ZItemWeapon* item, float distance);
        virtual double CalcImpact(const ZItemWeapon* item, float distance);
        virtual REFTAB* GetWeaponParts();
        virtual int GetMuzzleFire();
        virtual int GetMuzzleSmoke();
        virtual int GetMuzzleLight();
        virtual int GetCartridge(); //Same to GetMuzzleSmoke
        virtual int GetMuzzleEffect();

        // data (total size is 0x15C, ZItemTemplate size is 0x74)
        ZSDOwner m_soundOwner;
        Glacier::ZMSGID m_MSG_SniperMainCamera_SetQuality;
        Glacier::ZMSGID m_MSG_SniperMainCamera_SetOrgMatrix;
        Glacier::ZMSGID m_MSG_SniperMainCamera_SetOrgPosition;
        Glacier::ZMSGID m_MSG_SniperMainCamera_SetOverlay;
        int m_field8C;
        int m_field90;
        int m_field94;
        int m_field98;
        int m_field9C;
        EWeaponOperation m_pWeaponOperations;
        int m_weaponType;
        REFTAB m_allowedAmmoTypes;
        float m_fMuzzleVelocity;
        float m_fPrecisionDegrees;
        float m_fNearRange;
        float m_fFarRange;
        int m_fieldD4;
        int m_fieldD8;
        float m_fTimeBetweenShots;
        bool m_bCanFireProjectiles;
        char m_bCanHaveMagazines;
        bool m_bHasSniperMode;
        char field_E3;
        REFTAB m_weaponParts;
        int m_muzzleEffectREF;
        int m_field104;
        int m_field108;
        Matrix3x3 m_transform_10C;
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
    };
}