#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IOutputStream.h>

#include <cstdint>


namespace Glacier
{
    /**
     * IOutputStream that frames bytes for ZCRCInputStream (PC
     * `serializerlib/crcstream.cpp`).
     *
     * On-wire format (matching PC ZCRCInputStream::Read, 0x456090):
     *   repeat:
     *     u32  block header        (always written; unused by the PC reader)
     *     u8[0x4000] block data    (stale staging bytes past the final block's
     *                              valid size are never consumed by the reader)
     *     u32  block valid size    (< 0x4000 marks the final block)
     *
     * Note on the PC build: `Write` (0x455FD0) only emits a block once the
     * staging buffer is exactly full and its destructor (0x4561C0) does nothing
     * else, so a pending short final block is never flushed there and no CRC is
     * never computed (the header bytes are even left uninitialized by the PC
     * constructor). This implementation flushes the pending short final block
     * from the destructor so pairs of streams round-trip; other builds do flush
     * at destruction too (PS2 uses its own ~ZCRCOutputStream frame with a crc32
     * header and no trailing length field).
     */
    struct ZCRCOutputStream : public IOutputStream
    {
        static constexpr uint32_t BLOCK_SIZE = 0x4000;

        // vtbl
        /** Flushes a pending short final block (see class notes). PC 0x4561C0 is empty. */
        ~ZCRCOutputStream() override;

        /** Appends bytes into the staging buffer, emitting full blocks as needed. */
        uint32_t Write(const void* pAddr, const uint32_t lSize) override;

        // methods
        ZCRCOutputStream(IOutputStream* stream);

        // members
        /** Framed byte sink. */
        IOutputStream* m_Stream{nullptr};

        /** Current fill level of m_Buffer. */
        uint32_t m_BufferPos{0};

        /** Total caller-provided bytes ever passed to Write (PC accumulates at +0xC). */
        uint32_t m_Total{0};

        /** Raw 4-byte header emitted before every block (opaque, PC never computes a CRC). */
        uint32_t m_BlockHeader{0};

        /** Valid byte count of the block currently being emitted. */
        uint32_t m_BlockSize{0};

        /** Staging buffer for the block currently being filled. */
        uint8_t m_Buffer[BLOCK_SIZE]{};
    };
    // PC layout verified from ZCRCOutputStream ctor (0x4561A0) and Write (0x455FD0):
    // stream +0x04, buffer pos +0x08, accumulated total +0x0C, header +0x10,
    // block size +0x14, staging buffer +0x18.
    RE_VERIFY_OFFSET(ZCRCOutputStream, m_Stream, 0x4);
    RE_VERIFY_OFFSET(ZCRCOutputStream, m_BufferPos, 0x8);
    RE_VERIFY_OFFSET(ZCRCOutputStream, m_Total, 0xC);
    RE_VERIFY_OFFSET(ZCRCOutputStream, m_BlockHeader, 0x10);
    RE_VERIFY_OFFSET(ZCRCOutputStream, m_BlockSize, 0x14);
    RE_VERIFY_OFFSET(ZCRCOutputStream, m_Buffer, 0x18);
    RE_VERIFY_SIZE(ZCRCOutputStream, 0x4018);
}
