#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SVertexW4WintelDP3
    {
        ZVector3 p;
        ZVector3 w;
        uint8_t m[4];
        uint32_t n;
        uint32_t c;
        ZVector2 t;
        uint32_t T;
        uint32_t B;
    };
    RE_VERIFY_SIZE(SVertexW4WintelDP3, 0x34);
}