#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SVertexW4Wintel
    {
        ZVector3 p;
        ZVector3 w;
        uint8_t m[4];
        ZVector3 n;
        uint32_t c;
        ZVector2 t;
    };
    RE_VERIFY_SIZE(SVertexW4Wintel, 0x34);
}