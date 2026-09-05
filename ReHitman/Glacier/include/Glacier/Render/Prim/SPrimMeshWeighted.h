#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimMeshWeighted : SPrimMesh
    {
        uint32_t lNumCopyBones;
        uint32_t lCopyBones;
    };
    RE_VERIFY_SIZE(SPrimMeshWeighted, 0x40);
}