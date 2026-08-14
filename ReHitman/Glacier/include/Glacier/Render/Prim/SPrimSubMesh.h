#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimSubMesh
    {
        uint32_t lNumVertices;
        uint32_t lVertices;
        uint32_t lNumIndices;
        uint32_t lIndices;
    };
    RE_VERIFY_SIZE(SPrimSubMesh, 0x10);
}