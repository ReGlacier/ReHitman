#pragma once

#include <cstdint>


namespace Glacier
{
    enum eGlobalTreeType : uint32_t
    {
        GT_None = 0xFFFFFFFF,
        GT_StdObjs = 0x0,
        GT_Lights = 0x1,
        GT_Gates = 0x2,
        GT_Groups = 0x3,
        GT_TreeGroups = 0x4,
        GT_SIZE = 0x5,
    };
}