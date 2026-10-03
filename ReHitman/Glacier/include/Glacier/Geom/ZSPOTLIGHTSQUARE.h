#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZSPOTLIGHTSQUARE : public ZLIGHT
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZSPOTLIGHTSQUARE, 0x800020u);

        // vtbl
        ~ZSPOTLIGHTSQUARE() override;

        // ZSerializable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;

        // ZLIGHT
        void SetMultiplier(float fValue) override;
        void SetDiffuseColor(uint32_t lColor) override;

        // methods
        ZSPOTLIGHTSQUARE(const char* psName, ZBaseGeom* pBaseGeom);
    };
    RE_VERIFY_SIZE(ZSPOTLIGHTSQUARE, 0x20); // Verified PC alloc
}
