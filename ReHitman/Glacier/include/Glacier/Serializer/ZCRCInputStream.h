#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IInputStream.h>

#include <cstdint>


namespace Glacier
{
    /**
     * IInputStream that deframes the block format produced by ZCRCOutputStream
     * (PC `serializerlib/crcstream.cpp`).
     *
     * On-wire format (from PC ZCRCInputStream::Read, 0x456090):
     *   repeat:
     *     u32  block header        (unused by the base class; see notes)
     *     u8[0x4000] block data    (always 0x4000 bytes on the wire)
     *     u32  block valid size    (< 0x4000 marks the final block)
     *
     * The PC build never inspects the block header. The CRC flow that gives
     * this class its name only exists in other builds: their debug info adds a
     * `virtual void HandleError()` and the PS2 `Read` calls it on a crc32
     * mismatch. The PC vftables hold just destructor + Read (both 0x760698 and
     * the derived ZEngineDataCRCInputStream table at 0x76145C are followed by
     * alignment), so no such virtual is declared here. See
     * MemoryBank/Serializer/ZCRCInputStream.md.
     *
     * A truncated block read is fatal for the stream: Read() returns
     * (uint32_t)-1; reads past the valid bytes of the final short block
     * return 0.
     */
    struct ZCRCInputStream : public IInputStream
    {
        static constexpr uint32_t BLOCK_SIZE = 0x4000;

        // vtbl
        ~ZCRCInputStream() override;

        /** Copies the next logical bytes, deframing blocks; (uint32_t)-1 on truncated input. */
        uint32_t Read(void* address, const uint32_t size) override;

        // methods
        ZCRCInputStream(IInputStream* stream);

        // members
        /** Framed byte source. */
        IInputStream* m_Stream{nullptr};

        /** Current position in the unpacked block (== BLOCK_SIZE means "no block loaded"). */
        uint32_t m_BlockPos{BLOCK_SIZE};

        /** Raw 4-byte header of the current block (opaque, matches PC). */
        uint32_t m_BlockHeader{0};

        /** Valid byte count of the current block (< BLOCK_SIZE means final block). */
        uint32_t m_BlockSize{0};

        /** Current block payload. */
        uint8_t m_Block[BLOCK_SIZE]{};

        /** Set once a final (short) block header has been seen. */
        bool m_LastBlock{false};

        /** Set once all valid bytes of the final block have been handed out. */
        bool m_EndOfStream{false};
    };
    // PC layout verified from ZCRCInputStream::Ctor (0x4561E0) and Read (0x456090):
    // stream +0x04, block pos +0x08, header +0x0C, block size +0x10, block +0x14,
    // last-block flag +0x4014, end-of-stream flag +0x4015.
    RE_VERIFY_OFFSET(ZCRCInputStream, m_Stream, 0x4);
    RE_VERIFY_OFFSET(ZCRCInputStream, m_BlockPos, 0x8);
    RE_VERIFY_OFFSET(ZCRCInputStream, m_BlockHeader, 0xC);
    RE_VERIFY_OFFSET(ZCRCInputStream, m_BlockSize, 0x10);
    RE_VERIFY_OFFSET(ZCRCInputStream, m_Block, 0x14);
    RE_VERIFY_OFFSET(ZCRCInputStream, m_LastBlock, 0x4014);
    RE_VERIFY_OFFSET(ZCRCInputStream, m_EndOfStream, 0x4015);
    RE_VERIFY_SIZE(ZCRCInputStream, 0x4018); // Derived from PC field offsets; the PC class-info records 0x4010, which conflicts with those observed offsets (flags at +0x4014/+0x4015) — see MemoryBank/Serializer/ZCRCInputStream.md.
}
