#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SVertexStaticShadowWintel
    {
        ZVector3 p;
        uint32_t c;
    };
    RE_VERIFY_SIZE(SVertexStaticShadowWintel, 0x10);
}