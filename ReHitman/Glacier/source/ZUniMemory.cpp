#include <Glacier/ZUniMemory.h>
#include <cstdlib>
#include <cstring>


void* ZUniMemory::Allocate(int bytes)
{
    return Allocate(bytes, Glacier::EAllocType::DEFAULT_MEM);
}

void* ZUniMemory::Allocate(int bytes, Glacier::EAllocType eAllocType)
{
    auto ptr = std::malloc(bytes);
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

    std::free(ptr);
}
