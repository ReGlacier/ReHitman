#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimMeshWeighted.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimMeshWeightedFur : SPrimMeshWeighted
    {
        int8_t lShellCount;
        float fShellDistance;
        bool bRenderShells;
        float fFinLength;
        bool bRenderFins;
    };
    RE_VERIFY_SIZE(SPrimMeshWeightedFur, 0x54);
}