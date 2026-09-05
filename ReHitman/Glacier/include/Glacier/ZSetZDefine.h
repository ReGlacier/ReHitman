#pragma once

#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>

namespace Glacier {
    class ZSetZDefine : public CBaseEvent<ZGEOM> {
    public:
        //vftable (no changes)
        //data (total size is 0x38, base size is 0x30)
        int m_field30;
        int m_field34;
    };
}