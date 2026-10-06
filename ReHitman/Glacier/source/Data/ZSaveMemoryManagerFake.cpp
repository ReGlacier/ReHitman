#include <Glacier/Data/ZSaveMemoryManagerFake.h>
#include <Glacier/ZUniMemory.h>

namespace Glacier
{
    // PC (0x45DCC0) routes allocations to the global allocator (ZSysMem::allocate)
    // rather than a save-specific arena; FreeMemory (0x45DCE0) just frees the block.
    void* ZSaveMemoryManagerFake::AllocMemory(int lSize)
    {
        return ZUniMemory::Allocate(lSize);
    }

    void ZSaveMemoryManagerFake::FreeMemory(void* ptr)
    {
        ZUniMemory::Free(ptr);
    }
}
