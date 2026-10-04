#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IInputStream.h>

#include <zlib.h>

#include <cstdint>


namespace Glacier
{
    /**
     * IInputStream that inflates a raw compressed byte source with zlib
     * (PC `serializerlib/crcstream.cpp`; the PC saved-game restore path wraps
     * this in ZInputStream + ZPackedInput, see ZEngineDataBase::AllocSequence).
     *
     * The source stream is read as-is (no framing), 4096 bytes at a time into
     * m_Buffer, and fed through `inflate` until the caller's buffer is full,
     * the source is exhausted, or the member stream ends (Z_STREAM_END).
     *
     * The PC constructor calls `inflateInit` from the vendored zlib 1.1.3
     * (`inflateInit(&m_Parameters, "1.1.3", 56)` at 0x455E10), so the member
     * type below must stay the vendored `z_stream`.
     */
    struct ZCompressedInputStream : public IInputStream
    {
        static constexpr uint32_t BUFFER_SIZE = 0x1000;

        // vtbl
        /** Ends the inflate stream (PC 0x455D10). */
        ~ZCompressedInputStream() override;

        /**
         * Produces up to size inflated bytes into address; 0 once the member
         * stream ended or the compressed source is exhausted.
         */
        uint32_t Read(void* address, const uint32_t size) override;

        // methods
        /** Primes m_Buffer with the first 4096 compressed bytes and initializes inflate. */
        ZCompressedInputStream(IInputStream* stream);

        // members
        /** Compressed byte source. */
        IInputStream* m_Stream{nullptr};

        /** Raw compressed bytes staged from m_Stream. */
        uint8_t m_Buffer[BUFFER_SIZE]{};

        /** inflate state (zlib 1.1.3 z_stream, 56 bytes). */
        z_stream m_Parameters{};
    };
    // PC layout verified from ZCompressedInputStream ctor (0x455E10) and Read (0x455EC0):
    // stream +0x04, buffer +0x08, z_stream +0x1008 (0x1008 + sizeof(z_stream) = end).
    // Debug info of other builds records 0x1050 with a non-1.1.3 z_stream; the PC
    // inflateInit call passes 56 for sizeof(z_stream), which matches this layout.
    RE_VERIFY_OFFSET(ZCompressedInputStream, m_Stream, 0x4);
    RE_VERIFY_OFFSET(ZCompressedInputStream, m_Buffer, 0x8);
    RE_VERIFY_OFFSET(ZCompressedInputStream, m_Parameters, 0x1008);
    RE_VERIFY_SIZE(ZCompressedInputStream, 0x1040);
}
