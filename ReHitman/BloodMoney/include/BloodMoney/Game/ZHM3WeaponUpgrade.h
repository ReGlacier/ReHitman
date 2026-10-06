#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZMessageResolver.h>
#include <BloodMoney/Game/Items/EUpgradeType.h>

#include <cstdint>


namespace Hitman
{
    class ZHM3ItemWeaponCustom;

    /**
     * @brief Purchase tier of a weapon upgrade (PC enum `EUpgradeTier`, RTP
     * "RTP::Type::Enum::ZHM3WeaponUpgrade::EUpgradeTier"). Stored by ZHM3WeaponUpgrade::m_eTier.
     */
    enum EUpgradeTier : int {
        UPGRADE_TIER1 = 0,
        UPGRADE_TIER2 = 1,
        UPGRADE_TIER3 = 2,
        UPGRADE_TIER4 = 3,
        UPGRADE_TIER5 = 4,
    };

    /**
     * @brief PC RTTI 0x79D31C (118 vtable slots, identical to ZSTDOBJ, so the class adds no
     * virtuals of its own). Registered geom class id 0x200436, parent ZSTDOBJ, size 0x2C
     * (PC 0x746E10 passes size 44).
     *
     * A single purchasable custom-weapon upgrade. The class is a scene-geom wrapper; applying it
     * to an item dispatches ZHM3WeaponUpgrade::m_msgApplyUpgrade ("ApplyWeaponUpgrade") to itself,
     * which the ZHM3WeaponUpgradeProperty event children use to modify the weapon's stats.
     */
    class ZHM3WeaponUpgrade : public Glacier::ZSTDOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3WeaponUpgrade, 0x200436u);

        // static
        STATIC_CLASS_VAR(ZHM3WeaponUpgrade, Glacier::ZMessageResolver, m_msgApplyUpgrade);

        // methods
        ZHM3WeaponUpgrade(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl
        ~ZHM3WeaponUpgrade() override;
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CopyData(const Glacier::ZGEOM* Source) override;

        // methods
        // PC 0x652980. Applies (or, with bApply == false, only re-reads) this upgrade on the item.
        void ApplyUpgrade(ZHM3ItemWeaponCustom* pWeapon, bool bApply);

        // data (total size is 0x2C, base ZSTDOBJ is 0x10)
        float m_fCost;                   // +0x10  "m_fCost"
        EUpgradeType m_eType;            // +0x14  "m_eType"
        Glacier::REFTAB* m_pRequires;    // +0x18  "m_pRequires"
        Glacier::REFTAB* m_pExcludes;    // +0x1C  "m_pExcludes"
        Glacier::REFTAB* m_BoneIncludes; // +0x20  "m_BoneIncludes"
        Glacier::REFTAB* m_BoneExcludes; // +0x24  "m_BoneExcludes"
        EUpgradeTier m_eTier;            // +0x28  "m_eTier"
    };
    RE_VERIFY_SIZE(ZHM3WeaponUpgrade, 0x2C);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgrade, m_fCost, 0x10);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgrade, m_eType, 0x14);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgrade, m_pRequires, 0x18);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgrade, m_pExcludes, 0x1C);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgrade, m_BoneIncludes, 0x20);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgrade, m_BoneExcludes, 0x24);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgrade, m_eTier, 0x28);
}
