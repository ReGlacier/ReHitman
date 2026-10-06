#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SGeomTypeCount
    {
        uint32_t m_lGeomType;
        uint32_t m_lGeomCount;
        uint32_t m_lNoNeedExtraGeom;
    };
    RE_VERIFY_SIZE(SGeomTypeCount, 0xC);
}
