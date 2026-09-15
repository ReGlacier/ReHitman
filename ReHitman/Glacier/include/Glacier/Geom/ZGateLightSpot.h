#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSPOTLIGHT.h>
#include <Glacier/Runtime/Macro.h>
#include <Glacier/ZSTL/REFTAB.h>


namespace Glacier
{
    class ZGateLightSpot : public ZSPOTLIGHT
    {
    public:
        // types
        enum EType
        {
            CLONES = 0,
            AIMATGATES = 1
        };

        enum EFilterType
        {
            EXCLUDE = 0,
            INCLUDE = 1
        };

        // RTTI
        DECLARE_GEOM_CLASS(ZGateLightSpot, 0x8000F9u);

        // vtbl
        ~ZGateLightSpot() override;

        // ZSerializable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void Activate(bool bActive) override;
        void CopyData(const ZGEOM* Source) override;

        // ZLIGHT
        void Enable() override;
        void Disable() override;
        void SetMultiplier(float fValue) override;

        // methods
        ZGateLightSpot(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        EType m_lType;
        REFTAB m_RoomsFilter;
        EFilterType m_lFilterType;
        REFTAB m_Slaves;
    };
    RE_VERIFY_SIZE(ZGateLightSpot, 0x60); // Verified PC alloc
}
