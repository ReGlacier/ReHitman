#include <Glacier/Serializer/ZMemoryOutputStream.h>
#include <Glacier/Serializer/ZOutputStream.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <algorithm>
#include <cstring>


namespace Glacier
{
    uint32_t ZMemoryOutputStream::WriteRaw(char* pAddr, const uint32_t lSize) 
    {
        if (!pAddr && lSize > 0)
        {
            ZASSERT(false && "pAddr is null but lSize is non-zero");
        }

        if (lSize == 0)
        {
            return 0;
        }

        uint32_t remaining = lSize;
        char* srcPtr = pAddr;

        while (remaining + m_Index > ZHeader::m_PageSize)
        {
            uint32_t spaceInPage = ZHeader::m_PageSize - m_Index;

            if (spaceInPage > 0)
            {
                std::memcpy(&m_Last->m_Data[m_Index], srcPtr, spaceInPage);
                remaining -= spaceInPage;
                srcPtr += spaceInPage;
            }

            void* rawMemory = ZUniMemory::Allocate(sizeof(ZHeader));
            ZHeader* newHeader = nullptr;
            
            if (rawMemory)
            {
                newHeader = ::new (rawMemory) ZHeader();
                newHeader->m_Next = nullptr;
            }

            ZASSERT(newHeader != nullptr && "Failed to allocate new ZHeader page");

            m_Last->m_Next = newHeader;
            m_Last = newHeader;
            m_Index = 0;
        }

        if (remaining > 0)
        {
            std::memcpy(&m_Last->m_Data[m_Index], srcPtr, remaining);
            m_Index += remaining;
        }
        
        return m_Index;
    }
    
    uint32_t ZMemoryOutputStream::WriteChangeEndianness(char * pAddr, const uint32_t lSize, const uint32_t lMask)
    {
        if (!pAddr && lSize > 0)
        {
            ZASSERT(false && "pAddr is null but lSize is non-zero");
        }

        if (lSize == 0)
        {
            return 0;
        }

        uint32_t bytesProcessed = 0;

        while (bytesProcessed < lSize)
        {
            if (m_Index == ZHeader::m_PageSize)
            {
                void* rawMemory = ZUniMemory::Allocate(sizeof(ZHeader));
                ZHeader* newHeader = nullptr;
                
                if (rawMemory)
                {
                    newHeader = ::new (rawMemory) ZHeader();
                    newHeader->m_Next = nullptr;
                }
                ZASSERT(newHeader != nullptr && "Failed to allocate new ZHeader page");

                m_Last->m_Next = newHeader;
                m_Last = newHeader;
                
                m_Index = 0;
            }

            uint32_t remainingInInput = lSize - bytesProcessed;
            uint32_t spaceInPage = ZHeader::m_PageSize - m_Index;
            uint32_t chunkSize = std::min(remainingInInput, spaceInPage);

            uint8_t* writePtr = &m_Last->m_Data[m_Index];

            for (uint32_t i = 0; i < chunkSize; ++i)
            {
                *writePtr = pAddr[lMask ^ bytesProcessed];
                
                ++writePtr;
                ++bytesProcessed;
            }

            m_Index += chunkSize;
        }

        return lSize;
    }
    
    uint32_t ZMemoryOutputStream::Write(const void* pAddr, const uint32_t lSize)
    {
        return WriteRaw(reinterpret_cast<char*>(const_cast<void*>(pAddr)), lSize);
    }

    ZMemoryOutputStream::ZMemoryOutputStream()
    {
        void* rawMemory = ZUniMemory::Allocate(sizeof(ZHeader));
        m_First = rawMemory ? ::new (rawMemory) ZHeader() : nullptr;
        ZASSERT(m_First != nullptr && "Failed to allocate first ZHeader page");

        m_Last = m_First;
        m_Index = 0;
    }

    ZMemoryOutputStream::~ZMemoryOutputStream() 
    {
        ZHeader* current = m_First;
    
        while (current != nullptr)
        {
            ZHeader* next = current->m_Next;
            ZUniMemory::Free(current);
            current = next;
        }
        
        m_First = nullptr;
        m_Last = nullptr;
        m_Index = 0;
    }

    void ZMemoryOutputStream::CopyTo(ZOutputStream& stream)
    {
        if (!m_First)
        {
            return;
        }

        ZHeader* current = m_First;

        while (current->m_Next != nullptr)
        {
            stream.WriteRaw(reinterpret_cast<char*>(current->m_Data), ZHeader::m_PageSize);            
            current = current->m_Next;
        }

        if (m_Index > 0)
        {
            stream.WriteRaw(reinterpret_cast<char*>(current->m_Data), m_Index);
        }
    }
}
