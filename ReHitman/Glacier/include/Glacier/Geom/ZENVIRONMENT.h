#pragma once

#include <Glacier/Geom/ZLIGHT.h>

namespace Glacier {
    class ZENVIRONMENT : public ZLIGHT {
    public:
        //vftable (no changes)
        //api
        void ToggleColor(bool value);
        void SetDiffuseColor(unsigned int c1, unsigned int c2);

        //data (total size is 0x38, base size is 0x20)
        int m_field20;
        int m_field24;
        int m_color1_1;
        int m_color1_2;
        int m_color2_1;
        int m_color2_2; //0x34
    };
}