#include <Glacier/ZSTL/ZBlockAlloc.h>
#include <Glacier/ZUniMemory.h>
#include <new>


namespace Glacier
{
    ZBlockAlloc::~ZBlockAlloc()
    {
        if (m_pBlockMem)
        {
            ZUniMemory::Free(m_pBlockMem);
            m_pBlockMem = nullptr;
        }

        if (m_pFreeMemTab)
        {
            ZUniMemory::Free(m_pFreeMemTab);
            m_pFreeMemTab = nullptr;
        }
    }

    void ZBlockAlloc::Create(int lNumBlocks, int lBlockSize)
    {
        if (m_pBlockMem) 
        {
            ZUniMemory::Free(m_pBlockMem);
            m_pBlockMem = nullptr;
        }
        
        m_pBlockMemAligned = nullptr;
        
        if (m_pFreeMemTab)
        {
            ZUniMemory::Free(m_pFreeMemTab);
            m_pFreeMemTab = nullptr;
        }

        m_lNumAllocated = 0;
        m_lNextFree = 0;

        if (lNumBlocks <= 0 || lBlockSize <= 0)
            return;

        m_lNumBlocks = lNumBlocks;
        m_lBlockSize = lBlockSize;

        size_t totalDataSize = static_cast<size_t>(lNumBlocks) * lBlockSize + m_lAlignment - 1;
        m_pBlockMem = static_cast<char*>(ZUniMemory::Allocate(static_cast<int>(totalDataSize)));
        
        size_t alignmentMask = static_cast<size_t>(m_lAlignment);
        size_t rawAddr = reinterpret_cast<size_t>(m_pBlockMem);
        size_t alignedAddr = (rawAddr + alignmentMask - 1) & ~(alignmentMask - 1);
        m_pBlockMemAligned = reinterpret_cast<char*>(alignedAddr);

        size_t tabSize = static_cast<size_t>(lNumBlocks) * sizeof(int);
        m_pFreeMemTab = static_cast<int*>(ZUniMemory::Allocate(static_cast<int>(tabSize)));

        int lastIdx = lNumBlocks - 1;
        for (int i = 0; i < lastIdx; ++i)
        {
            m_pFreeMemTab[i] = i + 1;
        }
        
        if (lNumBlocks > 0)
        {
            m_pFreeMemTab[lastIdx] = -1;
        }
    }

    void* ZBlockAlloc::AllocBlocks(int lNumBlocks)
    {
        if (lNumBlocks > (m_lNumBlocks - m_lNumAllocated))
            return nullptr;

        if (m_lNumAllocated == m_lNumBlocks)
            return nullptr;

        int firstAllocatedIdx = m_lNextFree;
        int currentIdx = firstAllocatedIdx;
        
        m_lNextFree = m_pFreeMemTab[currentIdx];
        m_pFreeMemTab[currentIdx] = -1; 
        ++m_lNumAllocated;

        if (lNumBlocks - 1 > 0)
        {
            int blocksToAlloc = lNumBlocks - 1;
            do
            {
                int nextAllocatedIdx = -1;
                if (m_lNumAllocated != m_lNumBlocks)
                {
                    nextAllocatedIdx = m_lNextFree;
                    m_lNextFree = m_pFreeMemTab[nextAllocatedIdx];
                    m_pFreeMemTab[nextAllocatedIdx] = -1;
                    ++m_lNumAllocated;
                }
                
                m_pFreeMemTab[currentIdx] = nextAllocatedIdx;
                currentIdx = nextAllocatedIdx;
                
                --blocksToAlloc;
            } 
            while (blocksToAlloc > 0);
        }

        return m_pBlockMemAligned + (firstAllocatedIdx * m_lBlockSize);
    }
}
