#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SSpriteArrayElement
    {
        ZVector3 p;
        float fScale;
        float fAngle;
        uint32_t lColor;
    };
    RE_VERIFY_SIZE(SSpriteArrayElement, 0x18);
}