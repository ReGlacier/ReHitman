#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class Z2DOBJ : public ZSTDOBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(Z2DOBJ, 0x10);

        // vtbl
        ~Z2DOBJ();

        // ZSerializable

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CalcCenSize() override;
        void CopyData(const ZGEOM* Source) override;

        // methods
        Z2DOBJ(const char* psName, ZBaseGeom* pBaseGeom);

        // members
        bool m_bRollAlign;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(Z2DOBJ, 0x14); // Verified PC alloc
}
