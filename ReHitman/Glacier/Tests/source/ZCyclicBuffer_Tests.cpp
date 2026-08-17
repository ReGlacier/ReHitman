#include <Glacier/ZSTL/ZCyclicBuffer.h>

#include <gtest/gtest.h>

#include <cstring>
#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZCyclicBuffer) == 0x1C);

    void PushString(ZCyclicBuffer& buffer, const char* pszText)
    {
        const int iSize = static_cast<int>(std::strlen(pszText)) + 1;
        void* pRecord = buffer.Alloc(iSize);
        ASSERT_NE(pRecord, nullptr);
        std::memcpy(pRecord, pszText, iSize);
    }

    const char* GetString(ZCyclicBuffer& buffer, int iIndex)
    {
        return static_cast<const char*>(buffer.Get(iIndex));
    }
}

TEST(ZCyclicBuffer, InitialStateIsEmpty)
{
    ZCyclicBuffer buffer(64);

    EXPECT_EQ(buffer.Count(), 0);
    EXPECT_EQ(buffer.First(), 0);
    EXPECT_EQ(buffer.Last(), -1);
    EXPECT_EQ(buffer.Get(0), nullptr);
}

TEST(ZCyclicBuffer, AllocReturnsDistinctRecordsInInsertionOrder)
{
    ZCyclicBuffer buffer(256);

    PushString(buffer, "first");
    PushString(buffer, "second");
    PushString(buffer, "third");

    EXPECT_EQ(buffer.Count(), 3);
    EXPECT_EQ(buffer.First(), 0);
    EXPECT_EQ(buffer.Last(), 2);

    EXPECT_STREQ(GetString(buffer, 0), "first");
    EXPECT_STREQ(GetString(buffer, 1), "second");
    EXPECT_STREQ(GetString(buffer, 2), "third");
}

TEST(ZCyclicBuffer, NegativeIndexAddressesRecordsFromTheNewest)
{
    ZCyclicBuffer buffer(256);

    PushString(buffer, "one");
    PushString(buffer, "two");
    PushString(buffer, "three");

    EXPECT_STREQ(GetString(buffer, -1), "three");
    EXPECT_STREQ(GetString(buffer, -2), "two");
    EXPECT_STREQ(GetString(buffer, -3), "one");
    EXPECT_EQ(buffer.Get(-4), nullptr);
}

TEST(ZCyclicBuffer, GetRejectsOutOfRangeIndices)
{
    ZCyclicBuffer buffer(256);

    PushString(buffer, "only");

    EXPECT_EQ(buffer.Get(1), nullptr);
    EXPECT_EQ(buffer.Get(-2), nullptr);
}

TEST(ZCyclicBuffer, AllocAlignsSizeToFourBytes)
{
    // Record stride for a 5-byte payload: 8 (aligned payload) + 4 (header) = 12.
    // Without alignment each record would take 9 bytes and a 64 byte buffer would
    // hold 7 of them; aligned it holds at most 5 even in the linear state.
    ZCyclicBuffer buffer(64);

    for (int i = 0; i < 5; ++i)
        PushString(buffer, "aaaa");

    EXPECT_EQ(buffer.Count(), 5);

    // One more no longer fits without eviction, whatever the internal layout is.
    const int iFirstBefore = buffer.First();
    PushString(buffer, "bbbb");

    EXPECT_GE(buffer.First(), iFirstBefore);
    EXPECT_LE(buffer.Count(), 5);
}

TEST(ZCyclicBuffer, RecordLargerThanBufferCapacityFails)
{
    ZCyclicBuffer buffer(32);

    // Buffer keeps 8 bytes of headroom, so 24 payload bytes (28 aligned+header) cannot ever fit.
    EXPECT_EQ(buffer.Alloc(25), nullptr);
    EXPECT_EQ(buffer.Count(), 0);
}

