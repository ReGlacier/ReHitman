#include <Glacier/Serializer/ZMemoryOutputStream.h>
#include <Glacier/Serializer/IOutputStream.h>
#include <Glacier/Serializer/ZOutputStream.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

using namespace Glacier;

namespace
{
    struct MemoryOutputStream final : public IOutputStream
    {
        std::vector<uint8_t> Bytes;
        std::vector<uint32_t> WriteSizes;

        uint32_t Write(const void* pAddr, const uint32_t lSize) override
        {
            WriteSizes.push_back(lSize);

            const auto* bytes = static_cast<const uint8_t*>(pAddr);
            Bytes.insert(Bytes.end(), bytes, bytes + lSize);
            return lSize;
        }
    };

    std::vector<uint8_t> MakePattern(size_t size)
    {
        std::vector<uint8_t> bytes(size);

        for (size_t i = 0; i < bytes.size(); ++i)
            bytes[i] = static_cast<uint8_t>(i & 0xFFu);

        return bytes;
    }
}

TEST(ZOutputStream, WriteRawBuffersSmallWritesUntilFlush)
{
    MemoryOutputStream memory;
    ZOutputStream stream(memory);
    char bytes[] = { 'a', 'b', 'c' };

    EXPECT_EQ(stream.WriteRaw(bytes, sizeof(bytes)), 3u);
    EXPECT_TRUE(memory.Bytes.empty());
    EXPECT_EQ(stream.m_BufferPtr, 3u);

    EXPECT_EQ(stream.FlushBuffer(), 3);
    EXPECT_EQ(memory.Bytes, (std::vector<uint8_t>{ 'a', 'b', 'c' }));
    EXPECT_EQ(stream.m_BufferPtr, 0u);
}

TEST(ZOutputStream, WriteRawFlushesPendingBytesBeforeLargeWrite)
{
    MemoryOutputStream memory;
    ZOutputStream stream(memory);
    char prefix[] = { 'x', 'y' };
    std::vector<uint8_t> largeBytes = MakePattern(0x400);

    EXPECT_EQ(stream.WriteRaw(prefix, sizeof(prefix)), 2u);
    EXPECT_EQ(stream.WriteRaw(reinterpret_cast<char*>(largeBytes.data()), static_cast<uint32_t>(largeBytes.size())), largeBytes.size());

    ASSERT_EQ(memory.WriteSizes.size(), 2u);
    EXPECT_EQ(memory.WriteSizes[0], 2u);
    EXPECT_EQ(memory.WriteSizes[1], largeBytes.size());
    ASSERT_EQ(memory.Bytes.size(), sizeof(prefix) + largeBytes.size());
    EXPECT_EQ(memory.Bytes[0], 'x');
    EXPECT_EQ(memory.Bytes[1], 'y');
    EXPECT_TRUE(std::equal(largeBytes.begin(), largeBytes.end(), memory.Bytes.begin() + sizeof(prefix)));
    EXPECT_EQ(stream.m_BufferPtr, 0u);
}

TEST(ZOutputStream, WriteChangeEndiannessRemapsSourceOffsets)
{
    MemoryOutputStream memory;
    ZOutputStream stream(memory);
    char bytes[] = { 1, 2, 3, 4 };

    EXPECT_EQ(stream.WriteChangeEndianness(bytes, sizeof(bytes), 3u), 4u);
    EXPECT_EQ(stream.FlushBuffer(), 4);

    EXPECT_EQ(memory.Bytes, (std::vector<uint8_t>{ 4, 3, 2, 1 }));
}

TEST(ZMemoryOutputStream, WriteRawStoresBytesAcrossPagesAndCopyToPreservesData)
{
    const std::vector<uint8_t> bytes = MakePattern(ZMemoryOutputStream::ZHeader::m_PageSize + 5u);
    ZMemoryOutputStream memoryStream;

    EXPECT_NE(memoryStream.m_First, nullptr);
    EXPECT_EQ(memoryStream.m_First, memoryStream.m_Last);
    EXPECT_EQ(memoryStream.m_Index, 0u);

    memoryStream.WriteRaw(reinterpret_cast<char*>(const_cast<uint8_t*>(bytes.data())), static_cast<uint32_t>(bytes.size()));

    ASSERT_NE(memoryStream.m_First->m_Next, nullptr);
    EXPECT_EQ(memoryStream.m_Last, memoryStream.m_First->m_Next);
    EXPECT_EQ(memoryStream.m_Index, 5u);

    MemoryOutputStream outputMemory;
    ZOutputStream outputStream(outputMemory);
    memoryStream.CopyTo(outputStream);
    outputStream.FlushBuffer();

    EXPECT_EQ(outputMemory.Bytes, bytes);
}

TEST(ZMemoryOutputStream, WriteChangeEndiannessRemapsSourceOffsets)
{
    ZMemoryOutputStream stream;
    char bytes[] = { 1, 2, 3, 4 };

    EXPECT_EQ(stream.WriteChangeEndianness(bytes, sizeof(bytes), 3u), 4u);

    ASSERT_NE(stream.m_First, nullptr);
    EXPECT_EQ(stream.m_Index, 4u);
    EXPECT_EQ(stream.m_First->m_Data[0], 4u);
    EXPECT_EQ(stream.m_First->m_Data[1], 3u);
    EXPECT_EQ(stream.m_First->m_Data[2], 2u);
    EXPECT_EQ(stream.m_First->m_Data[3], 1u);
}

TEST(ZMemoryOutputStream, IOutputStreamWriteAppendsRawBytes)
{
    ZMemoryOutputStream stream;
    const uint8_t bytes[] = { 'o', 'k' };

    EXPECT_EQ(stream.Write(bytes, sizeof(bytes)), 2u);

    ASSERT_NE(stream.m_First, nullptr);
    EXPECT_EQ(stream.m_Index, 2u);
    EXPECT_EQ(stream.m_First->m_Data[0], 'o');
    EXPECT_EQ(stream.m_First->m_Data[1], 'k');
}
