#pragma once

#include <Glacier/Serializer/IInputStream.h>
#include <Glacier/Serializer/ZInputStreamBase.h>

#include <cstddef>
#include <cstdint>


namespace Glacier
{
    /** Buffered input stream wrapper used by packed serializer readers. */
    struct ZInputStream : public ZInputStreamBase
    {
        // const
        /** Size of the local read-ahead buffer. */
        static constexpr size_t MAX_BUFFER_SIZE = 0x4000; // In MiniNinjas this value increased up to 0x8000

        // vtbl
        ~ZInputStream() override;

        /** Reads raw bytes from the buffered stream. */
        uint32_t ReadRaw(char* address, const uint32_t size) override;

        /** Reads bytes while remapping destination offsets with the supplied mask. */
        uint32_t ReadChangeEndianness(char * address, const uint32_t size, const uint32_t mask) override;

        // methods
        /** Creates a buffered wrapper around a low-level input stream. */
        ZInputStream(IInputStream& stream);

        /** Refills the local buffer from the underlying stream. */
        bool ReadNextBlock();

        // members
        /** Underlying byte source. */
        IInputStream& m_Stream;

        /** Current read position within m_Buffer. */
        size_t m_BufferPtr{0};

        /** One-past-last valid byte within m_Buffer. */
        size_t m_BufferEnd{0};

        /** Local read-ahead buffer. */
        char m_Buffer[MAX_BUFFER_SIZE]{'\0'};
    };
}
