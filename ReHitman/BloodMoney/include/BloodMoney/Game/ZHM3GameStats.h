#pragma once

#include <Glacier/ZGameStats.h>

namespace Hitman::BloodMoney {
    class ZHM3GameStats : public Glacier::ZGameStats {
    public:
        //vftable (no changes)
        //data (total size is 0x14, base size is 0xC)
        int m_fieldC;
        int m_field10;
    };
}