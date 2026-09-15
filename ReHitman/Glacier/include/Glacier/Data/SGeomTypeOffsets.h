#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>


namespace Glacier
{
    struct SGeomTypeOffsets
    {
        ZBaseGeom* GeomTypeBasePtrs[24];
    };
    RE_VERIFY_SIZE(SGeomTypeOffsets, 0x60);
}
