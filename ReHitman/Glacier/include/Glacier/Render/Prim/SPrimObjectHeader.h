#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrims.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimObjectHeader : SPrims
    {
        // types
        enum PROPERTY_FLAGS : uint32_t
        {
            HAS_BONES = 1,
            HAS_FRAMES = 2,
            IS_RIGID_OBJECT = 4,
            IS_WEIGHTED_OBJECT = 8,
            HAS_REFLECTION2D = 16,
            HAS_REFRACTION2D = 32,
            IS_TWOSIDED = 64,
            HAS_PARALLAX = 128,
            USE_BOUNDS = 256,
        };

        enum PROPERTY_INDEX : uint32_t
        {
            PROPERTY_NONE = 0,
            PROPERTY_BONES = 1,
            PROPERTY_REFLECTION2D = 2,
            MAX_PROPERTY_INDEX = 3,
        };


        // members
        uint32_t lPropertyFlags;
        uint32_t lPropertyData;
        uint32_t lNumObjects;
        uint32_t lObjectTable;
        uint32_t lColiId;
        ZVector3 vMin;
        ZVector3 vMax;
        uint32_t lPlanes;
    };
    RE_VERIFY_SIZE(SPrimObjectHeader, 0x3C);
}