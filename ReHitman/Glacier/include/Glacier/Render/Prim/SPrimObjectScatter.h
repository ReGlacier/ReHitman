#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimScatterDistributionType
    {
        uint32_t lCount[2];
        uint32_t lPayloadOffset;
        uint32_t lUnknown0C;
    };

    struct SPrimScatterDistributionItem
    {
        float fBarycentricX;
        float fBarycentricY;
        float fUnknown08;
        float fAlpha;
    };

    struct SPrimScatterTile
    {
        uint8_t lDistributionType;
        uint8_t lColor[4];
        RE_ADD_PADDING(3);
        ZVector3 vPosition;
    };

    struct SPrimObjectScatter : SPrimObject
    {
      uint8_t lTileX;
      uint8_t lTileZ;
      uint8_t lDensity;
      uint8_t lBillboardTypes;
      uint32_t lMasterPrim;
      uint32_t lMasterSize;
      uint32_t lDistributionPrim;
      uint32_t lDistributionSize;
      float fTileSize;
      ZVector3 vCen;
      ZVector3 vSize;
      float fDrawDist;
      ZVector3 vColA;
      ZVector3 vColB;
      uint8_t lMeshType;

    };

    struct SPrimObjectScatterPayload
    {
        SPrimObjectScatter sScatter;
        RE_ADD_PADDING(8);
        SPrimScatterTile aTiles[1];
    };
    RE_VERIFY_SIZE(SPrimScatterDistributionType, 0x10);
    RE_VERIFY_SIZE(SPrimScatterDistributionItem, 0x10);
    RE_VERIFY_SIZE(SPrimScatterTile, 0x14);
    RE_VERIFY_OFFSET(SPrimObjectScatterPayload, aTiles, 0x80);
    RE_VERIFY_OFFSET(SPrimObjectScatter, lMasterPrim, 0x2C);
    RE_VERIFY_OFFSET(SPrimObjectScatter, lDistributionPrim, 0x34);
    RE_VERIFY_OFFSET(SPrimObjectScatter, fTileSize, 0x3C);
    RE_VERIFY_OFFSET(SPrimObjectScatter, vCen, 0x40);
    RE_VERIFY_OFFSET(SPrimObjectScatter, vSize, 0x4C);
    RE_VERIFY_OFFSET(SPrimObjectScatter, fDrawDist, 0x58);
    RE_VERIFY_OFFSET(SPrimObjectScatter, vColA, 0x5C);
    RE_VERIFY_OFFSET(SPrimObjectScatter, vColB, 0x68);
    RE_VERIFY_OFFSET(SPrimObjectScatter, lMeshType, 0x74);
    RE_VERIFY_SIZE(SPrimObjectScatter, 0x78);
}
