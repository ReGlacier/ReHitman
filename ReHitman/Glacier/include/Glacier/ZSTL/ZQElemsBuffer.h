#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZQStaticMemBlock.h>
#include <cstdint>


namespace Glacier
{

    class ZQElemsBuffer : public ZQStaticMemBlock
    {
    public:
        using ZQStaticMemBlock::ZQStaticMemBlock;

        void* Add(uint32_t lSize, bool bLargeUseHigh);
        void Remove(int lOffset, uint32_t lSize);
        void Remove(void* pMemory, uint32_t lSize);
    };
    RE_VERIFY_SIZE(ZQElemsBuffer, 0x18);
}
