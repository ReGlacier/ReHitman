#include <Glacier/Render/ZRVertexContainer.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRVertexContainer::ZRVertexContainer() = default;

    uint32_t ZRVertexContainer::Create(uint32_t lNumVertices, uint32_t lVertexSize, ZRX86AllocIf* pAllocator, uint32_t lAlignment)
    {
        ZASSERT(!m_lNumVertices);
        if (!lNumVertices)
            return 0;

        m_lNumVertices = lNumVertices;
        m_pAllocator = pAllocator;
        m_lVertexSize = lVertexSize;

        uint32_t lAllocateSize = lAlignment + (lVertexSize * lNumVertices);
        m_lAllocateSize = lAllocateSize;

        uint32_t offset = (uint32_t)pAllocator->m_Allocator.Alloc(lAllocateSize, true);

        m_lAllocateOffset = offset;
        if (offset == (uint32_t)-1)
        {
            m_lAllocateOffset = 0;
            m_lAllocateSize = 0;
            m_lNumVertices = 0;
            return (uint32_t)-1;
        }

        uint32_t alignedOffset = offset;
        uint32_t result = offset;
        if (lAlignment != 0)
        {
            alignedOffset = lAlignment + offset - (offset % lAlignment);
            // PC quirk: the div used for the modulo leaves its quotient in eax,
            // so the original build returns offset / lAlignment here.
            result = offset / lAlignment;
        }

        m_lVertexOffset = alignedOffset;
        return result;
    }

    void ZRVertexContainer::Release()
    {
        if (m_lNumVertices != 0)
        {
            m_pAllocator->m_Allocator.Free(m_lAllocateOffset, m_lAllocateSize);
            m_lNumVertices = 0;
        }
    }
}