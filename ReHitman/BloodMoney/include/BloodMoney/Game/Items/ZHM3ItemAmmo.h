#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Items/ZItemAmmo.h>
#include <BloodMoney/Game/Items/EHM3ItemType.h>

namespace Hitman
{
    class ZHM3ItemAmmo : public Glacier::ZItemAmmo
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3ItemAmmo, 0x100434u);

        // methods
        ZHM3ItemAmmo(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl (RTTI)
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // vftable
        virtual EHM3ItemType GetHM3ItemType();
    };
    RE_VERIFY_SIZE(ZHM3ItemAmmo, 0x88); // Verified
}