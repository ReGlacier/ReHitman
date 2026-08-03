#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <Glacier/ScriptEngine/INTERNALSCRIPTFUNCTIONS.h>
#include <Glacier/ScriptEngine/ZScriptC_ZMessage.h>
#include <Glacier/ScriptEngine/SaveRefEntry.h>
#include <Glacier/ZSTL/PrivateAllocator.h>
#include <Glacier/ZSTL/STLport.h>
#include <Glacier/ZSTL/ZMallocSimple.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    struct _SCRIPTFUNCTIONS;
    class ZScheduledScript;
    class ZScriptC;
    
    // types
    using UniquesArray_t = const char*[806];
    using ZScriptC_ZMessagesArray_t = ZScriptC_ZMessage[643];
    // Engine-side script interface table copied into the scene script DLL on
    // attach (AttachSceneScripts copies exactly 0x2CC entries). On PC it spans
    // [0x007F2D20, 0x007F3850) right up to the Uniques symbol.
    using ScriptInterfacesArray_t = void*[0x2CC];
    // Script heap pool: on PC AddBlock(g_ScriptMemory, 0x80000) registers the
    // whole region [0x8289C8, 0x8A89C8), so the buffer must span 0x80000 bytes
    // (the named .data symbol g_ScriptMemory[30264] is only its head).
    using ScriptMemoryBuffer_t = char[0x80000];

    // Old STLPort things
    using SaveTableMap = stlp::map<
        void*,
        uint32_t,
        stlp::less<void*>,
        PrivateAllocator<stlp::pair<void* const, uint32_t>, ISaveMemoryManager>
    >;

    using SaveTableVector = stlp::vector<SaveRefEntry, PrivateAllocator<SaveRefEntry, ISaveMemoryManager>>;

    // decls
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, g_lCurrentScriptMaxTime);
    STATIC_GLOBAL_CLASS_INSTANCE(uint8_t*, g_pZScriptCDataBlock);
    STATIC_GLOBAL_CLASS_INSTANCE(ZScheduledScript*, g_pZSC);
    STATIC_GLOBAL_CLASS_INSTANCE(_SCRIPTFUNCTIONS*, g_SF);
    STATIC_GLOBAL_CLASS_INSTANCE(INTERNALSCRIPTFUNCTIONS, ISF);
    STATIC_GLOBAL_CLASS_INSTANCE(uint32_t, lScriptLoadedCount);
    STATIC_GLOBAL_CLASS_INSTANCE(SCRIPTCREATOR**, ScriptsPtr);
    STATIC_GLOBAL_CLASS_INSTANCE(void*, g_pScripts);
    STATIC_GLOBAL_CLASS_INSTANCE(ZScriptC_ZMessagesArray_t, ZScriptC_ZMessages);
    STATIC_GLOBAL_CLASS_INSTANCE(ZScriptC_ZMessage*, g_pZScriptC_Messages);
    STATIC_GLOBAL_CLASS_INSTANCE(UniquesArray_t, Uniques);
    STATIC_GLOBAL_CLASS_INSTANCE(ScriptInterfacesArray_t, ScriptInterfaces);
    STATIC_GLOBAL_CLASS_INSTANCE(const char**, g_pZScriptC_Uniques);
    STATIC_GLOBAL_CLASS_INSTANCE(bool, g_bScriptLoadResult);
    STATIC_GLOBAL_CLASS_INSTANCE(ZMallocSimple, g_ScriptAllocator);
    STATIC_GLOBAL_CLASS_INSTANCE(ScriptMemoryBuffer_t, g_ScriptMemory);
    STATIC_GLOBAL_CLASS_INSTANCE(ZREF, s_CurrentRootScriptCRef);
    STATIC_GLOBAL_CLASS_INSTANCE(ZOffsetAlloc*, g_pMessageAllocator);
    STATIC_GLOBAL_CLASS_INSTANCE(ScriptState*, g_pCurrentSS);
    STATIC_GLOBAL_CLASS_INSTANCE(SaveRefEntry*, s_pLoadEntries);
    STATIC_GLOBAL_CLASS_INSTANCE(int, s_lObjectSaveCount);
    STATIC_GLOBAL_CLASS_INSTANCE(bool, s_bSaving);
    STATIC_GLOBAL_CLASS_INSTANCE(ZScriptC*, s_pCurrentSaveGameObject);
    STATIC_GLOBAL_CLASS_INSTANCE(SaveTableMap*, g_pSavedPointersMap);
    STATIC_GLOBAL_CLASS_INSTANCE(SaveTableVector*, g_pSaveTable);
}
