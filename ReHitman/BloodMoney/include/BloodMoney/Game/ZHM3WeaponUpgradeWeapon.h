#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <cstdint>


namespace Hitman
{
    class ZHM3ItemWeaponCustom;

    /**
     * @brief PC RTTI 0x79D0CC (147 vtable slots, identical to ZGROUP, so the class adds no
     * virtuals of its own). Registered geom class id 0x100440, parent ZGROUP, size 0x50.
     *
     * It holds the set of "default" visible bones of a custom weapon upgrade; ApplyDefaultBones
     * pushes every referenced bone name onto the given weapon's visible-bone list.
     */
    class ZHM3WeaponUpgradeWeapon : public Glacier::ZGROUP
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3WeaponUpgradeWeapon, 0x100440u);

        // methods
        ZHM3WeaponUpgradeWeapon(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl
        ~ZHM3WeaponUpgradeWeapon() override;
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CopyData(const Glacier::ZGEOM* Source) override;

        // methods
        void ApplyDefaultBones(ZHM3ItemWeaponCustom* pCustomGun);

        // data (total size is 0x50, base ZGROUP is 0x4C)
        Glacier::REFTAB* m_pDefaultBones; // +0x4C
    };
    RE_VERIFY_SIZE(ZHM3WeaponUpgradeWeapon, 0x50);
}
