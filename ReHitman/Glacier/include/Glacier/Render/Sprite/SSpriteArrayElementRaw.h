#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SSpriteArrayElementRaw
    {
        // methods
        void ResetVerticies()
        {
            vertex[0] = ZVector3(-.5f, .0f, -.5f);
            vertex[1] = ZVector3(-.5f, .0f,  .5f);
            vertex[2] = ZVector3( .5f, .0f,  .5f);
            vertex[3] = ZVector3( .5f, .0f, -.5f);
        }

        void ResetTextureCoordinates()
        {
            uv[0] = 0.0f;
            uv[1] = 1.0f;
            uv[2] = 1.0f;
            uv[3] = 1.0f;
            uv[4] = 1.0f;
            uv[5] = 0.0f;
            uv[6] = 0.0f;
            uv[7] = 0.0f;
        }

        void VerticesAlignLeftBottom()
        {
            vertex[0] = ZVector3(0.0f, 0.0f, 0.0f);
            vertex[1] = ZVector3(0.0f, 0.0f, 1.0f);
            vertex[2] = ZVector3(1.0f, 0.0f, 1.0f);
            vertex[3] = ZVector3(1.0f, 0.0f, 0.0f);
        }

        // members
        ZVector3 p;
        float uv[8];
        ZVector3 vertex[4];
        ZVector2 fScale;
        uint32_t lColor[4];
    };
}