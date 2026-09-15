#include <Glacier/EventBase/ScriptCoroutineScheduler.h>
#include <Glacier/ZUniMemory.h>

// Decls
STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uintptr_t, g_pSchedulerSavedESP, 0x008BE10C, 0);
STATIC_GLOBAL_CLASS_INSTANCE_IMPL(void*,     g_pStoredStack,       0x008BE110, nullptr);
STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint32_t,  g_lStoredStackLength, 0x008BE114, 0);
STATIC_GLOBAL_CLASS_INSTANCE_IMPL(void*,     g_pNewStackFrame,     0x008BA104, nullptr);
STATIC_GLOBAL_CLASS_INSTANCE_IMPL(GlacierCoroFunction_t, g_pFunction, 0x008BE108, nullptr);

extern "C"
{
    uintptr_t* g_pSchedulerSavedESPPtr = &g_pSchedulerSavedESP;
    void** g_pStoredStackPtr = &g_pStoredStack;
    uint32_t* g_lStoredStackLengthPtr = &g_lStoredStackLength;
    void** g_pNewStackFramePtr = &g_pNewStackFrame;
    GlacierCoroFunction_t* g_pFunctionPtr = &g_pFunction;
}

// dealloc trampoline
extern "C" void __cdecl ScriptDeallocate(void* ptr)
{
    if (!ptr) return;

    ZUniMemory::Free(ptr);
}
