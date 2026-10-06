#pragma once

#include <Glacier/Items/EWeaponOperation.h>
#include <Glacier/ZSTL/ZBitfield.h>

#include <BloodMoney/Game/Items/ZHM3ItemTemplateAmmo.h>
#include <BloodMoney/Game/Items/EHM3RecoilRandom.h>
#include <BloodMoney/Game/Items/EHM3WeaponScope.h>
#include <BloodMoney/Game/Items/ESilencerType.h>


namespace Hitman
{
    class ZHM3ItemTemplateAmmoCustom : public ZHM3ItemTemplateAmmo
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3ItemTemplateAmmoCustom, 0x10042Eu);

        // methods
        ZHM3ItemTemplateAmmoCustom(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl (RTTI)
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        struct SDefaultValues // Тип 3358
        {
            int32_t m_lProjectilesPerMagazine;
            int32_t m_lProjectilesPerShot;
            float m_fNearDamage;
            float m_fFarDamage;
            bool m_bCanPenetrate;
            RE_ADD_PADDING(3);
        };
        RE_VERIFY_SIZE(SDefaultValues, 0x14);

        // vftable
        bool GetCanPenetrate() override;

        // data (total size is 0xC0, ZHM3ItemTemplateAmmo size is 0xA8)
        SDefaultValues m_DefaultValues;
        bool           m_bCanPenetrate;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZHM3ItemTemplateAmmoCustom, 0xC0); // Verified
    RE_VERIFY_OFFSET(ZHM3ItemTemplateAmmoCustom, m_DefaultValues, 0xA8);
    RE_VERIFY_OFFSET(ZHM3ItemTemplateAmmoCustom, m_bCanPenetrate, 0xBC);
}