#include <Glacier/Serializer/ZCRCInputStream.h>

#include <cstring>


namespace Glacier
{
    // PC 0x4561E0.
    ZCRCInputStream::ZCRCInputStream(IInputStream* stream)
        : m_Stream{stream}
        , m_BlockPos{BLOCK_SIZE} // Sentinel: no block loaded yet.
    {
    }

    // PC destructor is empty; the deleting destructor variant is at 0x456210.
    ZCRCInputStream::~ZCRCInputStream() = default;

    // PC 0x456090.
    uint32_t ZCRCInputStream::Read(void* address, const uint32_t size)
    {
        uint32_t remaining = size;

        if (size)
        {
            char* dst = static_cast<char*>(address);

            while (!m_EndOfStream)
            {
                if (m_BlockPos == BLOCK_SIZE)
                {
                    if (m_Stream->Read(&m_BlockHeader, 4) != 4
                        || m_Stream->Read(m_Block, BLOCK_SIZE) != BLOCK_SIZE
                        || m_Stream->Read(&m_BlockSize, 4) != 4)
                    {
                        // A partial block read latches a fatal framing error.
                        return static_cast<uint32_t>(-1);
                    }

                    if (m_BlockSize != BLOCK_SIZE)
                        m_LastBlock = true;
                    m_BlockPos = 0;
                }

                const uint32_t pos = m_BlockPos;
                uint32_t chunk = remaining;
                if (chunk > BLOCK_SIZE - pos)
                    chunk = BLOCK_SIZE - pos;
                if (m_LastBlock)
                {
                    if (chunk > m_BlockSize - pos)
                    {
                        chunk = m_BlockSize - pos;
                        m_EndOfStream = true;
                    }
                }

                std::memcpy(dst, m_Block + pos, chunk);
                remaining -= chunk;
                m_BlockPos = pos + chunk;
                dst += chunk;

                if (!remaining)
                    return size;
            }
        }

        return size - remaining;
    }
}
