#include <Glacier/Render/Draw/ZRenderDraw.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

using namespace Glacier;

namespace
{
    using ZRenderEntryMap = ZRenderDraw::ZRenderEntryMap;
    using ZEntry = ZRenderEntryMap::ZEntry;

    static_assert(sizeof(ZEntry) == 0xC);
    static_assert(sizeof(ZRenderEntryMap) == 0x4C04);
    static_assert(offsetof(ZRenderEntryMap, m_HashToFirst) == 0x0);
    static_assert(offsetof(ZRenderEntryMap, m_Entries) == 0x400);

    constexpr uint32_t MakeCollidingIdentifier(uint32_t lBase, uint32_t lBucketStep)
    {
        return lBase + lBucketStep * 0x1000;
    }
}

TEST(ZRenderEntryMap, LayoutMatchesPcInstance)
{
    ZRenderEntryMap map;

    EXPECT_EQ(reinterpret_cast<size_t>(&map.m_Entries.m_lCount) - reinterpret_cast<size_t>(&map), 0x4C00u);
    EXPECT_EQ(reinterpret_cast<size_t>(&map.m_Entries.m_lFirstFreeEntry) - reinterpret_cast<size_t>(&map), 0x4C02u);
    EXPECT_EQ(sizeof(map.m_HashToFirst), 1024u);
    EXPECT_EQ(sizeof(map.m_Entries.m_Array), 0x4800u);
}

TEST(ZRenderEntryMap, ConstructorInitializesEmptyFreeListAndBuckets)
{
    ZRenderEntryMap map;

    EXPECT_EQ(map.m_Entries.Count(), 0u);
    EXPECT_EQ(map.m_Entries.m_lFirstFreeEntry, 0u);
    EXPECT_EQ(map.m_Entries.TotalNrEntries(), 1536u);

    for (const ZEntry* bucket : map.m_HashToFirst)
    {
        EXPECT_EQ(bucket, nullptr);
    }
}

TEST(ZRenderEntryMap, HashOfIdentifierUsesByteTwoOfIdentifier)
{
    EXPECT_EQ(ZRenderEntryMap::HashOfIdentifier(0x00000000u), 0x00u);
    EXPECT_EQ(ZRenderEntryMap::HashOfIdentifier(0x00000010u), 0x01u);
    EXPECT_EQ(ZRenderEntryMap::HashOfIdentifier(0x000000FFu), 0x0Fu);
    EXPECT_EQ(ZRenderEntryMap::HashOfIdentifier(0x12345678u), 0x67u);
    EXPECT_EQ(ZRenderEntryMap::HashOfIdentifier(0xFFFFFFFFu), 0xFFu);

    for (uint32_t lIdentifier = 0; lIdentifier < 0x10000; lIdentifier += 0x37)
    {
        const uint32_t lHash = ZRenderEntryMap::HashOfIdentifier(lIdentifier);
        EXPECT_LT(lHash, 256u);
        EXPECT_EQ(lHash, (lIdentifier >> 4) & 0xFFu);
    }
}

TEST(ZRenderEntryMap, AddInsertsEntryAtBucketHead)
{
    ZRenderEntryMap map;

    ZRenderEntry* pRenderEntryA = reinterpret_cast<ZRenderEntry*>(0x11111110);
    ZRenderEntry* pRenderEntryB = reinterpret_cast<ZRenderEntry*>(0x22222220);

    ZEntry* pEntryA = map.Add(0x12345670u, pRenderEntryA);
    ASSERT_NE(pEntryA, nullptr);
    EXPECT_EQ(pEntryA->m_lIndentifier, 0x12345670u);
    EXPECT_EQ(pEntryA->m_pRenderEntry, pRenderEntryA);
    EXPECT_EQ(pEntryA->m_pNext, nullptr);
    EXPECT_EQ(map.m_HashToFirst[ZRenderEntryMap::HashOfIdentifier(0x12345670u)], pEntryA);
    EXPECT_EQ(map.m_Entries.Count(), 1u);

    const uint32_t lIdentifierB = MakeCollidingIdentifier(0x12345670u, 1);
    ZEntry* pEntryB = map.Add(lIdentifierB, pRenderEntryB);
    ASSERT_NE(pEntryB, nullptr);
    EXPECT_EQ(pEntryB->m_pNext, pEntryA);
    EXPECT_EQ(map.m_HashToFirst[ZRenderEntryMap::HashOfIdentifier(0x12345670u)], pEntryB);
    EXPECT_EQ(map.m_Entries.Count(), 2u);
}

