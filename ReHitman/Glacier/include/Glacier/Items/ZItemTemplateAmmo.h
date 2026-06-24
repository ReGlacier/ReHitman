#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Items/EDamageType.h>

namespace Glacier
{
    class ZItemTemplateAmmo : public ZItemTemplate
    {
    public:
        // vftable
        virtual int GetDefaultProjectilesPerMagazine();
        virtual int GetProjectilesPerShot();
        virtual float GetNearDamage();
        virtual float GetFarDamage();
        virtual void GetMaterialEnumId(int* pRes);
        virtual float GetSplashDamage();
        virtual bool GetCanPenetrate(); //always false :(
        virtual ZGEOM* GetProjectile();
        virtual ZGEOM* GetCartridge();
        virtual ZGEOM* GetProjectileInstance();
        virtual EDamageType GetDamageType();

        // data (total size is 0xA4, ZItemTemplate size is 0x74)
        int m_lProjectilesPerMagazine;
        int m_lProjectilesPerShot;
        float m_fNearDamage;
        float m_fFarDamage;
        bool m_bCanSplashDamage;
        bool m_bUseImpactSound;
        RE_ADD_PADDING(2);
        int m_MaterialEnumId; // typedef ZTypedef<int,EHardTypedef_TEnumID> TEnumID;
        ZREF m_rProjectile;
        ZREF m_rCartridge;
        EDamageType m_eDamageType;
        ZREF* m_pProjectileList;
        uint32_t m_lProjectileListSize;
        uint32_t m_lProjectileCurrent;
    };
    RE_VERIFY_SIZE(ZItemTemplateAmmo, 0xA4); // Verified
}