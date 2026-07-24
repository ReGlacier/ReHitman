#pragma once

#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    struct _ScriptState;
    class ZScheduledScript;

    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lCurrentScriptMaxTime);
    STATIC_GLOBAL_CLASS_INSTANCE(uint8_t*, g_pZScriptCDataBlock);
    STATIC_GLOBAL_CLASS_INSTANCE(ZScheduledScript*, g_pZSC);
    STATIC_GLOBAL_CLASS_INSTANCE(_ScriptState*, ISF);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, lScriptLoadedCount);
    STATIC_GLOBAL_CLASS_INSTANCE(uint8_t*, ScriptsPtr);
    STATIC_GLOBAL_CLASS_INSTANCE(uint8_t*, g_pScripts);
}