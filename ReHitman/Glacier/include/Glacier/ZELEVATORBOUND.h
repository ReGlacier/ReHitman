#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Geom/ZSTDOBJ.h>

namespace Glacier
{
    class ZELEVATORBOUND : public ZSTDOBJ
    {
    public:
        // vtable (not changed)

        // data (total size is 0x2C)
        Glacier::ZVector3 m_topPoint;
        Glacier::ZVector3 m_bottomPoint;
    };
}