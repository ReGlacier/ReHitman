#include <Glacier/ZSTL/LINKREFTAB.h>
#include <gtest/gtest.h>

#include <vector>

using namespace Glacier;

namespace
{
    std::vector<uint32_t> CollectForward(LINKREFTAB& tab)
    {
        std::vector<uint32_t> refs;

        RefRun it;
        tab.RunInitNxtRef(&it);

        while (auto* ref = tab.RunNxtRefPtr(&it))
        {
            refs.push_back(*ref);
        }

        return refs;
    }

    std::vector<uint32_t> CollectBackward(LINKREFTAB& tab)
    {
        std::vector<uint32_t> refs;

        RefRun it;
        tab.RunInitPrevRef(&it);

        while (auto* ref = tab.RunPrevRefPtr(&it))
        {
            refs.push_back(*ref);
        }

        return refs;
    }
}


TEST(LINKREFTAB, InitiallyEmpty)
{
    LINKREFTAB tab(8, 0);

    EXPECT_EQ(tab.Count(), 0);
    EXPECT_EQ(tab.First, nullptr);
    EXPECT_EQ(tab.Last, nullptr);
}

TEST(LINKREFTAB, AddCreatesSingleElement)
{
    LINKREFTAB tab(8, 0);

    uint32_t* ref = tab.Add(123);

    ASSERT_NE(ref, nullptr);

    EXPECT_EQ(*(ref - 1), 123u);
    EXPECT_EQ(tab.Count(), 1);

    EXPECT_EQ(tab.First, tab.Last);
    EXPECT_EQ(tab.First->Prev, nullptr);
    EXPECT_EQ(tab.First->Next, nullptr);
}

TEST(LINKREFTAB, ConstructorInitializesLinkSpecificStorage)
{
    LINKREFTAB tab(4, 2);

    EXPECT_EQ(tab.EleCount, 0);
    EXPECT_EQ(tab.EleSize, 5u);
    EXPECT_EQ(tab.BlkSize, 20u);
    EXPECT_EQ(tab.Count(), 0);
    EXPECT_EQ(tab.First, nullptr);
    EXPECT_EQ(tab.Last, nullptr);
    EXPECT_EQ(tab.FreeStack, nullptr);
}

TEST(LINKREFTAB, AddStartAndAddEndMaintainIterationOrder)
{
    LINKREFTAB tab(8, 0);

    tab.Add(20);
    tab.AddStart(10);
    tab.AddEnd(30);

    EXPECT_EQ(tab.Count(), 3);
    EXPECT_EQ(CollectForward(tab), (std::vector<uint32_t>{10, 20, 30}));
    EXPECT_EQ(CollectBackward(tab), (std::vector<uint32_t>{30, 20, 10}));

    ASSERT_NE(tab.First, nullptr);
    ASSERT_NE(tab.Last, nullptr);
    EXPECT_EQ(tab.First->Prev, nullptr);
    EXPECT_EQ(tab.Last->Next, nullptr);
}

TEST(LINKREFTAB, InsertBeforeHandlesHeadMiddleAndNullTarget)
{
    LINKREFTAB tab(8, 0);

    tab.Add(20);
    tab.Add(40);

    uint32_t* ref40 = tab.GetRefPtrNr(1);
    ASSERT_NE(ref40, nullptr);
    tab.InsertBefore(ref40, 30);

    uint32_t* ref20 = tab.GetRefPtrNr(0);
    ASSERT_NE(ref20, nullptr);
    tab.InsertBefore(ref20, 10);

    tab.InsertBefore(nullptr, 50);

    EXPECT_EQ(tab.Count(), 5);
    EXPECT_EQ(CollectForward(tab), (std::vector<uint32_t>{10, 20, 30, 40, 50}));
    EXPECT_EQ(CollectBackward(tab), (std::vector<uint32_t>{50, 40, 30, 20, 10}));
}

TEST(LINKREFTAB, GetRefNrAndGetRefPtrNrFollowLinkedOrder)
{
    LINKREFTAB tab(2, 0);

    tab.Add(10);
    tab.Add(20);
    tab.AddStart(5);
    tab.AddEnd(30);

    EXPECT_EQ(tab.GetRefNr(0), 5u);
    EXPECT_EQ(tab.GetRefNr(1), 10u);
    EXPECT_EQ(tab.GetRefNr(2), 20u);
    EXPECT_EQ(tab.GetRefNr(3), 30u);
    EXPECT_EQ(tab.GetRefNr(4), 0u);

    ASSERT_NE(tab.GetRefPtrNr(0), nullptr);
    ASSERT_NE(tab.GetRefPtrNr(3), nullptr);
    EXPECT_EQ(*tab.GetRefPtrNr(0), 5u);
    EXPECT_EQ(*tab.GetRefPtrNr(3), 30u);
    EXPECT_EQ(tab.GetRefPtrNr(4), nullptr);
}

