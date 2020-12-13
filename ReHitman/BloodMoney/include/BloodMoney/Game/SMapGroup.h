#pragma once

#include <Glacier/GlacierFWD.h>

namespace Hitman::BloodMoney
{
    struct SMapGroup
    {
        int             m_unk0;     //0x0000
        Glacier::ZREF   m_ROOM_REF; //0x0004 (ZGROUP or derived from ZGROUP)
        char*           m_name;     //0x0008
    }; //Size is 0xC
}