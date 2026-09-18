#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimHeaderStrip.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimHeaderStripBones : SPrimHeaderStrip
    {
        uint16_t NumBonesUsedLOD[8];
        uint32_t lBoneConstraintsHeader;
    };
    RE_VERIFY_SIZE(SPrimHeaderStripBones, 0x4C);
}