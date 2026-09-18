#include <Glacier/Serializer/IInputStream.h>
#include <Glacier/Serializer/ZInputStream.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>

using namespace Glacier;

namespace
{
    struct MemoryInputStream final : public IInputStream
    {
        std::vector<char> Bytes;
        size_t Offset{0};
        uint32_t MaxReadSize{0};
        uint32_t ReadCalls{0};

        MemoryInputStream(std::vector<char> bytes, uint32_t maxReadSize = ZInputStream::MAX_BUFFER_SIZE)
            : Bytes(std::move(bytes))
            , MaxReadSize(maxReadSize)
        {
        }

        uint32_t Read(void* address, const uint32_t size) override
        {
            ++ReadCalls;

            const uint32_t remaining = static_cast<uint32_t>(Bytes.size() - Offset);
            const uint32_t readSize = std::min({size, MaxReadSize, remaining});

            if (readSize != 0)
                std::memcpy(address, Bytes.data() + Offset, readSize);

            Offset += readSize;
            return readSize;
        }
    };
}

TEST(ZInputStream, ReadRawReadsAcrossMultipleBlocks)
{
    MemoryInputStream memory({ 'a', 'b', 'c', 'd', 'e' }, 2);
    ZInputStream stream(memory);

    char output[5]{};
    const uint32_t bytesRead = stream.ReadRaw(output, sizeof(output));

    EXPECT_EQ(bytesRead, 5u);
    EXPECT_EQ(std::memcmp(output, "abcde", sizeof(output)), 0);
    EXPECT_EQ(memory.ReadCalls, 3u);
    EXPECT_EQ(stream.m_BufferPtr, 1u);
    EXPECT_EQ(stream.m_BufferEnd, 1u);
}

TEST(ZInputStream, ReadRawReturnsPartialCountAtEndOfStream)
{
    MemoryInputStream memory({ 'x', 'y', 'z' }, 2);
    ZInputStream stream(memory);

    char output[5]{};
    const uint32_t bytesRead = stream.ReadRaw(output, sizeof(output));

    EXPECT_EQ(bytesRead, 3u);
    EXPECT_EQ(std::memcmp(output, "xyz", 3), 0);
    EXPECT_EQ(output[3], '\0');
    EXPECT_EQ(output[4], '\0');
    EXPECT_EQ(memory.ReadCalls, 3u);
}

TEST(ZInputStream, ReadRawConsumesBufferedBytesBeforeRefill)
{
    MemoryInputStream memory({ 'a', 'b', 'c', 'd' }, 4);
    ZInputStream stream(memory);

    char first[2]{};
    char second[2]{};

    EXPECT_EQ(stream.ReadRaw(first, sizeof(first)), 2u);
    EXPECT_EQ(stream.ReadRaw(second, sizeof(second)), 2u);

    EXPECT_EQ(std::memcmp(first, "ab", sizeof(first)), 0);
    EXPECT_EQ(std::memcmp(second, "cd", sizeof(second)), 0);
    EXPECT_EQ(memory.ReadCalls, 1u);
}

TEST(ZInputStream, ReadChangeEndiannessRemapsDestinationOffsets)
{
    MemoryInputStream memory({ 1, 2, 3, 4 }, 2);
    ZInputStream stream(memory);

    char output[4]{};
    const uint32_t bytesRead = stream.ReadChangeEndianness(output, sizeof(output), 3u);

    EXPECT_EQ(bytesRead, 4u);
    EXPECT_EQ(output[0], 4);
    EXPECT_EQ(output[1], 3);
    EXPECT_EQ(output[2], 2);
    EXPECT_EQ(output[3], 1);
    EXPECT_EQ(memory.ReadCalls, 2u);
}

TEST(ZInputStream, ReadNextBlockRefillsBufferAndReportsEof)
{
    MemoryInputStream memory({ 'q' }, 1);
    ZInputStream stream(memory);

    EXPECT_TRUE(stream.ReadNextBlock());
    EXPECT_EQ(stream.m_BufferPtr, 0u);
    EXPECT_EQ(stream.m_BufferEnd, 1u);
    EXPECT_EQ(stream.m_Buffer[0], 'q');

    EXPECT_FALSE(stream.ReadNextBlock());
    EXPECT_EQ(stream.m_BufferPtr, 0u);
    EXPECT_EQ(stream.m_BufferEnd, 0u);
}
