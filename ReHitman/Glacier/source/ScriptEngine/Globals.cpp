#include <Glacier/ScriptEngine/Globals.h>


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint32_t, g_lCurrentScriptMaxTime, 0x008289B0, 0);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint8_t*, g_pZScriptCDataBlock, 0x009A2670, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZScheduledScript*, g_pZSC, 0x009A2664, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(INTERNALSCRIPTFUNCTIONS, ISF, 0x008289BC, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(_SCRIPTFUNCTIONS*, g_SF, 0x008289C0, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint32_t, lScriptLoadedCount, 0x008A89D8, 0u);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(SCRIPTCREATOR**, ScriptsPtr, 0x008A89D4, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(void*, g_pScripts, 0x008289B8, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZScriptC_ZMessagesArray_t, ZScriptC_ZMessages, 0x007596A0, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZScriptC_ZMessage*, g_pZScriptC_Messages, 0x008A89CC, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(UniquesArray_t, Uniques, 0x007F3850, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ScriptInterfacesArray_t, ScriptInterfaces, 0x007F2D20, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(const char**, g_pZScriptC_Uniques, 0x008A89C8, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(bool, g_bScriptLoadResult, 0x008ACA2C, false);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZMallocSimple, g_ScriptAllocator, 0x008A89E0, ZMallocSimple(0x800, 4));
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ScriptMemoryBuffer_t, g_ScriptMemory, 0x008289C8, {});
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZREF, s_CurrentRootScriptCRef, 0x009A2668, 0);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZOffsetAlloc*, g_pMessageAllocator, 0x008A89D0, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ScriptState*, g_pCurrentSS, 0x009A266C, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(SaveRefEntry*, s_pLoadEntries, 0x009A2688, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(int, s_lObjectSaveCount, 0x009A2684, 0);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(bool, s_bSaving, 0x009A2680, false);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(bool, s_bLoading, 0x009A2678, false);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(ZScriptC*, s_pCurrentSaveGameObject, 0x009A2658, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(SaveTableMap*, g_pSavedPointersMap, 0x009A265C, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(SaveTableVector*, g_pSaveTable, 0x009A2660, nullptr);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(StringMap*, s_pStringMap, 0x009A267C, nullptr);

    // The script heap pool must cover exactly [0x008289C8, 0x008A89C8):
    // AddBlock(g_ScriptMemory, 0x80000) on PC and the next named global
    // (g_pZScriptC_Uniques) sits right at the pool end.
    static_assert(sizeof(g_ScriptMemory) == 0x80000, "g_ScriptMemory must span the whole PC script heap pool");
}
