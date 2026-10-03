#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SVertexWintelDP3
    {
        ZVector3 p;
        uint32_t n;
        uint32_t c;
        ZVector2 t;
        uint32_t T;
        uint32_t B;
        uint32_t S;
    };
    RE_VERIFY_SIZE(SVertexWintelDP3, 0x28);
}