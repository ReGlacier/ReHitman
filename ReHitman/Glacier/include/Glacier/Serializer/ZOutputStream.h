#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IOutputStream.h>
#include <Glacier/Serializer/ZOutputStreamBase.h>

#include <cstddef>
#include <cstdint>


namespace Glacier
{
    /** Buffered output stream wrapper for serializer byte sinks. */
    struct ZOutputStream : public ZOutputStreamBase
    {
        // consts
        /** Size of the local write buffer before flushing to the sink. */
        static constexpr size_t MAX_BUFFER_SIZE = 4096;
        // vtbl
        /** Flushes pending bytes before destroying the stream. */
        ~ZOutputStream() override;

        /** Writes raw bytes, buffering small writes and forwarding large writes. */
        uint32_t WriteRaw(char* address, const uint32_t size) override;

        /** Writes bytes while remapping source offsets with the supplied mask. */
        uint32_t WriteChangeEndianness(char * address, const uint32_t size, const uint32_t mask) override;
        
        // methods
        /** Creates a buffered wrapper around a low-level output sink. */
        ZOutputStream(IOutputStream& Stream);

        /** Flushes pending buffered bytes to the underlying sink. */
        int FlushBuffer();

        // members
        /** Underlying byte sink. */
        IOutputStream& m_Stream;

        /** Number of bytes currently stored in m_Buffer. */
        size_t m_BufferPtr{0};

        /** Local write buffer used for small writes. */
        uint8_t m_Buffer[MAX_BUFFER_SIZE]{'\0'};
    };
    RE_VERIFY_SIZE(ZOutputStream, 0x1010); // Verified in PC
}
