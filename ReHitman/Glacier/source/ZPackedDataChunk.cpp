#include <Glacier/ZPackedDataChunk.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <zlib.h>
#include <cstring>

namespace Glacier
{
    // PC 0x42BC20. Members the ctor does not touch are zeroed by the original
    // as well (the decompile zeroes the whole stack object before the call).
    ZPackedDataChunk::ZPackedDataChunk()
        : m_bCustomBuffer(false)
        , m_pPackedData(nullptr)
        , m_iPackedDataLength(0)
        , m_iRawDataLength(0)
        , m_iCompressionLevel(6)
        , m_eCompression(ZIPPED)
    {
    }

    // PC 0x42BC40. The packed stream skips a fixed 9-byte header. The original
    // returns 0 for every success path and 1 for failures (the only in-game
    // caller ignores the value); normalized here to true == success.
    bool ZPackedDataChunk::unzip(char* pOutputBuffer, int iOutputBufferSize)
    {
        if (m_eCompression == UNCOMPRESSED)
        {
            int copied = m_iRawDataLength;
            if (iOutputBufferSize <= copied)
                copied = iOutputBufferSize;
            memcpy(pOutputBuffer, m_pPackedData + kStream_Header_Size, copied);
            return true;
        }

        z_stream stream = {};
        stream.next_in = m_pPackedData + kStream_Header_Size;
        stream.avail_in = m_iPackedDataLength;
        stream.next_out = reinterpret_cast<Bytef*>(pOutputBuffer);
        stream.avail_out = static_cast<uInt>(iOutputBufferSize);
        stream.zalloc = nullptr;
        stream.zfree = nullptr;

        // PC called inflateInit2_ with zlib 1.1.3 ABI parameters; the project
        // links its own zlib, so the inflateInit2 macro is the faithful form.
        if (inflateInit2(&stream, -15) != Z_OK)
            return false;

        const int result = inflate(&stream, Z_FINISH);
        inflateEnd(&stream);

        // Success is either a finished stream or a buffer-full stop.
        return result == Z_STREAM_END || (result == Z_BUF_ERROR && stream.avail_out == 0);
    }

    // PC 0x42C970. The packed buffer is only owned when it was not supplied
    // externally (m_bCustomBuffer == false).
    ZPackedDataChunk::~ZPackedDataChunk()
    {
        if (m_pPackedData)
        {
            if (!m_bCustomBuffer)
                ZUniMemory::Free(m_pPackedData);
            m_pPackedData = nullptr;
            m_iPackedDataLength = 0;
            m_iRawDataLength = 0;
            m_eCompression = ZIPPED;
        }
    }
}
