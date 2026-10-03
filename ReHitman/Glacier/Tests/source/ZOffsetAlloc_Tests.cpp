#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <gtest/gtest.h>

#include <cstdint>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZOffsetAlloc::ZLink) == 0x8);
    static_assert(sizeof(ZOffsetAlloc) == 0x14);

    uint32_t OffsetOf(void* value)
    {
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(value));
    }
}

TEST(ZOffsetAlloc, ConstructorInitializesSingleFreeRangeWithInternalBuffer)
{
    ZOffsetAlloc alloc(100, 200, 4, nullptr, DEFAULT_MEM);

    ASSERT_NE(alloc.m_pLinks, nullptr);
    EXPECT_FALSE(alloc.m_bUserSuppliedLinkBuffer);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 1u);
    EXPECT_EQ(alloc.GetFreeTotal(), 100);
    EXPECT_EQ(alloc.GetLargestFreeBlock(), 100u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lOffset, 100u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lSize, 100u);
}

TEST(ZOffsetAlloc, ConstructorUsesUserSuppliedLinkBuffer)
{
    ZOffsetAlloc::ZLink links[4]{};
    ZOffsetAlloc alloc(10, 30, 4, links, DEFAULT_MEM);

    EXPECT_EQ(alloc.m_pLinks, links);
    EXPECT_TRUE(alloc.m_bUserSuppliedLinkBuffer);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 1u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lOffset, 10u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lSize, 20u);

}

TEST(ZOffsetAlloc, AllocCarvesFromStartOfBestFitBlock)
{
    ZOffsetAlloc alloc(0, 100, 8, nullptr, DEFAULT_MEM);

    EXPECT_EQ(OffsetOf(alloc.Alloc(20, false)), 0u);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 1u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lOffset, 20u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lSize, 80u);

    alloc.Free(0, 10);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 2u);

    EXPECT_EQ(OffsetOf(alloc.Alloc(8, false)), 0u);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 2u);
    EXPECT_EQ(alloc.GetFreeTotal(), 82);
    EXPECT_EQ(alloc.GetLargestFreeBlock(), 80u);
}

TEST(ZOffsetAlloc, AllocExactFitRemovesFreeLink)
{
    ZOffsetAlloc alloc(0, 100, 8, nullptr, DEFAULT_MEM);

    EXPECT_EQ(OffsetOf(alloc.Alloc(100, false)), 0u);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 0u);
    EXPECT_EQ(alloc.GetFreeTotal(), 0);
    EXPECT_EQ(alloc.GetLargestFreeBlock(), 0u);
    EXPECT_EQ(alloc.Alloc(1, false), reinterpret_cast<void*>(-1));
}

TEST(ZOffsetAlloc, AllocHiCarvesFromEndOfHighestOffsetBlock)
{
    ZOffsetAlloc alloc(0, 100, 8, nullptr, DEFAULT_MEM);

    EXPECT_EQ(OffsetOf(alloc.AllocHi(25)), 75u);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 1u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lOffset, 0u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lSize, 75u);

    alloc.Free(90, 10);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 2u);
    EXPECT_EQ(OffsetOf(alloc.AllocHi(5)), 95u);
}

TEST(ZOffsetAlloc, FreeMergesWithLeftAndRightNeighbors)
{
    ZOffsetAlloc alloc(0, 100, 8, nullptr, DEFAULT_MEM);

    EXPECT_EQ(OffsetOf(alloc.Alloc(10, false)), 0u);
    EXPECT_EQ(OffsetOf(alloc.Alloc(10, false)), 10u);
    EXPECT_EQ(OffsetOf(alloc.Alloc(10, false)), 20u);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 1u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lOffset, 30u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lSize, 70u);

    alloc.Free(0, 10);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 2u);

    alloc.Free(20, 10);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 2u);
    EXPECT_EQ(alloc.GetFreeTotal(), 90);

    alloc.Free(10, 10);
    EXPECT_EQ(alloc.GetNrFreeLinks(), 1u);
    EXPECT_EQ(alloc.GetFreeTotal(), 100);
    EXPECT_EQ(alloc.GetLargestFreeBlock(), 100u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lOffset, 0u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lSize, 100u);
}

TEST(ZOffsetAlloc, ResetReplacesFreeList)
{
    ZOffsetAlloc alloc(0, 100, 4, nullptr, DEFAULT_MEM);

    alloc.Alloc(40, false);
    alloc.Reset(50, 90);

    EXPECT_EQ(alloc.GetNrFreeLinks(), 1u);
    EXPECT_EQ(alloc.GetFreeTotal(), 40);
    EXPECT_EQ(alloc.m_pLinks[0].m_lOffset, 50u);
    EXPECT_EQ(alloc.m_pLinks[0].m_lSize, 40u);
}

TEST(ZOffsetAlloc, GetFreeListWritesPairsForElementSizeOneReftab)
{
    ZOffsetAlloc alloc(0, 100, 8, nullptr, DEFAULT_MEM);
    REFTAB freeList(8, 0);

    alloc.Alloc(20, false);
    alloc.Free(0, 10);

    alloc.GetFreeList(&freeList);

    ASSERT_EQ(freeList.Count(), 4);

    uint32_t firstOffset = freeList.GetRefNr(0);
    uint32_t firstSize = freeList.GetRefNr(1);
    uint32_t secondOffset = freeList.GetRefNr(2);
    uint32_t secondSize = freeList.GetRefNr(3);

    EXPECT_TRUE((firstOffset == 20u && firstSize == 80u && secondOffset == 0u && secondSize == 10u) ||
                (firstOffset == 0u && firstSize == 10u && secondOffset == 20u && secondSize == 80u));
}

TEST(ZOffsetAlloc, GetFreeListWritesSizeAndOffsetForElementSizeTwoReftab)
{
    ZOffsetAlloc alloc(0, 100, 4, nullptr, DEFAULT_MEM);
    REFTAB freeList(4, 1);

    alloc.GetFreeList(&freeList);

    ASSERT_EQ(freeList.Count(), 1);

    uint32_t* entry = freeList.GetRefPtrNr(0);
    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry[0], 100u);
    EXPECT_EQ(entry[1], 0u);
}

TEST(ZOffsetAlloc, InvalidResetAndNullFreeListAssert)
{
    ZOffsetAlloc alloc(0, 100, 2, nullptr, DEFAULT_MEM);

    EXPECT_THROW(alloc.Reset(100, 100), std::runtime_error);
    EXPECT_THROW(alloc.Reset(0x80000000u, 0x80000010u), std::runtime_error);
    EXPECT_THROW(alloc.GetFreeList(nullptr), std::runtime_error);
}

TEST(ZOffsetAlloc, AddFreeSpaceAssertsWhenLinkArrayIsFull)
{
    ZOffsetAlloc alloc(0, 100, 1, nullptr, DEFAULT_MEM);

    EXPECT_THROW(alloc.AddFreeSpace(200, 10), std::runtime_error);
}
