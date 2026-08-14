#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Render/Prim/SPrimMesh.h>
#include <cstring>


namespace Glacier
{
    struct SPrimStaticShadowMesh : SPrimMesh
    {
        ZMat4x4 mProjectionMatrix;
        uint32_t lShadowColor;
    };
    RE_VERIFY_SIZE(SPrimStaticShadowMesh, 0x7C);
}