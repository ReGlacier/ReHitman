#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/Fwd.h>


namespace Glacier
{
    struct _INTERNALSCRIPTFUNCTIONS
    {
        _ScriptState* RunningThread;
    };
    RE_VERIFY_SIZE(_INTERNALSCRIPTFUNCTIONS, 4);

    using INTERNALSCRIPTFUNCTIONS = _INTERNALSCRIPTFUNCTIONS;
}