#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Runtime/Macro.h>
#include <cstdint>


namespace Glacier
{
    class ZBOUND : public ZSTDOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZBOUND, 0x20001Cu);

        // vtbl
        ~ZBOUND() override;

        // ZSerializable
        bool PostLoad(ISerializerStream& stream) override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        eGlobalTreeType GetBoundTreeType() const override;

        // methods
        ZBOUND(const char* psName, ZBaseGeom* pBaseGeom);
    };
    RE_VERIFY_SIZE(ZBOUND, 0x10); // Verified PC alloc
}
