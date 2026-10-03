#include <Glacier/ZSTL/ZPoolAllocLinkSortRefTab.h>
#include <gtest/gtest.h>

#include <array>
#include <vector>

using namespace Glacier;

namespace
{
    std::vector<uint32_t> CollectRefs(ZPoolAllocLinkSortRefTab& tab)
    {
        std::vector<uint32_t> refs;
        RefRun run;
        tab.RunInitNxtRef(&run);

        while (auto* ref = tab.RunNxtRefPtr(&run))
            refs.push_back(*ref);

        return refs;
    }
}

TEST(ZPoolAllocLinkSortRefTab, ConstructorStoresAllocatorAndUsesSortLayout)
{
    std::array<uint8_t, 4096> storage{};
    ZPoolAllocator allocator(reinterpret_cast<char*>(storage.data()), storage.size(), "LinkSort", false);
    ZPoolAllocLinkSortRefTab tab(&allocator, 2, 0);

    EXPECT_EQ(tab.m_pAllocator, &allocator);
    EXPECT_EQ(tab.EleSize, 4u);
    EXPECT_EQ(tab.BlkSize, 8);
    EXPECT_EQ(tab.Count(), 0);
}

TEST(ZPoolAllocLinkSortRefTab, NewBlocksComeFromPoolAllocator)
{
    std::array<uint8_t, 4096> storage{};
    ZPoolAllocator allocator(reinterpret_cast<char*>(storage.data()), storage.size(), "LinkSort", false);
    ZPoolAllocLinkSortRefTab tab(&allocator, 2, 0);

    EXPECT_EQ(allocator.m_iAllocated, 0u);

    tab.AddSort(10, 1.0f, 0);

    EXPECT_NE(tab.TabFirstPtr, nullptr);
    EXPECT_EQ(tab.TabFirstPtr, tab.TabBlockPtr);
    EXPECT_EQ(allocator.m_iAllocated, 52u);
    EXPECT_EQ(*tab.GetRefPtrNr(0), 10u);
    EXPECT_FLOAT_EQ(tab.GetSort(tab.GetRefPtrNr(0)), 1.0f);
}

TEST(ZPoolAllocLinkSortRefTab, BlocksAndFreeStackUseSamePool)
{
    std::array<uint8_t, 8192> storage{};
    ZPoolAllocator allocator(reinterpret_cast<char*>(storage.data()), storage.size(), "LinkSort", false);
    ZPoolAllocLinkSortRefTab tab(&allocator, 1, 0);

    tab.AddSort(10, 1.0f, 0);
    tab.AddSort(20, 2.0f, 0);
    ASSERT_EQ(tab.Count(), 2);
    EXPECT_EQ(allocator.m_iAllocated, 2u * 36u);

    uint32_t* first = tab.GetRefPtrNr(0);
    ASSERT_NE(first, nullptr);
    tab.DelRefPtr(first);

    ASSERT_NE(tab.FreeStack, nullptr);
    EXPECT_EQ(allocator.m_iAllocated, 2u * 36u + 36u + 52u);
    EXPECT_EQ(tab.Count(), 1);

    tab.AddSort(30, 3.0f, 0);

    EXPECT_EQ(tab.FreeStack, nullptr);
    EXPECT_EQ(allocator.m_iAllocated, 2u * 36u);
    EXPECT_EQ(CollectRefs(tab), (std::vector<uint32_t>{20, 30}));
}

TEST(ZPoolAllocLinkSortRefTab, ClearReturnsAllPoolAllocations)
{
    std::array<uint8_t, 8192> storage{};
    ZPoolAllocator allocator(reinterpret_cast<char*>(storage.data()), storage.size(), "LinkSort", false);
    ZPoolAllocLinkSortRefTab tab(&allocator, 2, 0);

    tab.AddSort(10, 1.0f, 0);
    tab.AddSort(20, 2.0f, 0);
    tab.DelRefPtr(tab.GetRefPtrNr(0));

    ASSERT_GT(allocator.m_iAllocated, 0u);
    ASSERT_NE(tab.FreeStack, nullptr);

    tab.Clear();

    EXPECT_EQ(tab.Count(), 0);
    EXPECT_EQ(tab.TabFirstPtr, nullptr);
    EXPECT_EQ(tab.TabBlockPtr, nullptr);
    EXPECT_EQ(tab.FreeStack, nullptr);
    EXPECT_EQ(allocator.m_iAllocated, 0u);
}

TEST(ZPoolAllocLinkSortRefTab, DestructorReturnsPoolAllocations)
{
    std::array<uint8_t, 8192> storage{};
    ZPoolAllocator allocator(reinterpret_cast<char*>(storage.data()), storage.size(), "LinkSort", false);

    {
        ZPoolAllocLinkSortRefTab tab(&allocator, 2, 0);
        tab.AddSort(10, 1.0f, 0);
        tab.AddSort(20, 2.0f, 0);
        tab.DelRefPtr(tab.GetRefPtrNr(0));
        ASSERT_GT(allocator.m_iAllocated, 0u);
    }

    EXPECT_EQ(allocator.m_iAllocated, 0u);
}
