#pragma once

#include <Glacier/ScriptEngine/ZScriptC_ZMessage.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    struct _ScriptState;
    class ZScheduledScript;

    // types
    using UniquesArray_t = const char*[806];
    using ZScriptC_ZMessagesArray_t = ZScriptC_ZMessage[643];


    // decls

    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lCurrentScriptMaxTime);
    STATIC_GLOBAL_CLASS_INSTANCE(uint8_t*, g_pZScriptCDataBlock);
    STATIC_GLOBAL_CLASS_INSTANCE(ZScheduledScript*, g_pZSC);
    STATIC_GLOBAL_CLASS_INSTANCE(_ScriptState*, ISF);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, lScriptLoadedCount);
    STATIC_GLOBAL_CLASS_INSTANCE(uint8_t*, ScriptsPtr);
    STATIC_GLOBAL_CLASS_INSTANCE(uint8_t*, g_pScripts);
    STATIC_GLOBAL_CLASS_INSTANCE(ZScriptC_ZMessagesArray_t, ZScriptC_ZMessages);
    STATIC_GLOBAL_CLASS_INSTANCE(ZScriptC_ZMessage*, g_pZScriptC_Messages);
    STATIC_GLOBAL_CLASS_INSTANCE(UniquesArray_t, Uniques);
    STATIC_GLOBAL_CLASS_INSTANCE(const char**, g_pZScriptC_Uniques);
}