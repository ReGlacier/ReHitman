#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SCompiledGeom
    {
        // methods
        SCompiledGeom();

        bool CanExtraGeomBeZero() const;

        // members
        uint32_t lOffsetName;
        uint32_t lOffsetMat;
        uint32_t lOffsetPos;
        uint32_t lPrim;
        uint32_t iColiId;
        uint32_t lGeomType;
        uint32_t lEvents;
        uint32_t lGeomAndGroupCon;
        uint32_t lExData;
        uint32_t lDataBlock;
        uint32_t lWorldFileId;
        uint32_t lScrits;
        uint32_t m_iGeomNr;
        uint8_t cLightCon;
        uint8_t cAssumedGeomDrawList;
        uint8_t cDummy[2];
    };
    RE_VERIFY_SIZE(SCompiledGeom, 0x38);
}
