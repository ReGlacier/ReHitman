#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GUI/ZCHAROBJ.h>


namespace Glacier
{
    class ZExtCharObj : public ZCHAROBJ
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZExtCharObj, 0x200039u);

        // vtbl
        // data (no new data)
    };
    RE_VERIFY_SIZE(ZExtCharObj, 0xA0);
}
