#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZSTDOBJ.h>
#include <cstdint>


namespace Glacier
{
    class ZSTDOBJPRIO : public ZSTDOBJ
    {
    public:
        // vtbl
        // data
        uint8_t m_iDrawPriority;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(ZSTDOBJPRIO, 0x14);
}