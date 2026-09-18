#include <Glacier/ZSTL/CFastLookup2.h>
#include <Glacier/ZSTL/LINKREFTAB.h>
#include <Glacier/ZSTL/ZValTree.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>

using namespace Glacier;

TEST(CFastLookup2, CalcChkSumMatchesOriginalPackedDwordAlgorithm)
{
    CFastLookup2 lookup(4);

    EXPECT_EQ(lookup.CalcChkSum("", 0), 0);
    EXPECT_EQ(lookup.CalcChkSum("abc", 3), 3 + 'a' + 'b' + 'c');
    EXPECT_EQ(lookup.CalcChkSum("abcd", 4), 4 + 0x64636261);
    EXPECT_EQ(lookup.CalcChkSum("abcde", 5), 5 + 0x64636261 + 'e');
}

TEST(CFastLookup2, SetAndGetRoundTripValues)
{
    CFastLookup2 lookup(4);

    lookup.Set("alpha", 10);
    lookup.Set("beta", 20);

    EXPECT_EQ(lookup.Get("alpha"), 10u);
    EXPECT_EQ(lookup.Get("beta"), 20u);
    EXPECT_EQ(lookup.Get("missing"), 0u);
    EXPECT_TRUE(lookup.m_bRuntime);
    EXPECT_FALSE(lookup.m_bStatic);
}

TEST(CFastLookup2, SetReplacesExistingKeyWithoutCreatingDuplicateLink)
{
    CFastLookup2 lookup(4);

    lookup.Set("same", 10);
    lookup.Set("same", 99);

    EXPECT_EQ(lookup.Get("same"), 99u);

    const int checksum = lookup.CalcChkSum("same", 4);
    auto* list = reinterpret_cast<LINKREFTAB*>(lookup.m_pChkSumTree->GetKeyVal(checksum));
    ASSERT_NE(list, nullptr);
    EXPECT_EQ(list->Count(), 1);
}

TEST(CFastLookup2, HandlesChecksumCollisionsByComparingStringAndLength)
{
    CFastLookup2 lookup(4);

    ASSERT_EQ(lookup.CalcChkSum("ab", 2), lookup.CalcChkSum("ba", 2));

    lookup.Set("ab", 12);
    lookup.Set("ba", 34);

    EXPECT_EQ(lookup.Get("ab"), 12u);
    EXPECT_EQ(lookup.Get("ba"), 34u);
}

TEST(CFastLookup2, RemoveDeletesOnlyRequestedCollidingEntry)
{
    CFastLookup2 lookup(4);

    lookup.Set("ab", 12);
    lookup.Set("ba", 34);

    lookup.Remove("ab", 0);

    EXPECT_EQ(lookup.Get("ab"), 0u);
    EXPECT_EQ(lookup.Get("ba"), 34u);

    lookup.Remove("ba", 2);
    EXPECT_EQ(lookup.Get("ba"), 0u);
}

TEST(CFastLookup2, LowerCaseHelpersNormalizeLookupBeforeOperation)
{
    CFastLookup2 lookup(4);

    lookup.SetLowerCase("MiXeD/Path.TXT", 77);

    EXPECT_EQ(lookup.GetLowerCase("mixed/path.txt"), 77);
    EXPECT_EQ(lookup.GetLowerCase("MIXED/PATH.TXT"), 77);

    lookup.RemoveLowerCase("Mixed/Path.Txt");
    EXPECT_EQ(lookup.GetLowerCase("mixed/path.txt"), 0);
}

TEST(CFastLookup2, ClearAndResetRemoveAllEntriesAndResetFlags)
{
    CFastLookup2 lookup(4);

    lookup.Set("one", 1);
    lookup.Set("two", 2);

    lookup.Clear();

    EXPECT_EQ(lookup.Get("one"), 0u);
    EXPECT_EQ(lookup.Get("two"), 0u);
    EXPECT_FALSE(lookup.m_bStatic);
    EXPECT_FALSE(lookup.m_bRuntime);

    lookup.Set("three", 3);
    lookup.Reset();

    EXPECT_EQ(lookup.Get("three"), 0u);
    EXPECT_FALSE(lookup.m_bRuntime);
}

TEST(CFastLookup2, FindFastLinkReturnsMatchingRecord)
{
    CFastLookup2 lookup(4);

    lookup.Set("target", 123);
    const int checksum = lookup.CalcChkSum("target", static_cast<int>(std::strlen("target")));
    auto* list = reinterpret_cast<LINKREFTAB*>(lookup.m_pChkSumTree->GetKeyVal(checksum));

    ASSERT_NE(list, nullptr);
    SFastLookupLink* link = lookup.FindFastLink(list, "target", static_cast<int>(std::strlen("target")));

    ASSERT_NE(link, nullptr);
    EXPECT_STREQ(link->m_pLookup, "target");
    EXPECT_EQ(link->m_lLookupLen, 6);
    EXPECT_EQ(link->m_lValue, 123u);
    EXPECT_EQ(lookup.FindFastLink(list, "targe", 5), nullptr);
}
