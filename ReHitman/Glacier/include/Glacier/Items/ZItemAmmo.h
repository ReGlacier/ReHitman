#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Items/ZItem.h>

namespace Glacier
{
    class ZItemAmmo : public ZItem
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZItemAmmo, 0x1007D8u);

        // methods
        ZItemAmmo(const char* psName, ZBaseGeom* pBaseGeom);

        // vtbl (RTTI / ZItem overrides)
        ~ZItemAmmo() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CopyData(const ZGEOM* Source) override;

        // vftable
        virtual int GetNrProjectiles();
        virtual int SetNrProjectiles(int value);
        virtual void AddNrProjectiles(int amount);
        virtual void SubNrProjectiles(int amount);

        // data (total size is 0x88, ZItem size is 0x84)
        int m_lNrProjectiles;
    };
    RE_VERIFY_SIZE(ZItemAmmo, 0x88); // Verified
}
