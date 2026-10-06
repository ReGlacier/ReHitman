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
        // RTTI
        DECLARE_GEOM_CLASS(ZItemTemplateAmmo, 0x1007D5u);

        // methods
        ZItemTemplateAmmo(const char* psName, ZBaseGeom* pBaseGeom);

        // vtbl (RTTI / ZGEOM / ZItemTemplate overrides)
        ~ZItemTemplateAmmo() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void ClassInit2() override;
        void PostClassInit() override;
        void LoadSave(ISerializerStream& stream, bool bSave) override;
        void SetStates(CCom* pCom) override;
        void CopyData(const ZGEOM* Source) override;
        uint32_t GetItemClassId() const override;

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
    RE_VERIFY_OFFSET(ZItemTemplateAmmo, m_lProjectilesPerMagazine, 0x74);
}
