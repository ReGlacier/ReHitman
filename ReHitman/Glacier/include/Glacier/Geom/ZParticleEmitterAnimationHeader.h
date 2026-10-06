#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    struct ZParticleEmitterAnimationHeader
    {
        int32_t iFrameStart;
        int32_t iFrameEnd; // Exclusive; packed StreamPacker samples follow this header.
    };
    RE_VERIFY_SIZE(ZParticleEmitterAnimationHeader, 0x8);
    RE_VERIFY_OFFSET(ZParticleEmitterAnimationHeader, iFrameStart, 0x0);
    RE_VERIFY_OFFSET(ZParticleEmitterAnimationHeader, iFrameEnd, 0x4);
}
