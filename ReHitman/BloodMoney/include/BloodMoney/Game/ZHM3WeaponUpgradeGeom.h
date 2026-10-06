#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/Runtime/Macro.h>
#include <BloodMoney/Game/Items/EUpgradeType.h>

#include <cstdint>


namespace Hitman
{
    /**
     * @brief PC RTTI 0x79CEEC (118 vtable slots, identical to ZSTDOBJ). Registered geom class id
     * 0x20044C, parent ZSTDOBJ, size 0x14 (PC 0x746E70 passes size 20).
     *
     * Minimal upgrade marker geom: it only carries the EUpgradeType it represents ("m_eType"). The
     * PC ctor (0x653110) chains ZSTDOBJ, installs the vftable and touches nothing else; the extra
     * 4 bytes over ZSTDOBJ (0x10) are exactly this RTP enum member.
     */
    class ZHM3WeaponUpgradeGeom : public Glacier::ZSTDOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3WeaponUpgradeGeom, 0x20044Cu);

        // methods
        ZHM3WeaponUpgradeGeom(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl
        ~ZHM3WeaponUpgradeGeom() override;
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // data (total size is 0x14, base ZSTDOBJ is 0x10)
        EUpgradeType m_eType; // +0x10  "m_eType"
    };
    RE_VERIFY_SIZE(ZHM3WeaponUpgradeGeom, 0x14);
    RE_VERIFY_OFFSET(ZHM3WeaponUpgradeGeom, m_eType, 0x10);
}
