#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SVertexWintel
    {
        ZVector3 p;
        ZVector3 n;
        uint32_t c;
        ZVector2 t;
    };
    RE_VERIFY_SIZE(SVertexWintel, 0x24);
}