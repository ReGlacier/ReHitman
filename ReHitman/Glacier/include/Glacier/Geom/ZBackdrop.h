#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/ZSTL/ZStackArray.h>

namespace Glacier
{
    class ZBackdrop : public ZROOM
    {
    public:
        //vftable

        //data (total size is 0x248, base size is 0x144)
        ZStackArray<64, ZGROUP*> m_Groups; //+0x144
    };
    RE_VERIFY_SIZE(ZBackdrop, 0x248);
    RE_VERIFY_OFFSET(ZBackdrop, m_Groups, 0x144);
}