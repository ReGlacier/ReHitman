#include <Glacier/ZUniMemory.h>
#include <cstdlib>
#include <cstring>

#if defined(REHITMAN_STANDALONE) || defined(REHITMAN_TESTS) // Only for separated testing without game instance
#   define USE_STL_ALLOCATOR
#else
#   include <Glacier/ZSysMem.h>
#   define USE_GLACIER_ALLOCATOR
#endif


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
    ptr = Glacier::ZSysMem::m_pInstance->New(eAllocType, bytes);
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
    Glacier::ZSysMem::m_pInstance->Delete(ptr);
#   endif
}
