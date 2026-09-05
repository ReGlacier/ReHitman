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
    RE_VERIFY_SIZE(SPrimObjectScatter, 0x78);
}
