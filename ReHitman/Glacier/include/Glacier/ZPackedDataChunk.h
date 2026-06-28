#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct ZPackedDataChunk
    {
        enum eCompressionType : int { ZIPPED = 0x0 };

        bool m_bCustomBuffer;
        uint8_t* m_pPackedData;
        int m_iPackedDataLength;
        int m_iRawDataLength;
        int m_iCompressionLevel;
        eCompressionType m_eCompression;
    };
    RE_VERIFY_SIZE(ZPackedDataChunk, 0x18); // Verified
}