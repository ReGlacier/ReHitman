#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Render/Prim/SPrimHeaderStrip.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimHeaderStripMirror : SPrimHeaderStrip
    {
        ZVector3 vMax;
        float Plane0[4];
        float Plane1[4];
        ZVector3 vMin;
        uint32_t m_lFOGColor;
        float m_fFOGNear;
        float m_fFOGFar;
        bool m_bOnlyCurrentRoom;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_SIZE(SPrimHeaderStripMirror, 0x80);
}