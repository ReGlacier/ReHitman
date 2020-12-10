#pragma once

#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    class ZPackedDataChunk
    {
    public:
        bool m_bufferWasFreed;
        bool __unused1__;
        bool __unused2__;
        bool __unused3__;
        uint8_t* m_buffer;
        uint32_t m_bufferSize;
        uint32_t m_uncompressedSize;
        int field_10;
        bool m_isUncompressedAlready;
    };
}