TEST(LINKREFTAB, GetPrevAndNextRefPtrReturnLinkedNeighbors)
{
    LINKREFTAB tab(8, 0);

    tab.Add(10);
    tab.Add(20);
    tab.Add(30);

    uint32_t* ref10 = tab.GetRefPtrNr(0);
    uint32_t* ref20 = tab.GetRefPtrNr(1);
    uint32_t* ref30 = tab.GetRefPtrNr(2);

    ASSERT_NE(ref10, nullptr);
    ASSERT_NE(ref20, nullptr);
    ASSERT_NE(ref30, nullptr);

    EXPECT_EQ(tab.GetPrevRefPtr(ref10), nullptr);
    EXPECT_EQ(tab.GetNextRefPtr(ref10), ref20);
    EXPECT_EQ(tab.GetPrevRefPtr(ref20), ref10);
    EXPECT_EQ(tab.GetNextRefPtr(ref20), ref30);
    EXPECT_EQ(tab.GetPrevRefPtr(ref30), ref20);
    EXPECT_EQ(tab.GetNextRefPtr(ref30), nullptr);
}

TEST(LINKREFTAB, RunDelRefRemovesCurrentNodeWithoutChangingStorageCount)
{
    LINKREFTAB tab(8, 0);

    tab.Add(10);
    tab.Add(20);
    tab.Add(30);

    RefRun it;
    tab.RunInitNxtRef(&it);
    ASSERT_EQ(*tab.RunNxtRefPtr(&it), 10u);
    ASSERT_EQ(*tab.RunNxtRefPtr(&it), 20u);

    tab.RunDelRef(&it);

    EXPECT_EQ(tab.EleCount, 3);
    EXPECT_EQ(tab.Count(), 2);
    ASSERT_NE(tab.FreeStack, nullptr);
    EXPECT_EQ(CollectForward(tab), (std::vector<uint32_t>{10, 30}));
    EXPECT_EQ(CollectBackward(tab), (std::vector<uint32_t>{30, 10}));
}

TEST(LINKREFTAB, DeletedRecordIsReusedByNextAdd)
{
    LINKREFTAB tab(8, 0);

    tab.Add(10);
    tab.Add(20);
    tab.Add(30);

    uint32_t* ref20 = tab.GetRefPtrNr(1);
    ASSERT_NE(ref20, nullptr);

    RefRun it;
    tab.RunInitNxtRef(&it);
    tab.RunNxtRefPtr(&it);
    tab.RunNxtRefPtr(&it);
    tab.RunDelRef(&it);

    ASSERT_NE(tab.FreeStack, nullptr);
    EXPECT_EQ(tab.Count(), 2);

    tab.Add(40);

    EXPECT_EQ(tab.EleCount, 3);
    EXPECT_EQ(tab.Count(), 3);
    EXPECT_EQ(tab.FreeStack, nullptr);
    EXPECT_EQ(tab.GetRefPtrNr(2), ref20);
    EXPECT_EQ(CollectForward(tab), (std::vector<uint32_t>{10, 30, 40}));
}

TEST(LINKREFTAB, ClearRemovesActiveAndFreeRecords)
{
    LINKREFTAB tab(8, 0);

    tab.Add(10);
    tab.Add(20);

    RefRun it;
    tab.RunInitNxtRef(&it);
    tab.RunNxtRefPtr(&it);
    tab.RunDelRef(&it);

    ASSERT_NE(tab.FreeStack, nullptr);

    tab.Clear();

    EXPECT_EQ(tab.EleCount, 0);
    EXPECT_EQ(tab.Count(), 0);
    EXPECT_EQ(tab.First, nullptr);
    EXPECT_EQ(tab.Last, nullptr);
    EXPECT_EQ(tab.FreeStack, nullptr);
    EXPECT_EQ(tab.TabFirstPtr, nullptr);
    EXPECT_EQ(tab.TabBlockPtr, nullptr);
}
