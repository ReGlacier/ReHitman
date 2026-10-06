#pragma once

#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZOMNILIGHT : public ZLIGHT
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZOMNILIGHT, 0x800024u);

        // vtbl
        ~ZOMNILIGHT() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // ZLIGHT
        void SetMultiplier(float fValue) override;
        void SetDiffuseColor(uint32_t lColor) override;

        // ZOMNILIGHT
        virtual void SetNearRange(float value);
        virtual void SetFarRange(float value);
        virtual float GetNearRange();
        virtual float GetFarRange();

        // methods
        ZOMNILIGHT(const char* psName, ZBaseGeom* pBaseGeom);
    };
    RE_VERIFY_SIZE(ZOMNILIGHT, 0x20); // Verified PC alloc
}
