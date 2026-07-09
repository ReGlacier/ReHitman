#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>


namespace Glacier
{
    class Z2DOBJ : public ZSTDOBJ
    {
    public:
        // vtbl (no new members)
        // data
        bool m_bRollAlign;
        RE_ADD_PADDING(3);
    }; // Verified size 0x14
    RE_VERIFY_SIZE(Z2DOBJ, 0x14);
}