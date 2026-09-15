#include <Glacier/Serializer/ZOutputStream.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZOutputStream::ZOutputStream(IOutputStream& Stream)
        : m_Stream{Stream}
    {
    }

    ZOutputStream::~ZOutputStream()
    {
        FlushBuffer();
    }

    uint32_t ZOutputStream::WriteRaw(char* pAddr, const uint32_t lSize)
    {
        static constexpr size_t kSmallBufferSize = 0x400;

        if (!pAddr && lSize > 0)
        {
            ZASSERT(false && "pAddr is null but lSize is non-zero");
        }

        if (lSize == 0)
        {
            return 0;
        }

        if (lSize < kSmallBufferSize)
        {
            uint32_t bytesWritten = 0;

            while (bytesWritten < lSize)
            {
                if (m_BufferPtr == MAX_BUFFER_SIZE)
                {
                    m_Stream.Write(m_Buffer, MAX_BUFFER_SIZE);
                    m_BufferPtr = 0;
                }

                uint32_t remainingInInput = lSize - bytesWritten;
                uint32_t spaceInBuffer = MAX_BUFFER_SIZE - m_BufferPtr;
                uint32_t chunkSize = std::min(remainingInInput, spaceInBuffer);

                std::memcpy(&m_Buffer[m_BufferPtr], &pAddr[bytesWritten], chunkSize);

                m_BufferPtr += chunkSize;
                bytesWritten += chunkSize;
            }
        }
        else
        {
            if (m_BufferPtr > 0)
            {
                m_Stream.Write(m_Buffer, m_BufferPtr);
                m_BufferPtr = 0;
            }

            m_Stream.Write(pAddr, lSize);
        }

        return lSize;
    }

    uint32_t ZOutputStream::WriteChangeEndianness(char* pdAddr, const uint32_t lSize, const uint32_t lMask)
    {
        if (!pdAddr && lSize > 0)
        {
            ZASSERT(false && "pdAddr is null but lSize is non-zero");
        }

        if (lSize == 0)
        {
            return 0;
        }

        uint32_t bytesProcessed = 0;

        while (bytesProcessed < lSize)
        {
            if (m_BufferPtr == MAX_BUFFER_SIZE)
            {
                m_Stream.Write(m_Buffer, MAX_BUFFER_SIZE);
                m_BufferPtr = 0;
            }

            uint32_t remainingInInput = lSize - bytesProcessed;
            uint32_t spaceInBuffer = MAX_BUFFER_SIZE - m_BufferPtr;
            uint32_t chunkSize = std::min(remainingInInput, spaceInBuffer);

            for (uint32_t i = 0; i < chunkSize; ++i)
            {
                m_Buffer[m_BufferPtr] = pdAddr[lMask ^ bytesProcessed];
                
                ++m_BufferPtr;
                ++bytesProcessed;
            }
        }
        
        return lSize;
    }

    int ZOutputStream::FlushBuffer()
    {
        int iWrittenBytesNr = m_Stream.Write(&m_Buffer[0], m_BufferPtr);
        m_BufferPtr = 0;
        return iWrittenBytesNr;
    }
}