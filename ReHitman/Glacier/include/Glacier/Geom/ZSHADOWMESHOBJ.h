#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Geom/ZSTDOBJ.h>


namespace Glacier
{
    class ZSHADOWMESHOBJ : public ZSTDOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZSHADOWMESHOBJ, 0x200110u);

        // vtbl
        ~ZSHADOWMESHOBJ() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        bool DisableParentBoundAdjust() override;
        void ClassInit() override;
        void CopyData(const ZGEOM* Source) override;

        // ZSHADOWMESHOBJ
        virtual bool GetParentBox(float* pMin, float* pMax);

        // methods
        ZSHADOWMESHOBJ(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        int m_iFlags;
        ZREF m_rLight;
        class ZLIGHT* m_pLight;
    };
    RE_VERIFY_SIZE(ZSHADOWMESHOBJ, 0x1C); // Verified PC alloc
}
