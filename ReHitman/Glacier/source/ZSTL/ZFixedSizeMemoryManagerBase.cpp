#include <Glacier/ZSTL/ZFixedSizeMemoryManager.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <new>


namespace Glacier
{
    ZFixedSizeMemoryManagerBase::ZFixedSizeMemoryManagerBase(int lBlockSize, uint32_t iNumberOfBlocks)
        : m_FreeListValid(false)
        , m_BlockSize(lBlockSize < 4 ? 4 : lBlockSize)
        , m_NumberOfBlocks(iNumberOfBlocks == 0 ? 1 : iNumberOfBlocks)
        , m_Buffer(nullptr)
        , m_ID(nullptr)
        , m_FirstFreeBlock(nullptr)
        , m_LastFreeBlock(nullptr)
    {
        size_t bufferSize = static_cast<size_t>(m_NumberOfBlocks) * m_BlockSize;
        m_Buffer = static_cast<char*>(ZUniMemory::Allocate(bufferSize));

        size_t idTableSize = static_cast<size_t>(m_NumberOfBlocks) * sizeof(int16_t);
        m_ID = static_cast<int16_t*>(ZUniMemory::Allocate(idTableSize));

        uint32_t i = m_NumberOfBlocks;
        while (i > 0)
        {
            --i;
            m_ID[i] = -1;
        }

        ValidateFreeList();
    }

    ZFixedSizeMemoryManagerBase::~ZFixedSizeMemoryManagerBase()
    {
        if (m_Buffer)
        {
            ZUniMemory::Free(m_Buffer);
            m_Buffer = nullptr;
        }

        if (m_ID)
        {
            ZUniMemory::Free(m_ID);
            m_ID = nullptr;
        }
    }

    void ZFixedSizeMemoryManagerBase::ValidateFreeList()
    {
        int index = static_cast<int>(m_NumberOfBlocks);

        while (true)
        {
            --index;
            if (index < 0)
            {
                m_LastFreeBlock = nullptr;
                m_FirstFreeBlock = nullptr;
                m_FreeListValid = true;
                return;
            }

            if (m_ID[index] < 0)
            {
                void* blockAddr = &m_Buffer[index * m_BlockSize];
                
                ZFreeBlock* pBlock = ::new(blockAddr) ZFreeBlock();

                m_FirstFreeBlock = pBlock;
                m_LastFreeBlock = pBlock;
                break;
            }
        }

        while (--index != -1)
        {
            if (m_ID[index] < 0)
            {
                void* blockAddr = &m_Buffer[index * m_BlockSize];
                
                ZFreeBlock* pBlock = ::new(blockAddr) ZFreeBlock(m_FirstFreeBlock);
                m_FirstFreeBlock = pBlock;
            }
        }

        m_FreeListValid = true;
    }

    void* ZFixedSizeMemoryManagerBase::REF2Ptr(uint32_t rRef) const
    {
        if (rRef == 0xFFFFFFFF) // -1
            return nullptr;

        uint32_t index = REF2Index(rRef);
        int16_t currentID = m_ID[index];

        if (currentID == REF2ID(rRef))
        {
            return &m_Buffer[index * m_BlockSize];
        }
        
        return nullptr;
    }

    uint32_t ZFixedSizeMemoryManagerBase::REF2Index(uint32_t rRef) const
    {
        ZASSERT(rRef != 0xFFFFFFFF); // a2 == -1
        return rRef % m_NumberOfBlocks;
    }

    uint16_t ZFixedSizeMemoryManagerBase::REF2ID(uint32_t rRef) const
    {
        ZASSERT(rRef != 0xFFFFFFFF);
        return rRef / m_NumberOfBlocks;
    }

    uint32_t ZFixedSizeMemoryManagerBase::Ptr2REF(const void* pPtr) const
    {
        uint32_t index = Address2Index(pPtr);
        
        if (m_ID[index] < 0)
            return 0xFFFFFFFF; // -1

        return static_cast<uint32_t>(m_ID[index]) * m_NumberOfBlocks + index;
    }

    bool ZFixedSizeMemoryManagerBase::IsAllocated(void* pPtr) const
    {
        return GetID(pPtr) >= 0;
    }

    void* ZFixedSizeMemoryManagerBase::Index2Address(int iIndex) const
    {
        ZASSERT(static_cast<uint32_t>(iIndex) < m_NumberOfBlocks);
        return &m_Buffer[iIndex * m_BlockSize];
    }

    int16_t ZFixedSizeMemoryManagerBase::GetID(const void* pPtr) const
    {
        uint32_t index = Address2Index(pPtr);
        return m_ID[index];
    }

    uint32_t ZFixedSizeMemoryManagerBase::GetCapacity() const
    {
        return m_NumberOfBlocks;
    }

    void* ZFixedSizeMemoryManagerBase::GetBufferPtr() const
    {
        return m_Buffer;
    }

    void ZFixedSizeMemoryManagerBase::Free(void* pPtr)
    {
        if (!pPtr)
            return;

        uint32_t index = Address2Index(pPtr);
        ZASSERT(m_ID[index] >= 0);

        m_ID[index] = -m_ID[index] - 2;

        if (m_FreeListValid)
        {
            ZFreeBlock* pNewBlock = ::new(pPtr) ZFreeBlock();

            if (m_LastFreeBlock)
            {
                ZASSERT(m_FirstFreeBlock != nullptr);
                
                m_LastFreeBlock->m_Next = pNewBlock;
                m_LastFreeBlock = pNewBlock;
            }
            else
            {
                ZASSERT(m_FirstFreeBlock == nullptr);
                
                m_FirstFreeBlock = pNewBlock;
                m_LastFreeBlock = pNewBlock;
            }
        }
    }

    uint32_t ZFixedSizeMemoryManagerBase::Count() const
    {
        uint32_t iAllocatedCount = 0;
        for (uint32_t i = 0; i < m_NumberOfBlocks; ++i)
        {
            if (m_ID[i] >= 0)
            {
                ++iAllocatedCount;
            }
        }
        return iAllocatedCount;
    }
    
    void* ZFixedSizeMemoryManagerBase::AllocDirect(uint32_t rRef)
    {
        uint32_t index = REF2Index(rRef);
        ZASSERT(m_ID[index] < 0);
        m_ID[index] = REF2ID(rRef);
        m_FreeListValid = false;
        return &m_Buffer[index * m_BlockSize];
    }
    
    void* ZFixedSizeMemoryManagerBase::Alloc()
    {
        if (!m_FreeListValid)
        {
            ValidateFreeList();
        }

        ZFreeBlock* pAllocatedBlock = m_FirstFreeBlock;
        if (pAllocatedBlock)
        {
            uint32_t index = Address2Index(pAllocatedBlock);

            ZASSERT(m_ID[index] < 0);
            m_ID[index] = ~m_ID[index];
            m_FirstFreeBlock = pAllocatedBlock->m_Next;
            if (!m_FirstFreeBlock)
            {
                m_LastFreeBlock = nullptr;
            }
        }

        return pAllocatedBlock;
    }

    uint32_t ZFixedSizeMemoryManagerBase::Address2Index(const void* pAddr) const
    {
        const char* address = static_cast<const char*>(pAddr);
            
        size_t poolSize = static_cast<size_t>(m_BlockSize) * m_NumberOfBlocks;
        ZASSERT(address >= m_Buffer && address < (m_Buffer + poolSize));

        size_t offset = address - m_Buffer;

        ZASSERT((offset % m_BlockSize) == 0);
        return static_cast<uint32_t>(offset / m_BlockSize);
    }
}
