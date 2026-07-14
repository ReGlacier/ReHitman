#include <Glacier/Serializer/ZInputStream.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <algorithm>
#include <cstring>


namespace Glacier
{
    ZInputStream::~ZInputStream() = default;

    ZInputStream::ZInputStream(IInputStream& stream) 
        : m_Stream(stream)
    {}

    bool ZInputStream::ReadNextBlock()
    {
        m_BufferEnd = m_Stream.Read(m_Buffer, MAX_BUFFER_SIZE);
        m_BufferPtr = 0;

        return m_BufferEnd != 0;
    }

    uint32_t ZInputStream::ReadRaw(char* address, const uint32_t size)
    {
        ZASSERT(address != nullptr || size == 0); // WTF? In original code address != NULL || size == 0

        uint32_t offset = 0;

        while (offset < size)
        {
            if (m_BufferPtr >= m_BufferEnd)
            {
                if (!ReadNextBlock())
                    break;
            }

            const uint32_t copySize = std::min<uint32_t>(static_cast<uint32_t>(m_BufferEnd - m_BufferPtr), size - offset);
            std::memcpy(&address[offset], &m_Buffer[m_BufferPtr], copySize);

            offset += copySize;
            m_BufferPtr += copySize;
        }

        return offset;
    }

    uint32_t ZInputStream::ReadChangeEndianness(char * address, const uint32_t size, const uint32_t mask)
    {
        ZASSERT(address != nullptr || size == 0); // WTF? In original code address != NULL || size == 0

        uint32_t offset = 0;

        while (offset < size)
        {
            if (m_BufferPtr >= m_BufferEnd)
            {
                if (!ReadNextBlock())
                    break;
            }

            uint32_t copySize = std::min<uint32_t>(static_cast<uint32_t>(m_BufferEnd - m_BufferPtr), size - offset);

            while (copySize-- != 0)
            {
                address[offset ^ mask] = m_Buffer[m_BufferPtr];
                ++m_BufferPtr;
                ++offset;
            }
        }

        return offset;
    }
}