TEST(ZRenderEntryMap, AddHandsOutSequentialSlots)
{
    ZRenderEntryMap map;

    ZEntry* pFirst = map.Add(0x10u, reinterpret_cast<ZRenderEntry*>(1));
    ZEntry* pSecond = map.Add(0x20u, reinterpret_cast<ZRenderEntry*>(2));
    ZEntry* pThird = map.Add(0x30u, reinterpret_cast<ZRenderEntry*>(3));

    EXPECT_EQ(pFirst, map.m_Entries.GetEntry(0));
    EXPECT_EQ(pSecond, map.m_Entries.GetEntry(1));
    EXPECT_EQ(pThird, map.m_Entries.GetEntry(2));
    EXPECT_TRUE(map.m_Entries.Validate(pFirst));
    EXPECT_TRUE(map.m_Entries.Validate(pSecond));
    EXPECT_TRUE(map.m_Entries.Validate(pThird));
}

TEST(ZRenderEntryMap, AddReturnsNullptrWhenPoolExhausted)
{
    ZRenderEntryMap map;

    for (int i = 0; i != ZRenderEntryMap::ENTRIES_NR; ++i)
    {
        ASSERT_NE(map.Add(static_cast<uint32_t>(i) * 0x10u, reinterpret_cast<ZRenderEntry*>(i)), nullptr);
    }
    EXPECT_EQ(map.m_Entries.Count(), 1536u);
    EXPECT_EQ(map.Add(0xDEADBEE0u, reinterpret_cast<ZRenderEntry*>(0xDEAD)), nullptr);
    EXPECT_EQ(map.m_Entries.Count(), 1536u);
}

TEST(ZRenderEntryMap, GetAndRemoveReturnsEntryAndUnlinksHead)
{
    ZRenderEntryMap map;

    ZRenderEntry* pRenderEntryA = reinterpret_cast<ZRenderEntry*>(0xAAAAAAAA);
    ZRenderEntry* pRenderEntryB = reinterpret_cast<ZRenderEntry*>(0xBBBBBBBB);

    const uint32_t lIdentifierA = 0x00000010u;
    const uint32_t lIdentifierB = MakeCollidingIdentifier(lIdentifierA, 1);

    ZEntry* pEntryA = map.Add(lIdentifierA, pRenderEntryA);
    ZEntry* pEntryB = map.Add(lIdentifierB, pRenderEntryB);
    ASSERT_NE(pEntryA, nullptr);
    ASSERT_NE(pEntryB, nullptr);

    EXPECT_EQ(map.GetAndRemove(lIdentifierB), pRenderEntryB);
    EXPECT_EQ(map.m_Entries.Count(), 1u);
    EXPECT_EQ(map.m_HashToFirst[0x1], pEntryA);
    EXPECT_EQ(pEntryA->m_pNext, nullptr);
    EXPECT_FALSE(map.m_Entries.Validate(pEntryB));

    EXPECT_EQ(map.GetAndRemove(lIdentifierA), pRenderEntryA);
    EXPECT_EQ(map.m_Entries.Count(), 0u);
    EXPECT_EQ(map.m_HashToFirst[0x1], nullptr);
}

TEST(ZRenderEntryMap, GetAndRemoveUnlinksMiddleOfChain)
{
    ZRenderEntryMap map;

    ZRenderEntry* pRenderEntryA = reinterpret_cast<ZRenderEntry*>(0x1);
    ZRenderEntry* pRenderEntryB = reinterpret_cast<ZRenderEntry*>(0x2);
    ZRenderEntry* pRenderEntryC = reinterpret_cast<ZRenderEntry*>(0x3);

    const uint32_t lIdentifierA = 0x00000050u;
    const uint32_t lIdentifierB = MakeCollidingIdentifier(lIdentifierA, 1);
    const uint32_t lIdentifierC = MakeCollidingIdentifier(lIdentifierA, 2);

    ZEntry* pEntryA = map.Add(lIdentifierA, pRenderEntryA);
    ZEntry* pEntryB = map.Add(lIdentifierB, pRenderEntryB);
    ZEntry* pEntryC = map.Add(lIdentifierC, pRenderEntryC);
    ASSERT_NE(pEntryA, nullptr);
    ASSERT_NE(pEntryB, nullptr);
    ASSERT_NE(pEntryC, nullptr);

    EXPECT_EQ(map.GetAndRemove(lIdentifierB), pRenderEntryB);
    EXPECT_EQ(map.m_HashToFirst[0x5], pEntryC);
    EXPECT_EQ(pEntryC->m_pNext, pEntryA);

    EXPECT_EQ(map.GetAndRemove(lIdentifierC), pRenderEntryC);
    EXPECT_EQ(map.m_HashToFirst[0x5], pEntryA);

    EXPECT_EQ(map.GetAndRemove(lIdentifierA), pRenderEntryA);
    EXPECT_EQ(map.m_HashToFirst[0x5], nullptr);
    EXPECT_EQ(map.m_Entries.Count(), 0u);
}

