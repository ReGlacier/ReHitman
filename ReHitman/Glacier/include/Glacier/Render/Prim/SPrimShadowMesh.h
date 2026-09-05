#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Render/Prim/SPrimStrips.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimShadowMesh : SPrimStrips
    {
        uint32_t lNumTriangles;
        bool bIsLerp;
        RE_ADD_PADDING(3);
        ZVector3 vCen;
        ZVector3 vSize;
        int32_t iSizeOfVifStream;
        uint32_t uVifStreamPrim;
    };
    RE_VERIFY_SIZE(SPrimShadowMesh, 0x60);
}