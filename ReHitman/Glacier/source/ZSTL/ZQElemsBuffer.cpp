#include <Glacier/ZSTL/ZQElemsBuffer.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    void* ZQElemsBuffer::Add(uint32_t lSize, bool bLargeUseHigh)
    {
        ZASSERT((lSize & 3) == 0);
        ZASSERT(m_pOffsetAlloc != nullptr);

        void* pOffset = m_pOffsetAlloc->Alloc(lSize, bLargeUseHigh);
        ZASSERT(pOffset != reinterpret_cast<void*>(-1));

        return pOffset;
    }

    void ZQElemsBuffer::Remove(int lOffset, uint32_t lSize)
    {
        ZASSERT((lSize & 3) == 0);
        ZASSERT(lOffset >= static_cast<int>(m_lDynamicStart));
        ZASSERT(lOffset < static_cast<int>(GetSize()));
        ZASSERT(m_pOffsetAlloc != nullptr);

        m_pOffsetAlloc->Free(static_cast<uint32_t>(lOffset), lSize);
    }

    void ZQElemsBuffer::Remove(void* pMemory, uint32_t lSize)
    {
        Remove(static_cast<int>(static_cast<char*>(pMemory) - m_pStart), lSize);
    }
}
