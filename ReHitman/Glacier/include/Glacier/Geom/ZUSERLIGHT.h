#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZUSERLIGHT : public ZLIGHT
    {
    public:
        // types
        enum EType
        {
            LIGHTTYPE_OMNI = 0x0,
            LIGHTTYPE_SPOT = 0x1,
            LIGHTTYPE_SPOTSQUARE = 0x2,
            LIGHTTYPE_ENVIRONMENT = 0x3,
        };

        // RTTI
        DECLARE_GEOM_CLASS(ZUSERLIGHT, 0x8000E6u);

        // vtbl
        ~ZUSERLIGHT() override;

        // ZSerializable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;

        // methods
        ZUSERLIGHT(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        uint32_t m_lLightType;
        uint32_t m_lDiffuseColor1;
        uint32_t m_lDiffuseColor2;
        float m_fMultiplier;
        float m_fNearRange;
        float m_fFarRange;
        float m_fHotSpotAngle;
        float m_fFallOffAngle;
        float m_fAspect;
    };
    RE_VERIFY_SIZE(ZUSERLIGHT, 0x44); // Verified PC alloc
}
