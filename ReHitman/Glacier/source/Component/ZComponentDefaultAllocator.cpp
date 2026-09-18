#include <Glacier/Component/ZComponentDefaultAllocator.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    void* ZComponentDefaultAllocator::Alloc(size_t size)
    {
        return ZUniMemory::Allocate(size);
    }

    void ZComponentDefaultAllocator::Free(void* ptr)
    {
        ZUniMemory::Free(ptr);
    }
}