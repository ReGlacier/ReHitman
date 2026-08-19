#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/STempStripsUniqueId.h>


namespace Glacier
{
    struct ZRawStrip
    {
        // members
        uint32_t m_lLength;
        uint32_t m_lTriangleStartNr;
        STempStripsUniqueId m_HitCache;
        ZBaseGeom* m_pBaseGeom;
    };
    RE_VERIFY_SIZE(ZRawStrip, 0x18);
}
