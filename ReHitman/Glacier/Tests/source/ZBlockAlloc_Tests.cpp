#include <Glacier/ZSTL/ZBlockAlloc.h>
#include <gtest/gtest.h>

#include <cstdint>

using namespace Glacier;


TEST(ZBlockAlloc, CreateInitializesFreeListAndAlignedMemory)
{
    ZBlockAlloc alloc(16);

    alloc.Create(4, 24);

    ASSERT_NE(alloc.m_pBlockMem, nullptr);
    ASSERT_NE(alloc.m_pBlockMemAligned, nullptr);
    ASSERT_NE(alloc.m_pFreeMemTab, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(alloc.m_pBlockMemAligned) % 16, 0u);
    EXPECT_EQ(alloc.m_lNumBlocks, 4);
    EXPECT_EQ(alloc.m_lBlockSize, 24);
    EXPECT_EQ(alloc.m_lNumAllocated, 0);
    EXPECT_EQ(alloc.m_lNextFree, 0);
    EXPECT_EQ(alloc.m_pFreeMemTab[0], 1);
    EXPECT_EQ(alloc.m_pFreeMemTab[1], 2);
    EXPECT_EQ(alloc.m_pFreeMemTab[2], 3);
    EXPECT_EQ(alloc.m_pFreeMemTab[3], -1);
}

TEST(ZBlockAlloc, InvalidCreateClearsAllocatedStorage)
{
    ZBlockAlloc alloc;
    alloc.Create(2, 32);

    alloc.Create(0, 32);

    EXPECT_EQ(alloc.m_pBlockMem, nullptr);
    EXPECT_EQ(alloc.m_pBlockMemAligned, nullptr);
    EXPECT_EQ(alloc.m_pFreeMemTab, nullptr);
    EXPECT_EQ(alloc.m_lNumAllocated, 0);
    EXPECT_EQ(alloc.m_lNextFree, 0);
}

TEST(ZBlockAlloc, AllocBlocksReturnsSequentialBlockPointers)
{
    ZBlockAlloc alloc;
    alloc.Create(3, 32);

    auto* first = static_cast<char*>(alloc.AllocBlocks(1));
    auto* second = static_cast<char*>(alloc.AllocBlocks(1));
    auto* third = static_cast<char*>(alloc.AllocBlocks(1));

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(third, nullptr);
    EXPECT_EQ(first, alloc.m_pBlockMemAligned);
    EXPECT_EQ(second, alloc.m_pBlockMemAligned + 32);
    EXPECT_EQ(third, alloc.m_pBlockMemAligned + 64);
    EXPECT_EQ(alloc.m_lNumAllocated, 3);
    EXPECT_EQ(alloc.m_lNextFree, -1);
    EXPECT_EQ(alloc.AllocBlocks(1), nullptr);
}

TEST(ZBlockAlloc, AllocBlocksLinksMultiBlockChainInFreeTable)
{
    ZBlockAlloc alloc;
    alloc.Create(5, 16);

    void* chain = alloc.AllocBlocks(3);

    ASSERT_EQ(chain, alloc.m_pBlockMemAligned);
    EXPECT_EQ(alloc.m_lNumAllocated, 3);
    EXPECT_EQ(alloc.m_lNextFree, 3);
    EXPECT_EQ(alloc.m_pFreeMemTab[0], 1);
    EXPECT_EQ(alloc.m_pFreeMemTab[1], 2);
    EXPECT_EQ(alloc.m_pFreeMemTab[2], -1);
    EXPECT_EQ(alloc.m_pFreeMemTab[3], 4);
    EXPECT_EQ(alloc.m_pFreeMemTab[4], -1);
}

TEST(ZBlockAlloc, AllocBlocksReturnsNullWhenRequestExceedsAvailableBlocks)
{
    ZBlockAlloc alloc;
    alloc.Create(2, 16);

    EXPECT_EQ(alloc.AllocBlocks(3), nullptr);
    EXPECT_EQ(alloc.m_lNumAllocated, 0);
    EXPECT_EQ(alloc.m_lNextFree, 0);

    ASSERT_NE(alloc.AllocBlocks(2), nullptr);
    EXPECT_EQ(alloc.AllocBlocks(1), nullptr);
}
