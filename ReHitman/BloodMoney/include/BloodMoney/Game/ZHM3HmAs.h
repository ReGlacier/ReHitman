#pragma once

#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZBitfield.h>
#include <BloodMoney/Game/LevelControls/ESecurityZone.h>
#include <Glacier/ZSTL/ZRTStringObject.h>

namespace Hitman::BloodMoney
{
    enum EDressQuality : uint32_t {
        eQualityBad = 0,
        eQualityGood = 1,
        eQualitySuper = 2,
        eQualityPerfect = 3,
        eQualityHitman = 4,
    };

    class ZHM3HmAs : public Glacier::ZGROUP
    {
    public:
        /// vftable
        virtual void CopyData(ZGEOM const*) override;
        virtual bool IsZoneAllowed(ESecurityZone zoneKind);

        /// data (total size is 0xA0, parent size is 0x4C)
        Glacier::ZREF m_rClothBundleGroup;
        Glacier::ZAUDIOREF m_FootwearMaterial;
        Glacier::REFTAB m_rtWeaponsAllowed;
        EDressQuality m_eDressQuality;
        Glacier::ZBitfield<ESecurityZone> m_AllowedZones;
        Glacier::REFTAB m_rtItemContainerAllowed;
        int32_t m_iDisguiseGroup;
        bool m_bMayUseLadders;
        Glacier::ZRTString ExtraOptionName;
    };
    RE_VERIFY_SIZE(ZHM3HmAs, 0xA0); // Verified
}