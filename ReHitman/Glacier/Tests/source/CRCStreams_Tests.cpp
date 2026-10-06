#include <Glacier/Serializer/IInputStream.h>
#include <Glacier/Serializer/IOutputStream.h>
#include <Glacier/Serializer/ZCompressedInputStream.h>
#include <Glacier/Serializer/ZCRCInputStream.h>
#include <Glacier/Serializer/ZCRCOutputStream.h>
#include <gtest/gtest.h>

#include <zlib.h>

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

using namespace Glacier;

namespace
{
    /** Fixed in-memory byte source for framing/compressed stream tests. */
    struct MemoryInputStream final : public IInputStream
    {
        std::vector<char> Bytes;
        size_t Offset{0};

        explicit MemoryInputStream(std::vector<char> bytes)
            : Bytes(std::move(bytes))
        {
        }

        uint32_t Read(void* address, const uint32_t size) override
        {
            const uint32_t remaining = static_cast<uint32_t>(Bytes.size() - Offset);
            const uint32_t readSize = std::min(size, remaining);

            if (readSize != 0)
                std::memcpy(address, Bytes.data() + Offset, readSize);

            Offset += readSize;
            return readSize;
        }
    };

    /** Fixed in-memory byte sink capturing everything written. */
    struct MemoryOutputStream final : public IOutputStream
    {
        std::vector<char> Bytes;

        uint32_t Write(const void* pAddr, const uint32_t lSize) override
        {
            const char* src = static_cast<const char*>(pAddr);
            Bytes.insert(Bytes.end(), src, src + lSize);
            return 0;
        }
    };

    /** Deterministic pseudo-random payload. */
    std::vector<char> MakePayload(size_t count)
    {
        std::vector<char> data(count);
        uint32_t state = 0x12345678u;
        for (size_t i = 0; i < count; ++i)
        {
            state = state * 1103515245u + 12345u;
            data[i] = static_cast<char>((state >> 16) ^ static_cast<uint32_t>(i));
        }
        return data;
    }

    void AppendU32LE(std::vector<char>& out, uint32_t value)
    {
        for (int b = 0; b < 4; ++b)
            out.push_back(static_cast<char>((value >> (8 * b)) & 0xFFu));
    }

    uint32_t ReadU32LE(const char* p)
    {
        return static_cast<uint32_t>(
            static_cast<unsigned char>(p[0])
            | (static_cast<unsigned char>(p[1]) << 8)
            | (static_cast<unsigned char>(p[2]) << 16)
            | (static_cast<unsigned char>(p[3]) << 24));
    }

    /** Framed bytes as produced by our ZCRCOutputStream (dtor flushes the tail). */
    std::vector<char> FrameViaWriter(const std::vector<char>& payload)
    {
        MemoryOutputStream sink;
        {
            ZCRCOutputStream out(&sink);
            if (!payload.empty())
                out.Write(payload.data(), static_cast<uint32_t>(payload.size()));
        }
        return sink.Bytes;
    }

    /** Hand-built frame: [header][0x4000 data][valid size]. */
    std::vector<char> BuildFrame(uint32_t header, const std::vector<char>& blockData, uint32_t validSize)
    {
        std::vector<char> framing;
        AppendU32LE(framing, header);
        framing.insert(framing.end(), blockData.begin(), blockData.end());
        framing.resize(4 + ZCRCInputStream::BLOCK_SIZE, '\0');
        AppendU32LE(framing, validSize);
        return framing;
    }

    std::vector<char> DeflateAll(const std::vector<char>& raw)
    {
        std::vector<char> out(raw.size() + raw.size() / 1000 + 13);
        uLongf destLen = static_cast<uLongf>(out.size());
        const int status = compress2(
            reinterpret_cast<Bytef*>(out.data()), &destLen,
            reinterpret_cast<const Bytef*>(raw.data()),
            static_cast<uLong>(raw.size()), Z_DEFAULT_COMPRESSION);
        EXPECT_EQ(status, Z_OK);
        out.resize(destLen);
        return out;
    }
}

