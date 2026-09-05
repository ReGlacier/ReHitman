#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/Fwd.h>


namespace Glacier
{
    struct ScriptStateInfo
    {
        ScriptState* m_pRootScriptState;
        ScriptState* m_pCurrentScriptState;
    };
    RE_VERIFY_SIZE(ScriptStateInfo, 0x8);
}