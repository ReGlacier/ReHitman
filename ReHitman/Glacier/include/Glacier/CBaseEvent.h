#pragma once

#include <Glacier/ZBaseConRout.h>

namespace Glacier
{
    template<typename T>
    struct CBaseEvent : public ZBaseConRout
    {
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
