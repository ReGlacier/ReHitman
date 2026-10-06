#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimPlanes
    {
        float Plane0[4];
        float Plane1[4];
    };
    RE_VERIFY_SIZE(SPrimPlanes, 0x20);
}