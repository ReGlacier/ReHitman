#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/Runtime/Macro.h>


namespace Glacier
{
    class ZBackdrop : public ZROOM
    {
    public:
        // runtime
        DECLARE_GEOM_CLASS(ZBackdrop, 0x10003Fu);

        //vftable

        //data (total size is 0x248, base size is 0x144)
        ZStackArray<64, ZGROUP*> m_Groups; //+0x144
    };
    RE_VERIFY_SIZE(ZBackdrop, 0x248); // Verified
    RE_VERIFY_OFFSET(ZBackdrop, m_Groups, 0x144);
}
