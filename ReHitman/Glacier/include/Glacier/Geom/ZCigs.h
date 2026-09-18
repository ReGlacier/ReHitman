#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZAllocMany.h>


namespace Glacier
{
    class ZCigs : public ZAllocMany
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZCigs, 0x100456u);

        // vtbl
        ~ZCigs() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // methods
        ZCigs(const char* psName, ZBaseGeom* pBaseGeom);

        // members
    };
    RE_VERIFY_SIZE(ZCigs, 0x68); // Verified PC alloc
}
