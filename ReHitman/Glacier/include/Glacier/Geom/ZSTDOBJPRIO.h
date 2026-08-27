#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Runtime/Macro.h>
#include <cstdint>


namespace Glacier
{
    class ZSTDOBJPRIO : public ZSTDOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZSTDOBJPRIO, 0x200463u);

        // vtbl
        ~ZSTDOBJPRIO() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        uint8_t GetSortPriority() const override;
        void CopyData(const ZGEOM* Source) override;

        // methods
        ZSTDOBJPRIO(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        uint8_t m_lSortPriority;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZSTDOBJPRIO, 0x14); // Verified PC alloc
}
