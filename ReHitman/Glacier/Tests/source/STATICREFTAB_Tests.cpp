#include <gtest/gtest.h>
#include <Glacier/ZSTL/STATICREFTAB.h>

using namespace Glacier;

TEST(STATICREFTAB, AddSingle)
{
    auto* tab = new STATICREFTAB(4, 5);

    EXPECT_EQ(tab->Count(), 0);

    auto* p = tab->Add(123);

    ASSERT_NE(p, nullptr);
    EXPECT_EQ(tab->Count(), 1);

    delete tab;
}

TEST(STATICREFTAB, ReuseFreedSlot)
{
    auto* tab = new STATICREFTAB(4, 5);

    auto* a = tab->Add(1);
    auto* b = tab->Add(2);

    EXPECT_EQ(tab->Count(), 2);

    tab->DelRefPtr(a);

    EXPECT_EQ(tab->Count(), 1);

    auto* c = tab->Add(3);

    EXPECT_EQ(tab->Count(), 2);
    EXPECT_NE(c, nullptr);
    EXPECT_NE(c, b);

    delete tab;
}

TEST(STATICREFTAB, AllocateMultipleBlocks)
{
    auto* tab = new STATICREFTAB(2, 5);

    EXPECT_EQ(tab->Count(), 0);

    tab->Add(1);
    tab->Add(2);
    tab->Add(3);

    EXPECT_EQ(tab->Count(), 3);

    delete tab;
}

TEST(STATICREFTAB, DeleteEmptyBlock)
{
    auto* tab = new STATICREFTAB(2, 5);

    auto* a = tab->Add(1);
    auto* b = tab->Add(2);

    EXPECT_EQ(tab->Count(), 2);

    tab->DelRefPtr(a);
    EXPECT_EQ(tab->Count(), 1);

    tab->DelRefPtr(b);
    EXPECT_EQ(tab->Count(), 0);

    auto* c = tab->Add(3);

    ASSERT_NE(c, nullptr);
    EXPECT_EQ(tab->Count(), 1);

    delete tab;
}

TEST(STATICREFTAB, Clear)
{
    auto* tab = new STATICREFTAB(4, 5);

    tab->Add(10);
    tab->Add(20);
    tab->Add(30);

    EXPECT_EQ(tab->Count(), 3);

    tab->Clear();

    EXPECT_EQ(tab->Count(), 0);

    delete tab;
}