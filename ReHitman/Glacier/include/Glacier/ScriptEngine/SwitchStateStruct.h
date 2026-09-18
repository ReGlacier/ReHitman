#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <Glacier/ScriptEngine/LocalVarEntry.h>
#include <cstdint>


namespace Glacier
{
    struct _SwitchStateStruct
    {
        LocalVarEntry m_LVE;
        const STATECONTROLLER** m_pEnters;
        short unsigned int m_lNumEnters;
        short unsigned int m_lNumExits;
        const STATECONTROLLER*  stateController;
        const STATECONTROLLER*  pSS_pOldStateController;
    };
    RE_VERIFY_SIZE(_SwitchStateStruct, 0x24);

    using SwitchStateStruct = _SwitchStateStruct;
}