#include <Glacier/ScriptEngine/Globals.h>


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint32_t, g_lCurrentScriptMaxTime, 0x008289B0, 0);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint8_t*, g_pZScriptCDataBlock, 0x009A2670, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZScheduledScript*, g_pZSC, 0x009A2664, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(_ScriptState*, ISF, 0x0, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint32_t, lScriptLoadedCount, 0x008A89D8, 0u);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint8_t*, ScriptsPtr, 0x008A89D4, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint8_t*, g_pScripts, 0x008289B8, nullptr);
}