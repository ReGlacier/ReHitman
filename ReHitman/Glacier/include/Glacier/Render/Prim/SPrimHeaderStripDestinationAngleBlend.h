#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimHeaderStrip.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimHeaderStripDestinationAngleBlend : SPrimHeaderStrip
    {
        float m_fAngle1;
        float m_fOpacity1;
        float m_fAngle2;
        float m_fOpacity2;
        float m_fNear1;
        float m_fNear2;
        float m_fFar1;
        float m_fFar2;
    };
    RE_VERIFY_SIZE(SPrimHeaderStripDestinationAngleBlend, 0x58);
}