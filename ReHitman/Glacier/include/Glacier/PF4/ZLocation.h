#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>

namespace Glacier::PF4
{
    struct ZLocation
    {
        int16_t m_Component;
        int16_t m_Graph;
        uint8_t m_Inside;
        ZVector3 m_vPos;
    };
    RE_VERIFY_SIZE(ZLocation, 0x14);
}