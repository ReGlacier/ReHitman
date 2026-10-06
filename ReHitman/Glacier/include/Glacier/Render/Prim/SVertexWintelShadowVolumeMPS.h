#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SVertexWintelShadowVolumeMPS
    {
        ZVector3 p;
        ZVector3 w;
        uint8_t  m[4];
        ZVector3 n;
    };
    RE_VERIFY_SIZE(SVertexWintelShadowVolumeMPS, 0x28);
}