#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/ZTTFONT.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZKerningFont : public ZTTFONT
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZKerningFont, 0x20004Cu);

        // vtbl (no changes)
        // methods
        ZKerningFont(const char* psName, ZBaseGeom* pBaseGeom);

        // data (size is 0x9C, base size is 0x98)
        int32_t m_iExtraKerning;
    };
    RE_VERIFY_SIZE(ZKerningFont, 0x9C); // Verified PC alloc
}
