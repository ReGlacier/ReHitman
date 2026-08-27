#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZSPOTLIGHT : public ZLIGHT
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZSPOTLIGHT, 0x800023u);

        // vtbl
        ~ZSPOTLIGHT() override;

        // ZSerialiable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        void CopyData(const ZGEOM* Source) override;

        // ZLIGHT
        void SetMultiplier(float fValue) override;
        void SetDiffuseColor(uint32_t lColor) override;

        // ZSPOTLIGHT
        virtual float GetHotSpotAngle();
        virtual float GetFallOffAngle();
        virtual float GetNearRange();
        virtual float GetFarRange();

        // methods
        ZSPOTLIGHT(const char* psName, ZBaseGeom* pBaseGeom);
    };
    RE_VERIFY_SIZE(ZSPOTLIGHT, 0x20);
}
