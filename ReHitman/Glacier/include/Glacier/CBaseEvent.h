#pragma once

#include <Glacier/EventBase/ZBaseConRout.h>
#include <Glacier/ZUniAssert.h>

namespace Glacier
{
    template<typename T>
    struct CBaseEvent : public ZBaseConRout
    {
        // vtbl
        // methods
        CBaseEvent() = default;

        ZGEOM* GetGeom()
        {
            ZASSERT(m_pBaseGeom);
            return m_pBaseGeom;
        }
    };
}
