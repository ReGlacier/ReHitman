#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SVertexWintelShadowVolumeP
    {
        ZVector4 p;
    };
    RE_VERIFY_SIZE(SVertexWintelShadowVolumeP, 0x10);
}