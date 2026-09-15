#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
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
