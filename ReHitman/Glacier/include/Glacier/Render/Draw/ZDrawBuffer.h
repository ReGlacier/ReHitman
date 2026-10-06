#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZDrawBuffer
    {
    public:
        // vtbl
        virtual int GetSizeX() const = 0;
        virtual int GetSizeY() const = 0;
        virtual float ViewAspectXY() const = 0;
        virtual ZRenderEntry* DrawBonesArray(uint32_t, const SBonesArray*) = 0;
        virtual ZRenderEntry* DrawSpriteArray(uint32_t lPrim, const SSpriteArray* pSpriteArray, const float *m0, const float *p0, unsigned __int8 lPriority, bool bLocal, unsigned int lDrawMask) = 0;
    };
}