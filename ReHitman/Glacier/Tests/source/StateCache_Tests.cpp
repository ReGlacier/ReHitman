#include <Glacier/Animation/Manager.h>
#include <gtest/gtest.h>

using namespace Glacier::Animation;

TEST(StateCache, FindsHeadAndLinkedEntriesAndTracksHits)
{
    StateCache cache(1, 64);

    StateCacheEntry* first = cache.AllocEntry(0, 3);
    ASSERT_NE(first, nullptr);
    StateCacheEntry* second = cache.AllocEntry(0, 7);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(cache.FindEntry(0, 7), second);
    EXPECT_EQ(cache.FindEntry(0, 3), first);
    EXPECT_EQ(cache.m_Hits, 2);
    EXPECT_EQ(cache.m_Misses, 0);
    EXPECT_EQ(cache.FindEntry(0, 11), nullptr);
    EXPECT_EQ(cache.m_Hits, 2);
    EXPECT_EQ(cache.m_Misses, 1);
}

TEST(StateCache, UpdateReclaimsUnusedEntries)
{
    StateCache cache(1, 64);
    StateCacheEntry* entry = cache.AllocEntry(0, 3);
    ASSERT_NE(entry, nullptr);

    // A newly allocated entry is used during this frame and survives one update.
    EXPECT_EQ(cache.FindEntry(0, 3), entry);
    cache.Update();
    EXPECT_EQ(cache.m_CacheEntry[0], entry);

    cache.Update();
    EXPECT_EQ(cache.m_CacheEntry[0], nullptr);
    EXPECT_EQ(cache.m_AllocPos, 0);
    entry = cache.AllocEntry(0, 3);
    EXPECT_NE(entry, nullptr);
}