TEST(ZRenderEntryMap, GetAndRemoveReturnsNullptrForMissingIdentifier)
{
    ZRenderEntryMap map;

    EXPECT_EQ(map.GetAndRemove(0x12345670u), nullptr);

    const uint32_t lIdentifier = 0x00000020u;
    ASSERT_NE(map.Add(lIdentifier, reinterpret_cast<ZRenderEntry*>(0x20)), nullptr);

    EXPECT_EQ(map.GetAndRemove(lIdentifier + 1), nullptr);
    EXPECT_EQ(map.GetAndRemove(MakeCollidingIdentifier(lIdentifier, 5)), nullptr);
    EXPECT_EQ(map.m_Entries.Count(), 1u);
    EXPECT_EQ(map.GetAndRemove(lIdentifier), reinterpret_cast<ZRenderEntry*>(0x20));
}

TEST(ZRenderEntryMap, RemovedSlotIsReusedInLifoOrder)
{
    ZRenderEntryMap map;

    ZEntry* pEntryA = map.Add(0x00000030u, reinterpret_cast<ZRenderEntry*>(0x1));
    ZEntry* pEntryB = map.Add(0x00000040u, reinterpret_cast<ZRenderEntry*>(0x2));
    ASSERT_NE(pEntryA, nullptr);
    ASSERT_NE(pEntryB, nullptr);
    EXPECT_EQ(map.m_Entries.m_lFirstFreeEntry, 2u);

    ASSERT_EQ(map.GetAndRemove(0x00000030u), reinterpret_cast<ZRenderEntry*>(0x1));
    EXPECT_EQ(map.m_Entries.m_lFirstFreeEntry, 0u);

    ZEntry* pEntryC = map.Add(0x00000050u, reinterpret_cast<ZRenderEntry*>(0x3));
    ASSERT_NE(pEntryC, nullptr);
    EXPECT_EQ(pEntryC, pEntryA);
    EXPECT_TRUE(map.m_Entries.Validate(pEntryB));
}

TEST(ZRenderEntryMap, AddGetAndRemoveRoundTripWithManyEntries)
{
    ZRenderEntryMap map;

    constexpr uint32_t COUNT = 512;
    for (uint32_t i = 0; i < COUNT; ++i)
    {
        const uint32_t lIdentifier = i * 0x1000u + (i & 0xFu) * 0x10u + 7u;
        ASSERT_NE(map.Add(lIdentifier, reinterpret_cast<ZRenderEntry*>(0x1000u + i)), nullptr);
    }
    EXPECT_EQ(map.m_Entries.Count(), COUNT);

    for (uint32_t i = 0; i < COUNT; ++i)
    {
        const uint32_t lIdentifier = i * 0x1000u + (i & 0xFu) * 0x10u + 7u;
        ASSERT_EQ(map.GetAndRemove(lIdentifier), reinterpret_cast<ZRenderEntry*>(0x1000u + i));
    }
    EXPECT_EQ(map.m_Entries.Count(), 0u);

    for (const ZEntry* bucket : map.m_HashToFirst)
    {
        EXPECT_EQ(bucket, nullptr);
    }

    // Free-list is LIFO: the last removed slot (COUNT-1) becomes the head again
    EXPECT_EQ(map.Add(0x00000AB0u, reinterpret_cast<ZRenderEntry*>(0xFFFF)), map.m_Entries.GetEntry(COUNT - 1));
}
