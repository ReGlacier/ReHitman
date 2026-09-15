#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Data/SCompiledGeom.h>


namespace Glacier
{
    struct SCompiledGeomEditor : public SCompiledGeom
    {
        ZGROUP* pInsertDest;
    };
}
