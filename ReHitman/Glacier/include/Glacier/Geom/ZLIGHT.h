#pragma once

#include <Glacier/Geom/ZGEOM.h>

namespace Glacier {
    class ZLIGHT : public ZGEOM {
    public:
        //vftable
        virtual void Enable();
        virtual void Disable();
        virtual void SetMultiplier(float fValue); // In default implementation it's do nothing
        virtual void SetDiffuseColor(unsigned int rgba); // In default implementation it's do nothing
        virtual void MakeLightUnique();

        //data (total size is 0x20, base size is 0x10)
        int m_flags;
        int m_field14;
        int m_field18;
        int m_field1C;
    };
}