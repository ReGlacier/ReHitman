#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZLIST.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZWINOBJSPRITEHOLDER : public ZLIST
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZWINOBJSPRITEHOLDER, 0x8000049u);

        // vtbl
        ~ZWINOBJSPRITEHOLDER() override;

        // RTP::cBase
        const RTP::ZPropertyInfo& GetProperties() const override;

        // ZGEOM
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // methods
        ZWINOBJSPRITEHOLDER(const char* psName, ZBaseGeom* pBaseGeom);

        static REFTAB* GetFreeSpriteArraysList();
        void SaveCleanup(bool bSaving);

        // members
        REFTAB32 m_rtFreeSpriteArrays;
    };
    RE_VERIFY_SIZE(ZWINOBJSPRITEHOLDER, 0xC0);
    RE_VERIFY_OFFSET(ZWINOBJSPRITEHOLDER, m_rtFreeSpriteArrays, 0x14);
}
