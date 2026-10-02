#include <Glacier/ZUniMemory.h>
#include <Glacier/System/ZSysMem.h>
#include <cstdlib>
#include <cstring>


void* ZUniMemory::Allocate(int bytes)
{
    return Allocate(bytes, Glacier::EAllocType::DEFAULT_MEM);
}

void* ZUniMemory::Allocate(int bytes, Glacier::EAllocType eAllocType)
{
    // In the game env the whole engine funnels through ZSysMem (there is no global
    // operator new/delete override), so prefer it whenever it is up and running.
    if (Glacier::ISysMem::Exists())
        return Glacier::ISysMem::Instance().New(eAllocType, bytes);

    // Test env fallback.
    auto ptr = std::malloc(bytes);
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

    if (Glacier::ISysMem::Exists())
    {
        Glacier::ISysMem::Instance().Delete(ptr);
        return;
    }

    std::free(ptr);
}
