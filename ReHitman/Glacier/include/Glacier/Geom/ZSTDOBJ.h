#pragma once

#include <Glacier/Geom/ZGEOM.h>

namespace Glacier
{
    class ZSTDOBJ : public ZGEOM
    { // no changes, same size, same vftable size (maybe something overridden)
    public:
        STATIC_CLASS_VAR(ZSTDOBJ, uint32_t, m_Id);
        STATIC_CLASS_VAR(ZSTDOBJ, uint32_t, m_Mask);
    };
}
