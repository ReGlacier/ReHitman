#pragma once

#include <Glacier/Geom/ZLIGHT.h>

namespace Glacier {
    class ZUSERLIGHT : public ZLIGHT {
    public:
        //vftable (no changes)
        //data (total size is 0x44, base size is 0x20)
        int m_field20;
        int m_field24;
        int m_field28;
        int m_field2C;
        int m_field30;
        int m_field34;
        int m_field38;
        int m_field3C;
        int m_field40;
    };
}