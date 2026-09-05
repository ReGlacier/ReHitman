#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <cstdint>


namespace Glacier
{
    using EntryPoint_t = float(*)(_ScriptState*); // DronCode: yep, I guess it's ok

    struct _FUNCTIONCONTROLLER 
    {
        EntryPoint_t m_pEntryPoint;
        uint16_t m_lInputSize;
        uint16_t m_lDataSize;
        const char* m_pName;
        uint16_t* m_lStringOffsets;
    };
    RE_VERIFY_SIZE(_FUNCTIONCONTROLLER, 0x10);

    using FUNCTIONCONTROLLER = _FUNCTIONCONTROLLER;
}
