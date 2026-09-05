#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    struct SFastBoxColiTri
    {
        ZREF m_rGeom;
        ZVector3 m_vTriNorm;
        ZMat3x3 m_mTri;
        ZVector3 m_avVerts[3];
        ZVector3 m_avEdgePerps[3];
    };
    RE_VERIFY_SIZE(SFastBoxColiTri, 0x7C);
}
