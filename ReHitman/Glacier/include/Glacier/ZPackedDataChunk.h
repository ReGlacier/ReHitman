#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // Small zlib-wrapped data block used by packed game data loaders.
    // The packed stream always carries a 9-byte header before the raw deflate
    // data (or before the raw bytes when m_eCompression == UNCOMPRESSED).
    struct ZPackedDataChunk
    {
        enum eCompressionType : int
        {
            ZIPPED = 0x0,
            UNCOMPRESSED = 0x1, // raw copy of the stream after the 9-byte header
        };

        // constants
        static constexpr uint32_t kStream_Header_Size = 9u; // header before the deflate/raw payload

        // On-disk layout of the kStream_Header_Size header that precedes every
        // packed stream. m_pPackedData is expected to point at this header, so
        // unzip() skips kStream_Header_Size bytes before the payload. Only the
        // first 9 bytes belong to the stream; the trailing padding is the
        // compiler's alignment, not part of the file.
        struct SPacked_Stream_Header
        {
            int32_t m_iRaw_Size;    // +0x00 uncompressed size
            int32_t m_iPacked_Size; // +0x04 packed size
            uint8_t m_eCompression; // +0x08 eCompressionType (ZIPPED / UNCOMPRESSED)
        };

        // methods
        ZPackedDataChunk();                                          // rev @ 0x42BC20
        bool unzip(char* pOutputBuffer, int iOutputBufferSize);      // rev @ 0x42BC40
        ~ZPackedDataChunk();                                         // rev @ 0x42C970

        bool m_bCustomBuffer;
        uint8_t* m_pPackedData;
        int m_iPackedDataLength;
        int m_iRawDataLength;
        int m_iCompressionLevel;
        eCompressionType m_eCompression;
    };
    RE_VERIFY_SIZE(ZPackedDataChunk, 0x18); // Verified
}