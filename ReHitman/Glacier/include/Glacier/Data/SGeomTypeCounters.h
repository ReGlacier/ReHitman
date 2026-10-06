#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SGeomTypeCounters
    {
        uint32_t m_lTypeCounters[24];
    };
    RE_VERIFY_SIZE(SGeomTypeCounters, 0x60);
}