TEST(ZCRCStreams, OutputEmitsExpectedFraming)
{
    const std::vector<char> payload = MakePayload(ZCRCInputStream::BLOCK_SIZE + 100);

    MemoryOutputStream sink;
    {
        ZCRCOutputStream out(&sink);
        out.Write(payload.data(), static_cast<uint32_t>(payload.size()));
        // Nothing is written for the trailing 100 bytes until destruction.
        EXPECT_EQ(sink.Bytes.size(), 4u + ZCRCInputStream::BLOCK_SIZE + 4u);
    }

    const size_t blockStride = 4u + ZCRCInputStream::BLOCK_SIZE + 4u;
    ASSERT_EQ(sink.Bytes.size(), blockStride * 2u);

    const char* block0 = sink.Bytes.data();
    EXPECT_EQ(ReadU32LE(block0), 0u); // Opaque header, zero by construction.
    EXPECT_EQ(std::memcmp(block0 + 4, payload.data(), ZCRCInputStream::BLOCK_SIZE), 0);
    EXPECT_EQ(ReadU32LE(block0 + 4 + ZCRCInputStream::BLOCK_SIZE), ZCRCInputStream::BLOCK_SIZE);

    const char* block1 = block0 + blockStride;
    EXPECT_EQ(ReadU32LE(block1), 0u);
    EXPECT_EQ(std::memcmp(block1 + 4, payload.data() + ZCRCInputStream::BLOCK_SIZE, 100), 0);
    // The frame always carries a full 0x4000-byte block; bytes past the valid
    // size hold stale staging-buffer contents and are never consumed by the
    // reader, so they are deliberately not asserted here.
    EXPECT_EQ(ReadU32LE(block1 + 4 + ZCRCInputStream::BLOCK_SIZE), 100u);
}

TEST(ZCRCStreams, RoundTripsSingleReadForVariousLengths)
{
    for (const uint32_t length : {1u, 4095u, 4096u, 4097u, 12289u})
    {
        const std::vector<char> payload = MakePayload(length);
        const std::vector<char> framing = FrameViaWriter(payload);

        MemoryInputStream source(framing);
        ZCRCInputStream in(&source);

        std::vector<char> output(length);
        EXPECT_EQ(in.Read(output.data(), length), length) << "length " << length;
        EXPECT_EQ(std::memcmp(output.data(), payload.data(), length), 0) << "length " << length;
    }
}

TEST(ZCRCStreams, FinalShortBlockReadsBackAsZero)
{
    const std::vector<char> payload = MakePayload(5);
    MemoryInputStream source(FrameViaWriter(payload));
    ZCRCInputStream in(&source);

    char output[8]{};
    EXPECT_EQ(in.Read(output, sizeof(output)), 5u);
    EXPECT_EQ(std::memcmp(output, payload.data(), 5), 0);

    EXPECT_EQ(in.Read(output, sizeof(output)), 0u);
    EXPECT_EQ(in.Read(output, 1), 0u);
}

TEST(ZCRCStreams, ChunkedReadsSpanBlockBoundary)
{
    const uint32_t length = ZCRCInputStream::BLOCK_SIZE + 1u;
    const std::vector<char> payload = MakePayload(length);
    MemoryInputStream source(FrameViaWriter(payload));
    ZCRCInputStream in(&source);

    std::vector<char> output;
    output.reserve(length);

    // Chunk sizes deliberately straddle both block boundaries; a single Read
    // clamps to the block end (as PC does), so keep asking until the last
    // valid byte of the short tail block arrives.
    uint32_t got = 0;
    char buffer[2048];
    while (got < length)
    {
        const uint32_t want = std::min(2048u, length - got);
        const uint32_t n = in.Read(buffer, want);
        ASSERT_GT(n, 0u) << "read stopped early at " << got;
        output.insert(output.end(), buffer, buffer + n);
        got += n;
    }
    EXPECT_EQ(got, length);
    EXPECT_EQ(std::memcmp(output.data(), payload.data(), length), 0);

    char tail[4]{};
    EXPECT_EQ(in.Read(tail, sizeof(tail)), 0u);
}

