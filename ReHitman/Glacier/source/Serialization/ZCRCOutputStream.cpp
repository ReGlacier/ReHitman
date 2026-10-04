#include <Glacier/Serializer/ZCRCOutputStream.h>

#include <cstring>


namespace Glacier
{
    // PC 0x4561A0. The PC constructor leaves m_BlockHeader/m_BlockSize/m_Buffer
    // untouched; this class value-initializes them instead so the emitted
    // header bytes are deterministic.
    ZCRCOutputStream::ZCRCOutputStream(IOutputStream* stream)
        : m_Stream{stream}
    {
    }

    // PC 0x4561C0 only destroys the (trivial) base subobjects. This build also
    // flushes a pending short final block so ZCRCInputStream can reach its
    // clean end-of-stream state; other builds flush in the destructor as well
    // (PS2 writes its own crc32-headered frame there).
    ZCRCOutputStream::~ZCRCOutputStream()
    {
        if (m_BufferPos != 0)
        {
            // The frame width stays BLOCK_SIZE; bytes past m_BlockSize hold
            // stale staging-buffer contents that the reader never consumes.
            m_BlockSize = m_BufferPos;
            m_Stream->Write(&m_BlockHeader, 4);
            m_Stream->Write(m_Buffer, BLOCK_SIZE);
            m_Stream->Write(&m_BlockSize, 4);
            m_BufferPos = 0;
        }
    }

    // PC 0x455FD0.
    uint32_t ZCRCOutputStream::Write(const void* pAddr, const uint32_t lSize)
    {
        // PC accumulates the total before the empty-write early-out.
        m_Total += lSize;

        uint32_t result = reinterpret_cast<uint32_t>(pAddr); // PC leaves the pointer here when lSize == 0.
        uint32_t remaining = lSize;
        const char* src = static_cast<const char*>(pAddr);

        if (lSize)
        {
            do
            {
                const uint32_t pos = m_BufferPos;
                result = remaining;
                if (pos + remaining > BLOCK_SIZE)
                    result = BLOCK_SIZE - pos;

                std::memcpy(m_Buffer + pos, src, result);

                const uint32_t newPos = result + pos;
                remaining -= result;
                m_BufferPos = newPos;
                src += result;

                if (newPos == BLOCK_SIZE)
                {
                    m_BlockSize = BLOCK_SIZE;
                    m_Stream->Write(&m_BlockHeader, 4);
                    m_Stream->Write(m_Buffer, BLOCK_SIZE);
                    result = m_Stream->Write(&m_BlockSize, 4);
                    m_BufferPos = 0;
                }
            }
            while (remaining);
        }

        return result;
    }
}
