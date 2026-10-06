#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimHeaderStrip
    {
        float Plane0[4];
        float Plane1[4];
        ZVector3 vMax;
        ZVector3 vMin;
    };
    RE_VERIFY_SIZE(SPrimHeaderStrip, 0x38);
}