TEST(ZCRCStreams, TruncatedBlockReadReturnsSentinel)
{
    std::vector<char> framing = FrameViaWriter(std::vector<char>{'a', 'b', 'c', 'd', 'e'});
    ASSERT_EQ(framing.size(), 4u + ZCRCInputStream::BLOCK_SIZE + 4u);
    framing.resize(4u + 1000u); // Cut the block payload away.

    MemoryInputStream source(std::move(framing));
    ZCRCInputStream in(&source);

    char output[16]{};
    EXPECT_EQ(in.Read(output, sizeof(output)), static_cast<uint32_t>(-1));
}

TEST(ZCRCStreams, FullBlockWithoutShortTailEndsInErrorSentinel)
{
    // The writer emits no tail block when the payload is a whole number of
    // blocks (same for the PC build), so the reader reports the sentinel when
    // asked for more instead of a clean end.
    const uint32_t length = ZCRCInputStream::BLOCK_SIZE;
    MemoryInputStream source(FrameViaWriter(MakePayload(length)));
    ZCRCInputStream in(&source);

    std::vector<char> full(length);
    EXPECT_EQ(in.Read(full.data(), length), length);

    char next[4]{};
    EXPECT_EQ(in.Read(next, sizeof(next)), static_cast<uint32_t>(-1));
}

TEST(ZCRCInputStream, BlockHeaderIsNotValidated)
{
    std::vector<char> blockData = MakePayload(8);
    std::vector<char> framing = BuildFrame(0xDEADBEEFu, blockData, 5);

    MemoryInputStream source(std::move(framing));
    ZCRCInputStream in(&source);

    char output[8]{};
    EXPECT_EQ(in.Read(output, sizeof(output)), 5u);
    EXPECT_EQ(std::memcmp(output, blockData.data(), 5), 0);
}

TEST(ZCompressedInputStream, DecompressesPayloadAcrossSeveralBufferRefills)
{
    // Incompressible data makes the compressed size larger than the 0x1000
    // staging buffer, forcing multiple refills inside Read.
    const std::vector<char> raw = MakePayload(30000);
    const std::vector<char> compressed = DeflateAll(raw);
    ASSERT_GT(compressed.size(), ZCompressedInputStream::BUFFER_SIZE);

    MemoryInputStream source(compressed);
    ZCompressedInputStream in(&source);

    std::vector<char> output(raw.size() + 16);
    const uint32_t produced = in.Read(output.data(), static_cast<uint32_t>(raw.size()));
    ASSERT_EQ(produced, raw.size());
    EXPECT_EQ(std::memcmp(output.data(), raw.data(), raw.size()), 0);
}

TEST(ZCompressedInputStream, ChunkedReadsReassembleAndEndWithZero)
{
    const std::vector<char> raw = MakePayload(10000);
    MemoryInputStream source(DeflateAll(raw));
    ZCompressedInputStream in(&source);

    std::vector<char> output;
    char buffer[700];
    uint32_t n = 0;
    int guard = 0;
    while ((n = in.Read(buffer, sizeof(buffer))) != 0)
    {
        output.insert(output.end(), buffer, buffer + n);
        ASSERT_LT(++guard, 100);
    }

    ASSERT_EQ(output.size(), raw.size());
    EXPECT_EQ(std::memcmp(output.data(), raw.data(), raw.size()), 0);

    // After the member stream ended, every read stays at zero.
    EXPECT_EQ(in.Read(buffer, sizeof(buffer)), 0u);
}

TEST(ZCompressedInputStream, EmptyInputProducesNothing)
{
    MemoryInputStream source(DeflateAll({}));
    ZCompressedInputStream in(&source);

    char buffer[16]{};
    EXPECT_EQ(in.Read(buffer, 0), 0u);
    EXPECT_EQ(in.Read(buffer, sizeof(buffer)), 0u);
}
