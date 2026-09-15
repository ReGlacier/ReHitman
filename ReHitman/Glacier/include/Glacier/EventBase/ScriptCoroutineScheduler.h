#pragma once

#include <Glacier/ZUniMemory.h>
#include <cstdint>


extern "C" 
{
    /**
     * @brief Enter into scheduler mode.
     *        For x86 (32bit): save coroutine state and prepare CPU execution flow to yield. See ScriptCoroutineScheduler.asm for details
     *        For other platforms: not implemented
     */
    void __cdecl EnterSchedulerMode();
    void __cdecl ScriptDeallocate(void* ptr);
}

using GlacierCoroFunction_t = void(*)();

STATIC_GLOBAL_CLASS_INSTANCE(uintptr_t, g_pSchedulerSavedESP);
STATIC_GLOBAL_CLASS_INSTANCE(void*,     g_pStoredStack);
STATIC_GLOBAL_CLASS_INSTANCE(uint32_t,  g_lStoredStackLength);
STATIC_GLOBAL_CLASS_INSTANCE(void*,     g_pNewStackFrame);
STATIC_GLOBAL_CLASS_INSTANCE(GlacierCoroFunction_t, g_pFunction);