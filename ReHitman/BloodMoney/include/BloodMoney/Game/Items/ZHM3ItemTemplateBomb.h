#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateWeapon.h>

namespace Hitman
{
    class ZHM3ItemTemplateBomb : public ZHM3ItemTemplateWeapon
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3ItemTemplateBomb, 0x100451u);

        // methods
        ZHM3ItemTemplateBomb(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl (RTTI)
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // vftable (base override implemented by this class)
        // PC slot 149
        uint32_t GetItemClassId() const override;

        // data (total size is 0x1B4, base size is 0x1A0)
        float m_fMaxDamage;
        float m_fMaxDamageRange;
        float m_fMaxRange;
        float m_fExplodeTimer;
        Glacier::ZGROUP* m_pEffectGroup;
    };
    RE_VERIFY_SIZE(ZHM3ItemTemplateBomb, 0x1B4); // Verified
}