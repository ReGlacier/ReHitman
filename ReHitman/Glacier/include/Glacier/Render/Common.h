#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct _GLC_RENDER_RESOLUTION
    {
        uint32_t lWidth { 0 };
        uint32_t lHeight { 0 };
        uint32_t lBpp { 0 };
        uint32_t lUnknownC { 0 };
        uint32_t bFullscreen { 0 };
    };
}