#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZItemTemplate.h>

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
        virtual int GetDamageType();

        // data (total size is 0xA4, ZItemTemplate size is 0x74)
        int m_defaultProjectilesPerMagazine;
        int m_projectilesPerShot;
        float m_nearDamage;
        int m_field80;
        bool m_bCanSplashDamage;
        char m_field_85;
        char m_field_86;
        char m_field_87;
        int m_materialEnumID;
        Glacier::ZREF m_projectileRef;
        Glacier::ZREF m_cartridgeRef;
        int m_damageType;
        int m_field98;
        int m_field9C;
        int m_fieldA0;
    };
}