#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    struct SPrimObjectHeaderReflection2DData
    {
        uint32_t m_lOnlyCurrentRoom;
        uint32_t m_lFogColor;
        float m_fFogNear;
        float m_fFogFar;
    };
    RE_VERIFY_SIZE(SPrimObjectHeaderReflection2DData, 0x10);
}