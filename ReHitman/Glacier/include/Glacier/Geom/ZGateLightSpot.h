#pragma once

#include <Glacier/Geom/ZSPOTLIGHT.h>

namespace Glacier {
    class ZGateLightSpot : public ZSPOTLIGHT {
    public:
        //vftable (no changes)
        //data (total size is 0x60, base size is 0x20)
        int m_field20;
        int m_field24;
        int m_field28;
        int m_field2C;
        int m_field30;
        int m_field34;
        int m_field38;
        int m_field3C;
        int m_field40;
        int m_field44;
        int m_field48;
        int m_field4C;
        int m_field50;
        int m_field54;
        int m_field58;
        int m_field5C;
    };
}