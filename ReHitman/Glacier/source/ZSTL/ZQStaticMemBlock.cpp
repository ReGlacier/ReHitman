#include <Glacier/ZSTL/ZQStaticMemBlock.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZQStaticMemBlock::~ZQStaticMemBlock()
    {
        if (m_pStart)
        {
            ZUniMemory::Free(m_pStart);
            m_pStart = nullptr;
        }

        ZUniMemory::Delete(m_pOffsetAlloc);
        m_pOffsetAlloc = nullptr;

        ZUniMemory::Delete(m_pStack);
        m_pStack = nullptr;
    }

	ZQStaticMemBlock::ZQStaticMemBlock(uint32_t lSize, uint32_t* pFreeList)
    {
        m_pStack = nullptr;
        m_pStart = nullptr;
        m_pOffsetAlloc = nullptr;
        m_lDynamicStart = 0;
        m_lBufferSize = lSize;

        if (lSize)
        {
            m_pStart = static_cast<char*>(ZUniMemory::Allocate(lSize));

            ZASSERT(pFreeList == nullptr);

            m_pOffsetAlloc = ZUniMemory::New<ZOffsetAlloc>(m_lDynamicStart, m_lBufferSize, 1024, nullptr, EAllocType::RENDERCPU_MEM);
        }
    }
	
    void* ZQStaticMemBlock::GetPtr(uint32_t lOffset)
    {
        return m_pStart + lOffset;
    }
		
    uint32_t ZQStaticMemBlock::GetSize() const
    {
        return m_lBufferSize;
    }
		
    void* ZQStaticMemBlock::GetBufferPtr()
    {
        return m_pStart;
    }
}
