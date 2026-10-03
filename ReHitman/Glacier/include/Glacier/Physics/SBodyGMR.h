#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SBodyGMR
    {
        ZVector3 dir;
        ZVector3 target;
        float radius;
        bool grabbing;
        uint16_t par;
        float strained;
    };
    RE_VERIFY_SIZE(SBodyGMR, 0x24);
}
