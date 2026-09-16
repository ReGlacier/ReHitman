#include <Glacier/ZUniMemory.h>
#include <cstdlib>
#include <cstring>

#if defined(REHITMAN_STANDALONE) || defined(REHITMAN_TESTS) // Only for separated testing without game instance
#   define USE_STL_ALLOCATOR
#else
#   include <Glacier/System/ZSysMem.h>
#   define USE_GLACIER_ALLOCATOR
#endif


namespace
{
#ifdef USE_GLACIER_ALLOCATOR
    // Fallback bookkeeping used while the game's memory system is not up yet. This happens
    // during static (dynamic) initialization of this DLL: the game's ISysMem singleton is
    // only valid once the host process constructed it, which may be after our static
    // initializers ran. Anything allocated through ZUniMemory before that point is served
    // from the C heap and remembered here so the matching Free() does not hand it back to
    // the game allocator.
    constexpr int MAX_EARLY_HEAP_POINTERS = 256;
    void* g_aEarlyHeapPointers[MAX_EARLY_HEAP_POINTERS] = {};
    int g_lEarlyHeapPointerCount = 0;

    // The ISysMem singleton is a game global (see ZSysMem.cpp). Before the host constructs it,
    // its vtable slot is still null, which is the readiness signal we rely on.
    bool IsGlacierMemoryReady()
    {
        return Glacier::ZSysMem::m_pInstance != nullptr
            && *reinterpret_cast<void* const*>(Glacier::ZSysMem::m_pInstance) != nullptr;
    }

    bool IsEarlyHeapPointer(void* ptr)
    {
        for (int i = 0; i < g_lEarlyHeapPointerCount; ++i)
        {
            if (g_aEarlyHeapPointers[i] == ptr)
            {
                g_aEarlyHeapPointers[i] = g_aEarlyHeapPointers[--g_lEarlyHeapPointerCount];
                return true;
            }
        }

        return false;
    }

    void RememberEarlyHeapPointer(void* ptr)
    {
        if (ptr && g_lEarlyHeapPointerCount < MAX_EARLY_HEAP_POINTERS)
        {
            g_aEarlyHeapPointers[g_lEarlyHeapPointerCount++] = ptr;
        }
    }
#endif
}


void* ZUniMemory::Allocate(int bytes)
{
    return Allocate(bytes, Glacier::EAllocType::DEFAULT_MEM);
}

void* ZUniMemory::Allocate(int bytes, Glacier::EAllocType eAllocType)
{
    void* ptr = nullptr;
#   ifdef USE_STL_ALLOCATOR
    ptr = std::malloc(bytes);
#   else
    if (IsGlacierMemoryReady())
    {
        ptr = Glacier::ZSysMem::m_pInstance->New(eAllocType, bytes);
    }
    else
    {
        ptr = std::malloc(bytes);
        RememberEarlyHeapPointer(ptr);
    }
#   endif

    // idk, maybe not need to use this? Idk)
    if (ptr)
    {
        std::memset(ptr, 0x0, bytes);
    }

    return ptr;
}

void ZUniMemory::Free(void* ptr)
{
    if (ptr == nullptr) 
        return;

#   ifdef USE_STL_ALLOCATOR
    std::free(ptr);
#   else
    if (IsEarlyHeapPointer(ptr) || !IsGlacierMemoryReady())
    {
        std::free(ptr);
    }
    else
    {
        Glacier::ZSysMem::m_pInstance->Delete(ptr);
    }
#   endif
}
