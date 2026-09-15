#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct _SCRIPTIMPORTS
    {
        uint16_t m_SIT : 3;
        uint16_t m_lAmount : 13;
    };
    RE_VERIFY_SIZE(_SCRIPTIMPORTS, 0x2);

    using SCRIPTIMPORT = _SCRIPTIMPORTS;
}