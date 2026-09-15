#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    struct SD3DRenderVertex
    {
        ZVector3 p;
        ZVector3 n;
        ZVector2 t;
    };
    RE_VERIFY_SIZE(SD3DRenderVertex, 0x20);
}