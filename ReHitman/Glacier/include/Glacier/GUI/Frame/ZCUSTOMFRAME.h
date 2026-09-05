#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/Frame/ZFRAME.h>


namespace Glacier
{
    class ZCUSTOMFRAME : public ZFRAME
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZCUSTOMFRAME, 0x20004Fu);

        // vtbl
        ~ZCUSTOMFRAME() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CopyData(const ZGEOM* pSource) override;
        void SetOuterSize(int width, int height) override;
        void CreateGeometry() override;

        // methods
        ZCUSTOMFRAME(const char* psName, ZBaseGeom* pBaseGeom);
        void GetOuterSize(int32_t& width, int32_t& height) const;

        // data
        int32_t m_aWidths[3];
        int32_t m_aHeights[3];
    };
    RE_VERIFY_SIZE(ZCUSTOMFRAME, 0xD0); // Verified PC alloc
}
