#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct _SAVEGAMESTATICS
    {
        uint32_t m_eType : 8;
        int32_t m_lSize : 24;
        void* m_pAddr;
    };
    RE_VERIFY_SIZE(_SAVEGAMESTATICS, 8);

    using SAVEGAMESTATICS = _SAVEGAMESTATICS;
}