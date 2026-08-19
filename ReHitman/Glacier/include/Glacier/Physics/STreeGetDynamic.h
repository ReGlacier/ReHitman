#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Physics/eGlobalTreeType.h>
#include <Glacier/ZSTL/CQuadtree.h>
#include <cstdint>


namespace Glacier
{
    struct STreeGetDynamic : public SRecurseInfoCompiled
    {
        // constants
        static constexpr int MAX_MARKED_GEOMS_NR = 1024;
        
        // methods

        void ClearMarks();

        // members        
        ZBaseGeom** pGeomList;
        ZBaseGeom** pGeomListEnd;
        uint32_t lNrGeomsInList;
        uint32_t lGeomConMask;
        eGlobalTreeType eGTT;
        ZROOM* pRoom;
        ZBaseGeom* pMarkedGeoms[MAX_MARKED_GEOMS_NR];
        uint32_t lNrMarkedGeoms;
    };
    RE_VERIFY_SIZE(STreeGetDynamic, 0x1078);  // Confirmed KL2, HBM PS2
}