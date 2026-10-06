#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SSpriteArrayElementUV
    {
        // methods
        void ResetTextureCoordinates()
        {
            u.x = 0.0f;
            u.y = 1.0f;
            v.x = 1.0f;
            v.y = 0.0f;
        }
        // members
        ZVector3 p;
        ZVector2 u;
        ZVector2 v;
        ZVector2 fScale;
        uint32_t lColor;
    };
}