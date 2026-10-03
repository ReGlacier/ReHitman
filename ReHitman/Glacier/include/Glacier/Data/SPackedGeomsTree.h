#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SPackedGeomsTree
    {
        uint32_t lCompiledGeomOffset;
        uint32_t lLightListID;
    };
    RE_VERIFY_SIZE(SPackedGeomsTree, 0x8);
}
