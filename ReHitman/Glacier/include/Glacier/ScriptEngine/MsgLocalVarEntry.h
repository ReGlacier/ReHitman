#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/LocalVarEntry.h>


namespace Glacier
{
    struct _MsgLocalVarEntry
    {
        LocalVarEntry m_LVE;
    };
    RE_VERIFY_SIZE(_MsgLocalVarEntry, 0x14);

    using MsgLocalVarEntry = _MsgLocalVarEntry;
}