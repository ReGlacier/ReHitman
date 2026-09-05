#include <Glacier/Component/ZGlobalComponentAllocator.h>
#include <cstdlib>


namespace Glacier
{
    void* ZGlobalComponentAllocator::Alloc(size_t lSize)
    {
        // Like in original code
        return malloc(lSize);
    }

    void ZGlobalComponentAllocator::Free(void* ptr)
    {
        free(ptr);
    }
}