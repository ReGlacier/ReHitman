#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SSpriteArrayElementParticle
    {
        ZVector3 vC1;
        ZVector2 vC2;
        float fStartTime;
        float fEndTime;
        float fStartAngle;
        float fStartScale;
        uint8_t u8Flags;
        uint8_t u8Phase;
        uint8_t u8Extra1;
        uint8_t u8Extra2;
    }; // Not confirmed
}