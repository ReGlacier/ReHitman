#include <Glacier/ZSTL/ZCompletionList.h>

#include <gtest/gtest.h>

#include <cstring>
#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZCompletionList) == 0x18);

    int StrNCaseCmp(const char* pszLhs, const char* pszRhs, size_t iCount)
    {
#ifdef _WIN32
        return _strnicmp(pszLhs, pszRhs, iCount);
#else
        return strncasecmp(pszLhs, pszRhs, iCount);
#endif
    }
}

TEST(ZCompletionList, InitialStateIsEmpty)
{
    ZCompletionList list(256);

    EXPECT_EQ(list.Count(), 0);
}

TEST(ZCompletionList, AddStoresNewestEntryFirst)
{
    ZCompletionList list(256);

    list.Add("beta");
    list.Add("alpha");
    list.Add("gamma");

    // The pointer table grows downwards: the newest entry sits in slot 0 until
    // Sort()/Unique() reorder the table.
    EXPECT_EQ(list.Count(), 3);
    EXPECT_STREQ(list.Get(0), "gamma");
    EXPECT_STREQ(list.Get(1), "alpha");
    EXPECT_STREQ(list.Get(2), "beta");
}

TEST(ZCompletionList, AddFiltersByPrefixCaseInsensitively)
{
    ZCompletionList list(256);

    char szPrefix[] = "sh";
    list.Prefix(szPrefix);

    list.Add("show_debug");
    list.Add("SHOW_FPS");
    list.Add("hide_all");

    EXPECT_EQ(list.Count(), 2);
    EXPECT_STREQ(list.Get(0), "SHOW_FPS");
    EXPECT_STREQ(list.Get(1), "show_debug");
}

TEST(ZCompletionList, ClearResetsCountAndPrefix)
{
    ZCompletionList list(256);

    char szPrefix[] = "ab";
    list.Prefix(szPrefix);
    list.Add("abc");

    list.Clear();

    EXPECT_EQ(list.Count(), 0);

    // Prefix was reset to the empty string: everything is accepted again.
    list.Add("zzz");
    EXPECT_EQ(list.Count(), 1);
}

TEST(ZCompletionList, SortOrdersCaseInsensitively)
{
    ZCompletionList list(256);

    list.Add("charlie");
    list.Add("Alpha");
    list.Add("bravo");
    list.Add("delta");

    list.Sort();

    ASSERT_EQ(list.Count(), 4);
    EXPECT_STREQ(list.Get(0), "Alpha");
    EXPECT_STREQ(list.Get(1), "bravo");
    EXPECT_STREQ(list.Get(2), "charlie");
    EXPECT_STREQ(list.Get(3), "delta");
}

TEST(ZCompletionList, UniqueRemovesDuplicatesAndCompactsTable)
{
    ZCompletionList list(256);

    list.Add("beta");
    list.Add("alpha");
    list.Add("beta");
    list.Add("alpha");
    list.Add("gamma");

    list.Unique();

    ASSERT_EQ(list.Count(), 3);
    EXPECT_STREQ(list.Get(0), "alpha");
    EXPECT_STREQ(list.Get(1), "beta");
    EXPECT_STREQ(list.Get(2), "gamma");
}

TEST(ZCompletionList, LongestCommonPrefixOfEmptyList)
{
    ZCompletionList list(256);

    char* pszPrefix = reinterpret_cast<char*>(static_cast<uintptr_t>(0xDEADBEEF));
    int iLength = -1;

    list.LongestCommonPrefix(pszPrefix, iLength);

    EXPECT_EQ(pszPrefix, nullptr);
    EXPECT_EQ(iLength, 0);
}

TEST(ZCompletionList, LongestCommonPrefixAcrossEntries)
{
    ZCompletionList list(256);

    list.Add("show_debug");
    list.Add("show_fps");
    list.Add("SHOW_grid");

    char* pszPrefix = nullptr;
    int iLength = 0;

    list.LongestCommonPrefix(pszPrefix, iLength);

    ASSERT_NE(pszPrefix, nullptr);
    EXPECT_EQ(iLength, 5); // "show_" case-insensitively
    EXPECT_EQ(StrNCaseCmp(pszPrefix, "show_", 5), 0);
}

TEST(ZCompletionList, LongestCommonPrefixStopsAtMismatch)
{
    ZCompletionList list(256);

    list.Add("abc");
    list.Add("abd");
    list.Add("a");

    char* pszPrefix = nullptr;
    int iLength = 0;

    list.LongestCommonPrefix(pszPrefix, iLength);

    EXPECT_EQ(iLength, 1);
}

TEST(ZCompletionList, OverflowDropsNewEntriesButKeepsExisting)
{
    ZCompletionList list(48);

    // Each entry costs strlen+1 bytes of text plus 4 bytes of table; the 48 byte
    // arena can hold only a handful of entries before Add() starts dropping.
    list.Add("aaaaaaaa");
    list.Add("bbbbbbbb");
    list.Add("cccccccc");

    const int iCountBefore = list.Count();
    ASSERT_GT(iCountBefore, 0);

    for (int i = 0; i < 32; ++i)
        list.Add("dddddddd");

    // Buffer never corrupts existing entries and count stays within capacity.
    // The table grows downwards, so the oldest surviving entry sits in the last slot.
    EXPECT_GE(list.Count(), iCountBefore);
    EXPECT_STREQ(list.Get(list.Count() - 1), "aaaaaaaa");
}

TEST(ZCompletionList, UniqueOnEmptyListIsSafe)
{
    ZCompletionList list(256);

    list.Unique();
    list.Sort();

    EXPECT_EQ(list.Count(), 0);
}