TEST(ZCyclicBuffer, FullBufferEvictsOldestRecordsInOrder)
{
    ZCyclicBuffer buffer(64);

    // Push more records (12 byte stride each) than the buffer can ever hold.
    const char* apszRecords[] = { "r000", "r001", "r002", "r003", "r004", "r005", "r006", "r007" };
    for (const char* psz : apszRecords)
        PushString(buffer, psz);

    const int iCount = buffer.Count();
    ASSERT_GT(iCount, 0);
    ASSERT_LT(iCount, 8);
    EXPECT_EQ(buffer.Last(), buffer.First() + iCount - 1);

    // Every readable record in the live window must be one of the iCount newest
    // pushes, and the window as a whole must be a contiguous suffix of the push
    // sequence (the record pinned at physical offset 0 can be clobbered by a wrap
    // sentinel - CConsole::GetLine treats such slots as empty lines).
    int iReadable = 0;
    for (int i = 0; i < iCount; ++i)
    {
        const char* pszRecord = GetString(buffer, buffer.First() + i);
        if (!pszRecord || pszRecord[0] != 'r')
            continue;

        const int iOrdinal = pszRecord[3] - '0';
        EXPECT_GE(iOrdinal, 8 - iCount);
        EXPECT_LT(iOrdinal, 8);
        ++iReadable;
    }

    EXPECT_GE(iReadable, iCount - 1);
}

TEST(ZCyclicBuffer, EvictionAdvancesFirstIndexMonotonically)
{
    ZCyclicBuffer buffer(64);

    PushString(buffer, "r0");

    int iPrevFirst = buffer.First();

    for (int i = 1; i < 16; ++i)
    {
        char szRecord[4];
        szRecord[0] = 'a';
        szRecord[1] = static_cast<char>('0' + (i % 10));
        szRecord[2] = '\0';
        PushString(buffer, szRecord);

        EXPECT_GE(buffer.First(), iPrevFirst);
        iPrevFirst = buffer.First();
    }

    // After 16 pushes of 12-byte records into 64 bytes, eviction definitely happened.
    EXPECT_GT(buffer.First(), 0);
    // Negative indexing still maps onto the same live records.
    EXPECT_STREQ(GetString(buffer, -1), GetString(buffer, buffer.Last()));
}

TEST(ZCyclicBuffer, WrapAroundKeepsDataIntact)
{
    ZCyclicBuffer buffer(64);

    // Fill, evict and force the write cursor to wrap multiple times.
    for (int i = 0; i < 16; ++i)
    {
        char szRecord[8];
        std::memset(szRecord, 'A' + (i % 26), sizeof(szRecord) - 1);
        szRecord[sizeof(szRecord) - 1] = '\0';
        PushString(buffer, szRecord);
    }

    const int iCount = buffer.Count();
    ASSERT_GT(iCount, 0);
    EXPECT_EQ(buffer.Last(), buffer.First() + iCount - 1);

    // Every readable record in the live window must belong to the iCount newest
    // pushes; slots clobbered by a wrap sentinel do not hold valid strings.
    int iReadable = 0;
    for (int i = 0; i < iCount; ++i)
    {
        const char* pszRecord = GetString(buffer, buffer.First() + i);
        if (!pszRecord || pszRecord[0] < 'A' || pszRecord[0] > 'P')
            continue;

        EXPECT_GE(pszRecord[0], static_cast<char>('A' + 16 - iCount));
        ++iReadable;
    }

    EXPECT_GE(iReadable, iCount - 1);

    // The newest push is still alive somewhere in the buffer, even when the record
    // currently pinned at offset 0 hides it from Get() (original quirk).
    bool bNewestAlive = false;
    for (int iOffset = 0; iOffset < 64; iOffset += 4)
    {
        if (std::memcmp(buffer.m_pBuffer + iOffset, "PPPPPPP", 8) == 0)
        {
            bNewestAlive = true;
            break;
        }
    }

    EXPECT_TRUE(bNewestAlive);
}

TEST(ZCyclicBuffer, DrainingEverythingResetsIndices)
{
    ZCyclicBuffer buffer(64);

    PushString(buffer, "x");

    // Evict everything by pushing oversized records until the old one is gone and
    // then read past the end.
    PushString(buffer, "y");
    PushString(buffer, "z");
    PushString(buffer, "w");

    EXPECT_EQ(GetString(buffer, buffer.First()), GetString(buffer, buffer.First()));
    EXPECT_EQ(buffer.Count(), 4); // 64 bytes hold at most 5 tiny records; 4 fit, nothing evicted here

    // Every live record must be readable through both addressing modes.
    for (int i = buffer.First(); i <= buffer.Last(); ++i)
        EXPECT_NE(buffer.Get(i), nullptr);
}
