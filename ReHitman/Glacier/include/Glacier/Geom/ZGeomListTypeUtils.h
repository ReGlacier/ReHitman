#pragma once

#include <Glacier/Geom/ZBaseGeom.h> // ZBaseGeom
#include <cstdint>


namespace Glacier
{
    enum eBaseGeomListTypes : int32_t
    {
        BGLT_Group = 0x0,
        BGLT_StdObj = 0x1,
        BGLT_Light = 0x2,
        BGLT_Other = 0x3,
        BGLT_Dummy5 = 0x4,
        BGLT_Dummy6 = 0x5,
        BGLT_Dummy7 = 0x6,
        BGLT_Dummy8 = 0x7,
        BGLT_SIZE = 0x8,
    };

    eBaseGeomListTypes GetBaseGeomListType(ZBaseGeom* pBaseGeom);
}
