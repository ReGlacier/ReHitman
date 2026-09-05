#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimHeaderStrip.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimHeaderStripWobble : SPrimHeaderStrip
    {
        float m_fWobbleUSpeed;
        float m_fWobbleVSpeed;
        float m_fWobblePositionAmplitude;
        float m_fWobbleNormalAmplitude;
        float m_fWobblePeriod;
    };
    RE_VERIFY_SIZE(SPrimHeaderStripWobble, 0x4C);
}