#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Render/Prim/SPrims.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimObject : SPrims
    {
        // types
        enum SUBTYPE : uint32_t
        {
            SUBTYPE_STANDARD = 0x0,
            SUBTYPE_TWEENED  = 0x1,
            SUBTYPE_RIGID    = 0x2,
            SUBTYPE_WEIGHTED = 0x3,
            SUBTYPE_COUNT    = 0x4
        };

        enum PROPERTY_FLAGS : uint32_t
        {
            PROPERTY_XAXISLOCKED = 0x1,
            PROPERTY_YAXISLOCKED = 0x2,
            PROPERTY_ZAXISLOCKED = 0x4,
            PROPERTY_ISEDITABLE = 0x8,
        };

        struct STransformation
        {
            ZMat3x3 mTransform;
            ZVector3 vTransform;
        };
        RE_VERIFY_SIZE(STransformation, 0x30);

        // members
        uint8_t lSubType;
        uint8_t lProperties;
        uint8_t lLODMask;
        uint8_t lVariantId;
        uint8_t lNumInstances;
        RE_ADD_PADDING(1);
        uint16_t lMaterialId;
        uint32_t lColiBits;
        uint32_t lWireColor;
        uint32_t lDrawMode;
        uint32_t lTransformations;
        uint32_t lExtraData;
    };
    RE_VERIFY_SIZE(SPrimObject, 0x28);
}