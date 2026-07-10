#include <gtest/gtest.h>

#include <Glacier/ZSTL/STRREFTAB.h>

using namespace Glacier;

TEST(STRREFTAB, ConstructorInitialization)
{
    STRREFTAB tab(4, 1);

    EXPECT_EQ(tab.Count(), 0u);
    EXPECT_FALSE(tab.Exists("test"));
    EXPECT_EQ(tab.FindStr("test"), nullptr);
}

TEST(STRREFTAB, AddAndFind)
{
    STRREFTAB tab(4, 1);

    EXPECT_NE(tab.AddStr("Alpha"), nullptr);
    EXPECT_NE(tab.FindStr("Alpha"), nullptr);
    EXPECT_TRUE(tab.Exists("Alpha"));

    EXPECT_EQ(tab.Count(), 1u);
}

TEST(STRREFTAB, AddDuplicateRejected)
{
    STRREFTAB tab(4, 1);

    EXPECT_NE(tab.AddStr("Alpha"), nullptr);
    EXPECT_EQ(tab.AddStr("Alpha"), nullptr);

    EXPECT_EQ(tab.Count(), 1u);
}

TEST(STRREFTAB, AddAlwaysAllowsDuplicates)
{
    STRREFTAB tab(4, 1);

    EXPECT_NE(tab.AddAlways("Alpha"), nullptr);
    EXPECT_NE(tab.AddAlways("Alpha"), nullptr);

    EXPECT_EQ(tab.Count(), 2u);
}

TEST(STRREFTAB, RemoveExistingString)
{
    STRREFTAB tab(4, 1);

    tab.AddStr("Alpha");
    tab.AddStr("Beta");
    tab.AddStr("Gamma");

    tab.RemoveStr("Beta");

    EXPECT_FALSE(tab.Exists("Beta"));
    EXPECT_TRUE(tab.Exists("Alpha"));
    EXPECT_TRUE(tab.Exists("Gamma"));

    EXPECT_EQ(tab.Count(), 2u);
}

TEST(STRREFTAB, RemoveMissingStringDoesNothing)
{
    STRREFTAB tab(4, 1);

    tab.AddStr("Alpha");
    tab.AddStr("Beta");

    tab.RemoveStr("NoSuchString");

    EXPECT_EQ(tab.Count(), 2u);
    EXPECT_TRUE(tab.Exists("Alpha"));
    EXPECT_TRUE(tab.Exists("Beta"));
}

TEST(STRREFTAB, CaseInsensitiveSearch)
{
    STRREFTAB tab(4, 1);

    tab.m_bCaseSensitive = false;

    tab.AddStr("Hello");

    EXPECT_TRUE(tab.Exists("hello"));
    EXPECT_TRUE(tab.Exists("HELLO"));
    EXPECT_NE(tab.FindStr("hElLo"), nullptr);
}

TEST(STRREFTAB, CaseSensitiveSearch)
{
    STRREFTAB tab(4, 1);

    tab.m_bCaseSensitive = true;

    tab.AddStr("Hello");

    EXPECT_TRUE(tab.Exists("Hello"));
    EXPECT_FALSE(tab.Exists("hello"));

    EXPECT_NE(tab.FindStr("Hello"), nullptr);
    EXPECT_EQ(tab.FindStr("hello"), nullptr);
}

TEST(STRREFTAB, SortAlphabetically)
{
    STRREFTAB tab(2, 1);

    tab.AddAlways("Charlie");
    tab.AddAlways("Alpha");
    tab.AddAlways("Bravo");

    tab.Sort();

    RefRun it;
    tab.RunInitNxtRef(&it);

    EXPECT_STREQ(reinterpret_cast<char*>(tab.RunNxtRef(&it)), "Alpha");
    EXPECT_STREQ(reinterpret_cast<char*>(tab.RunNxtRef(&it)), "Bravo");
    EXPECT_STREQ(reinterpret_cast<char*>(tab.RunNxtRef(&it)), "Charlie");
}

TEST(STRREFTAB, SortCaseInsensitive)
{
    STRREFTAB tab(2, 1);

    tab.AddAlways("zebra");
    tab.AddAlways("Apple");
    tab.AddAlways("banana");

    tab.Sort();

    RefRun it;
    tab.RunInitNxtRef(&it);

    EXPECT_STREQ(reinterpret_cast<char*>(tab.RunNxtRef(&it)), "Apple");
    EXPECT_STREQ(reinterpret_cast<char*>(tab.RunNxtRef(&it)), "banana");
    EXPECT_STREQ(reinterpret_cast<char*>(tab.RunNxtRef(&it)), "zebra");
}

TEST(STRREFTAB, SortMultipleBlocks)
{
    STRREFTAB tab(2, 1);

    tab.AddAlways("Echo");
    tab.AddAlways("Delta");
    tab.AddAlways("Charlie");
    tab.AddAlways("Bravo");
    tab.AddAlways("Alpha");

    tab.Sort();

    static const char* expected[] =
    {
        "Alpha",
        "Bravo",
        "Charlie",
        "Delta",
        "Echo"
    };

    int idx = 0;
    for (auto entry : tab)
    {
        EXPECT_STREQ((const char*)entry, expected[idx]);
        ++idx;
    }
    
    EXPECT_EQ(idx, 5);
}

TEST(STRREFTAB, ClearRemovesEverything)
{
    STRREFTAB tab(2, 1);

    tab.AddAlways("One");
    tab.AddAlways("Two");
    tab.AddAlways("Three");

    tab.Clear();

    EXPECT_EQ(tab.Count(), 0u);
    EXPECT_FALSE(tab.Exists("One"));
    EXPECT_EQ(tab.FindStr("Two"), nullptr);

    RefRun it;
    tab.RunInitNxtRef(&it);
    EXPECT_EQ(tab.RunNxtRef(&it), 0u);
}

TEST(STRREFTAB, SortAfterDeletion)
{
    STRREFTAB tab(2, 1);

    tab.AddAlways("Delta");
    tab.AddAlways("Alpha");
    tab.AddAlways("Charlie");
    tab.AddAlways("Bravo");

    tab.RemoveStr("Charlie");

    tab.Sort();

    const char* expected[] =
    {
        "Alpha",
        "Bravo",
        "Delta"
    };

    RefRun it;
    tab.RunInitNxtRef(&it);

    int idx = 0;
    for (auto entry : tab)
    {
        EXPECT_STREQ((const char*)entry, expected[idx]);
        ++idx;
    }

    EXPECT_EQ(idx, 3);
}