#include <Glacier/System/ZRX86AllocIf.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>


namespace Glacier
{
    ZRX86AllocIf::ZRX86AllocIf(uint32_t lMaxNumObjects)
        : m_lMaxNumObjects(lMaxNumObjects)
        , m_Allocator(0u, lMaxNumObjects, 0x1000, nullptr, EAllocType::DEFAULT_MEM)
    {
    }

    ZRX86AllocIf::~ZRX86AllocIf() = default;
}