#include <Glacier/ZSTL/LINKSORTREFTAB.h>
#include <gtest/gtest.h>

#include <vector>

using namespace Glacier;

namespace
{
    std::vector<uint32_t> CollectRefs(LINKSORTREFTAB& tab)
    {
        std::vector<uint32_t> refs;
        RefRun run;
        tab.RunInitNxtRef(&run);

        while (auto* ref = tab.RunNxtRefPtr(&run))
            refs.push_back(*ref);

        return refs;
    }

    std::vector<float> CollectSorts(LINKSORTREFTAB& tab)
    {
        std::vector<float> sorts;
        RefRun run;
        tab.RunInitNxtRef(&run);

        while (auto* ref = tab.RunNxtRefPtr(&run))
            sorts.push_back(tab.GetSort(ref));

        return sorts;
    }
}

TEST(LINKSORTREFTAB, ConstructorReservesSortWord)
{
    LINKSORTREFTAB tab(4, 2);

    EXPECT_EQ(tab.EleSize, 6u);
    EXPECT_EQ(tab.BlkSize, 24);
    EXPECT_EQ(tab.m_pLastAddSort, nullptr);
}

TEST(LINKSORTREFTAB, AddSortMaintainsAscendingOrder)
{
    LINKSORTREFTAB tab(2, 0);

    tab.AddSort(30, 3.0f, 0);
    tab.AddSort(10, 1.0f, 0);
    tab.AddSort(20, 2.0f, 0);

    EXPECT_EQ(CollectRefs(tab), (std::vector<uint32_t>{10, 20, 30}));
    EXPECT_EQ(CollectSorts(tab), (std::vector<float>{1.0f, 2.0f, 3.0f}));
}

TEST(LINKSORTREFTAB, AddSortModesFollowPs2EqualSortPlacement)
{
    LINKSORTREFTAB tab(8, 0);

    tab.AddSort(10, 1.0f, 0);
    tab.AddSort(20, 1.0f, 0);
    tab.AddSort(30, 1.0f, 1);
    tab.AddSort(40, 1.0f, 2);

    EXPECT_EQ(CollectRefs(tab), (std::vector<uint32_t>{30, 20, 10, 40}));
    EXPECT_EQ(CollectSorts(tab), (std::vector<float>{1.0f, 1.0f, 1.0f, 1.0f}));
}

TEST(LINKSORTREFTAB, SetSortNrMovesRecordAndPreservesReference)
{
    LINKSORTREFTAB tab(8, 0);

    tab.AddSort(10, 1.0f, 0);
    tab.AddSort(20, 2.0f, 0);
    tab.AddSort(30, 3.0f, 0);

    uint32_t* ref20 = tab.GetRefPtrNr(1);
    ASSERT_NE(ref20, nullptr);
    ASSERT_EQ(*ref20, 20u);

    tab.SetSortNr(ref20, 4.0f);

    EXPECT_EQ(CollectRefs(tab), (std::vector<uint32_t>{10, 30, 20}));
    EXPECT_EQ(CollectSorts(tab), (std::vector<float>{1.0f, 3.0f, 4.0f}));
    EXPECT_EQ(tab.Count(), 3);
}

TEST(LINKSORTREFTAB, DeleteAndClearResetLastAddedSort)
{
    LINKSORTREFTAB tab(8, 0);

    tab.AddSort(10, 1.0f, 0);
    tab.AddSort(20, 2.0f, 0);
    ASSERT_NE(tab.m_pLastAddSort, nullptr);

    tab.DelRefPtr(tab.m_pLastAddSort);
    EXPECT_EQ(tab.m_pLastAddSort, nullptr);

    tab.AddSort(30, 3.0f, 0);
    ASSERT_NE(tab.m_pLastAddSort, nullptr);

    tab.Clear();

    EXPECT_EQ(tab.m_pLastAddSort, nullptr);
    EXPECT_EQ(tab.Count(), 0);
    EXPECT_EQ(tab.First, nullptr);
    EXPECT_EQ(tab.Last, nullptr);
}
