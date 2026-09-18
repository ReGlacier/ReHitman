#include <Glacier/Filesystem/CFastLookupFileCache_t.h>
#include <gtest/gtest.h>

#include <cstdint>

using namespace Glacier;

namespace
{
    IOZip_LocalFileHeader_t MakeHeader(uint32_t crc, uint32_t compressedSize, uint32_t uncompressedSize)
    {
        IOZip_LocalFileHeader_t header{};
        header.zipBody.versionExtract = 20;
        header.zipBody.generalPurpose = 1;
        header.zipBody.compressionMethod = 8;
        header.zipBody.lastModFileTime = 0x1234;
        header.zipBody.lastModFileDate = 0x5678;
        header.zipBody.crc32 = crc;
        header.zipBody.compressedSize = compressedSize;
        header.zipBody.uncompressedSize = uncompressedSize;
        header.zipBody.filenameLength = 9;
        header.zipBody.extraFieldLength = 3;
        return header;
    }

    void ExpectHeadersEqual(const IOZip_LocalFileHeader_t& lhs, const IOZip_LocalFileHeader_t& rhs)
    {
        EXPECT_EQ(lhs.zipBody.versionExtract, rhs.zipBody.versionExtract);
        EXPECT_EQ(lhs.zipBody.generalPurpose, rhs.zipBody.generalPurpose);
        EXPECT_EQ(lhs.zipBody.compressionMethod, rhs.zipBody.compressionMethod);
        EXPECT_EQ(lhs.zipBody.lastModFileTime, rhs.zipBody.lastModFileTime);
        EXPECT_EQ(lhs.zipBody.lastModFileDate, rhs.zipBody.lastModFileDate);
        EXPECT_EQ(lhs.zipBody.crc32, rhs.zipBody.crc32);
        EXPECT_EQ(lhs.zipBody.compressedSize, rhs.zipBody.compressedSize);
        EXPECT_EQ(lhs.zipBody.uncompressedSize, rhs.zipBody.uncompressedSize);
        EXPECT_EQ(lhs.zipBody.filenameLength, rhs.zipBody.filenameLength);
        EXPECT_EQ(lhs.zipBody.extraFieldLength, rhs.zipBody.extraFieldLength);
    }
}

TEST(CFastLookupFileCache, AddAndLookupRoundTripHeaderAndFilePosition)
{
    CFastLookupFileCache_t cache;
    IOZip_LocalFileHeader_t input = MakeHeader(0xAABBCCDD, 100, 200);
    IOZip_LocalFileHeader_t output{};
    int filePos = 0;

    cache.add("Data/Actors/Hero.zip", &input, 12345);

    ASSERT_TRUE(cache.lookup("Data/Actors/Hero.zip", &output, &filePos));
    EXPECT_EQ(filePos, 12345);
    ExpectHeadersEqual(output, input);
    EXPECT_EQ(cache.m_rtActiveCacheInfo.Count(), 1);
}

TEST(CFastLookupFileCache, LookupIsCaseInsensitive)
{
    CFastLookupFileCache_t cache;
    IOZip_LocalFileHeader_t input = MakeHeader(1, 2, 3);
    IOZip_LocalFileHeader_t output{};
    int filePos = 0;

    cache.add("Scenes/Mission01.ZIP", &input, 77);

    ASSERT_TRUE(cache.lookup("scenes/mission01.zip", &output, &filePos));
    EXPECT_EQ(filePos, 77);
    ExpectHeadersEqual(output, input);
}

TEST(CFastLookupFileCache, LookupMissingFileReturnsFalse)
{
    CFastLookupFileCache_t cache;
    IOZip_LocalFileHeader_t output{};
    int filePos = -1;

    EXPECT_FALSE(cache.lookup("missing.zip", &output, &filePos));
    EXPECT_EQ(filePos, -1);
}

TEST(CFastLookupFileCache, InvalidateRemovesLookupEntry)
{
    CFastLookupFileCache_t cache;
    IOZip_LocalFileHeader_t input = MakeHeader(10, 20, 30);
    IOZip_LocalFileHeader_t output{};
    int filePos = 0;

    cache.add("file.bin", &input, 44);
    ASSERT_TRUE(cache.lookup("FILE.BIN", &output, &filePos));

    cache.invalidate("File.Bin");

    EXPECT_FALSE(cache.lookup("file.bin", &output, &filePos));
}

TEST(CFastLookupFileCache, AddingSameFileReplacesLookupWithLatestBlock)
{
    CFastLookupFileCache_t cache;
    IOZip_LocalFileHeader_t first = MakeHeader(1, 10, 100);
    IOZip_LocalFileHeader_t second = MakeHeader(2, 20, 200);
    IOZip_LocalFileHeader_t output{};
    int filePos = 0;

    cache.add("same.bin", &first, 11);
    cache.add("same.bin", &second, 22);

    ASSERT_TRUE(cache.lookup("same.bin", &output, &filePos));
    EXPECT_EQ(filePos, 22);
    ExpectHeadersEqual(output, second);
    EXPECT_EQ(cache.m_rtActiveCacheInfo.Count(), 2);
}

TEST(CFastLookupFileCache, FlushRemovesLookupEntries)
{
    auto* cache = new CFastLookupFileCache_t();
    IOZip_LocalFileHeader_t input = MakeHeader(3, 30, 300);
    IOZip_LocalFileHeader_t output{};
    int filePos = 0;

    cache->add("flush.bin", &input, 333);
    ASSERT_TRUE(cache->lookup("flush.bin", &output, &filePos));

    cache->flush();

    EXPECT_FALSE(cache->lookup("flush.bin", &output, &filePos));
    EXPECT_EQ(cache->m_rtActiveCacheInfo.Count(), 0);

    delete cache;
}
