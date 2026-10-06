#include <Glacier/ZSTL/CMemPool.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    CMemPool::~CMemPool()
    {
        DeInit();
    }

    void CMemPool::Init(int iBlockSize, int iEntries, int iAlign)
    {
        m_iEntries = iEntries;
        m_iNumAlloc = 0;
        
        m_iBlockSize = ((iAlign - iBlockSize) & (iAlign - 1)) + iBlockSize;
        const auto iAllocTableSize = ((m_iBlockSize * iEntries) & 1) + m_iBlockSize * iEntries;

        m_pxPool = ZUniMemory::Allocate(iAllocTableSize + 2 * m_iEntries);
        m_bOwnPool = true;

        m_pAllocTable = (uint16_t*)((uint8_t*)m_pxPool + iAllocTableSize);
        
        if (m_iEntries > 0)
        {
            for (int i = 0; i < m_iEntries; ++i)
            {
                m_pAllocTable[i] = static_cast<uint16_t>(i);
            }
        }
    }
    
    void CMemPool::Init(int iBlockSize, int iEntries, void *pPool, int iAlign)
    {
        m_pxPool = pPool;
        m_iEntries = iEntries;
        m_iNumAlloc = 0;
        
        m_iBlockSize = ((iAlign - iBlockSize) & (iAlign - 1)) + iBlockSize;
        
        m_pAllocTable = (uint16_t*)ZUniMemory::Allocate(2 * m_iEntries); 
        m_bOwnPool = false;

        if (m_iEntries > 0)
        {
            for (int i = 0; i < m_iEntries; ++i)
            {
                m_pAllocTable[i] = static_cast<uint16_t>(i);
            }
        }
    }
    
    void* CMemPool::Alloc()
    {
        if (m_iNumAlloc == m_iEntries)
        {
            return nullptr;
        }

        int iFreeIndex = m_pAllocTable[m_iNumAlloc];
        ++m_iNumAlloc;

        return (uint8_t*)m_pxPool + (iFreeIndex * m_iBlockSize);
    }
    
    void* CMemPool::Alloc(int* iBlock)
    {
        if (iBlock == nullptr)
        {
            return nullptr;
        }

        if (m_iNumAlloc == m_iEntries)
        {
            *iBlock = -1;
            return nullptr;
        }

        int index = m_pAllocTable[m_iNumAlloc];
        *iBlock = index;

        ++m_iNumAlloc;
        return (uint8_t*)m_pxPool + (index * m_iBlockSize);
    }
    
    void CMemPool::DeAlloc(void* pxBlock)
    {
        if (pxBlock == nullptr || m_pxPool == nullptr)
        {
            return;
        }

        ZASSERT(m_iBlockSize > 0);
        --m_iNumAlloc;

        auto byteOffset = (uint8_t*)pxBlock - (uint8_t*)m_pxPool;

        uint16_t iReleasedIndex = static_cast<uint16_t>(byteOffset / m_iBlockSize);
        m_pAllocTable[m_iNumAlloc] = iReleasedIndex;
    }

    void CMemPool::DeInit()
    {
        void* pMemoryToFree = m_bOwnPool ? m_pxPool : m_pAllocTable;

        if (pMemoryToFree != nullptr)
        {
            ZUniMemory::Free(pMemoryToFree);
        }

        m_pxPool = nullptr;
        m_pAllocTable = nullptr;
        m_iNumAlloc = 0;
        m_iEntries = 0;
    }

    void* CMemPool::GetPtrFromBlockNum(int iBlock) const
    {
        ZASSERT(iBlock >= 0 && iBlock < m_iEntries);
        return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(m_pxPool) + (iBlock * m_iBlockSize));
    }

    bool CMemPool::IsFull() const
    {
        return m_iNumAlloc == m_iEntries;
    }
}