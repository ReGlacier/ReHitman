#pragma once

#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/LINKREFTAB.h>

namespace Hitman::BloodMoney {
    class CEVENTCONTROLLER {
    public:
        //vftable: no vftable
        //data (total size is 0x18)
        Glacier::REFTAB* m_refTab0;
        Glacier::LINKREFTAB* m_linkRefTab4;
        int m_field8;
        int m_fieldC;
        int m_field10;
        int m_field14;
    };
}