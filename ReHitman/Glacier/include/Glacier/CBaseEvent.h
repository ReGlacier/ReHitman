#pragma once

#include <Glacier/EventBase/ZBaseConRout.h>

namespace Glacier
{
    template<typename T>
    struct CBaseEvent : public ZBaseConRout
    {
        // vtbl
        // methods
        CBaseEvent() = default;

        // From PS2 Pre-alpha build
        ZGEOM* GetGeom() 
        {
            /*
            This method was found in PS2 pre-alpha build and it looks like
            ZASSERT(m_pBaseGeom);
            return m_pBaseGeom;
            */ 
            return m_pBaseGeom;
        }
    };
}
