#include <Glacier/ZSTL/ZBlockArrayBase.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZBlockArrayBase::ZBlockArrayBase(uint32_t item_size, uint32_t block_size_exhibitor)
        : m_ItemSize(item_size)
        , m_BlockSizeExhibitor(block_size_exhibitor)
        , m_SubIndexMask((1 << block_size_exhibitor) - 1)
        , m_PrimaryArrayCapacity(0)
        , m_PrimaryArray(nullptr)
        , m_Size(0)
    {
    }

    ZBlockArrayBase::~ZBlockArrayBase() = default;

    void ZBlockArrayBase::SetupWithoutInit(uint32_t new_size)
    {
        uint32_t required_blocks = CalculatePrimaryArraySize(new_size);

        m_PrimaryArrayCapacity = 2 * required_blocks;
        m_PrimaryArray = reinterpret_cast<char**>(ZUniMemory::Allocate(sizeof(char*) * m_PrimaryArrayCapacity));

        uint32_t block_byte_size = m_ItemSize << m_BlockSizeExhibitor;

        for (uint32_t i = 0; i < required_blocks; ++i)
        {
            m_PrimaryArray[i] = reinterpret_cast<char*>(ZUniMemory::Allocate(block_byte_size));
        }

        for (uint32_t j = required_blocks; j < m_PrimaryArrayCapacity; ++j)
        {
            m_PrimaryArray[j] = nullptr;
        }

        m_Size = new_size;
    }

    void ZBlockArrayBase::Setup(uint32_t new_size)
    {
        SetupWithoutInit(new_size);
        ConstructItems(0, new_size);
    }

    void ZBlockArrayBase::Resize(uint32_t new_size)
    {
        if (new_size == m_Size)
            return;

        uint32_t old_blocks = CalculatePrimaryArraySize(m_Size);
        uint32_t new_blocks = CalculatePrimaryArraySize(new_size);

        if (m_Size >= new_size)
        {
            DestructItems(m_Size, new_size);

            if (new_blocks < old_blocks)
            {
                for (uint32_t i = new_blocks; i < old_blocks; ++i)
                {
                    ZUniMemory::Free(m_PrimaryArray[i]);
                    m_PrimaryArray[i] = nullptr;
                }
            }
        }
        else
        {            
            if (old_blocks < new_blocks)
            {
                uint32_t block_byte_size = m_ItemSize << m_BlockSizeExhibitor;

                if (m_PrimaryArrayCapacity < new_blocks)
                {
                    m_PrimaryArrayCapacity = 2 * new_blocks;
                    
                    char** pNewPrimaryArray = reinterpret_cast<char**>(ZUniMemory::Allocate(sizeof(char*) * m_PrimaryArrayCapacity));

                    for (uint32_t j = 0; j < old_blocks; ++j)
                    {
                        pNewPrimaryArray[j] = m_PrimaryArray[j];
                    }

                    for (uint32_t j = old_blocks; j < m_PrimaryArrayCapacity; ++j)
                    {
                        pNewPrimaryArray[j] = nullptr;
                    }

                    if (m_PrimaryArray != nullptr)
                    {
                        ZUniMemory::Free(m_PrimaryArray);
                    }

                    m_PrimaryArray = pNewPrimaryArray;
                }

                for (uint32_t k = old_blocks; k < new_blocks; ++k)
                {
                    m_PrimaryArray[k] = reinterpret_cast<char*>(ZUniMemory::Allocate(block_byte_size));
                }
            }

            ConstructItems(m_Size, new_size);
        }
        
        m_Size = new_size;
    }

    void ZBlockArrayBase::Reset(uint32_t size)
    {
        Cleanup();
        Setup(size);
    }
    
    uint32_t ZBlockArrayBase::GetSize() const
    {
        return m_Size;
    }

    void* ZBlockArrayBase::GetItem(uint32_t item_idx)
    {
        if (item_idx >= m_Size)
        {
            Resize(item_idx + 1);
        }

        uint32_t block_idx = item_idx >> m_BlockSizeExhibitor;
        char* pBlock = m_PrimaryArray[block_idx];
        uint32_t local_idx = item_idx & m_SubIndexMask;

        ZASSERT(m_PrimaryArray != nullptr);
        ZASSERT(m_PrimaryArray[block_idx] != nullptr);

        return pBlock + (local_idx * m_ItemSize);
    }
    
    void ZBlockArrayBase::Cleanup()
    {
        DestructItems(m_Size, 0);

        int block_idx = static_cast<int>(CalculatePrimaryArraySize(m_Size));

        while (--block_idx != -1)
        {
            if (m_PrimaryArray[block_idx] != nullptr)
            {
                ZUniMemory::Free(m_PrimaryArray[block_idx]);
                m_PrimaryArray[block_idx] = nullptr;
            }
        }

        if (m_PrimaryArray != nullptr)
        {
            ZUniMemory::Free(m_PrimaryArray);
            m_PrimaryArray = nullptr;
        }

        m_PrimaryArrayCapacity = 0;
        m_Size = 0;
    }
    
    void ZBlockArrayBase::ConstructItems(const uint32_t from, const uint32_t to)
    {
        uint32_t idx = from;
        while (idx < to)
        {
            uint32_t block_idx = idx >> m_BlockSizeExhibitor;
            uint32_t local_idx = idx & m_SubIndexMask;
            
            ZASSERT(m_PrimaryArray != nullptr);
            ZASSERT(m_PrimaryArray[block_idx] != nullptr);

            void* pItemMemory = m_PrimaryArray[block_idx] + (local_idx * m_ItemSize);
            Construct(pItemMemory);

            ++idx;
        }
    }

    void ZBlockArrayBase::DestructItems(const uint32_t from, const uint32_t to)
    {
        uint32_t idx = from;
        while (idx > to)
        {
            --idx;

            uint32_t block_idx = idx >> m_BlockSizeExhibitor;
            uint32_t local_idx = idx & m_SubIndexMask;

            ZASSERT(m_PrimaryArray != nullptr);
            ZASSERT(m_PrimaryArray[block_idx] != nullptr);

            void* pItemMemory = m_PrimaryArray[block_idx] + (local_idx * m_ItemSize);
            Destruct(pItemMemory);
        }
    }
    
    uint32_t ZBlockArrayBase::CalculatePrimaryArraySize(uint32_t new_size) const
    {
        ZASSERT(m_BlockSizeExhibitor > 0 && m_BlockSizeExhibitor < 32);

        return (new_size + m_SubIndexMask) >> m_BlockSizeExhibitor;
    }
}