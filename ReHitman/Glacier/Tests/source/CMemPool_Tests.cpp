#include <Glacier/ZSTL/CMemPool.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>

using namespace Glacier;

namespace
{
    static_assert(sizeof(CMemPool) == 0x18);
}

TEST(CMemPool, DefaultConstructsEmpty)
{
    CMemPool pool;

    EXPECT_EQ(pool.m_pxPool, nullptr);
    EXPECT_EQ(pool.m_pAllocTable, nullptr);
    EXPECT_EQ(pool.m_iEntries, 0);
    EXPECT_EQ(pool.m_iBlockSize, 0);
    EXPECT_EQ(pool.m_iNumAlloc, 0);
    EXPECT_FALSE(pool.m_bOwnPool);
}

TEST(CMemPool, InitAllocatesOwnedPoolAndRoundsBlockSizeToAlignment)
{
    CMemPool pool;

    pool.Init(5, 3, 4);

    EXPECT_NE(pool.m_pxPool, nullptr);
    EXPECT_NE(pool.m_pAllocTable, nullptr);
    EXPECT_EQ(pool.m_iEntries, 3);
    EXPECT_EQ(pool.m_iBlockSize, 8);
    EXPECT_EQ(pool.m_iNumAlloc, 0);
    EXPECT_TRUE(pool.m_bOwnPool);
    EXPECT_EQ(pool.m_pAllocTable[0], 0u);
    EXPECT_EQ(pool.m_pAllocTable[1], 1u);
    EXPECT_EQ(pool.m_pAllocTable[2], 2u);
}

TEST(CMemPool, AllocReturnsSequentialBlocksUntilExhausted)
{
    CMemPool pool;
    pool.Init(5, 3, 4);

    void* first = pool.Alloc();
    void* second = pool.Alloc();
    void* third = pool.Alloc();
    void* exhausted = pool.Alloc();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(third, nullptr);
    EXPECT_EQ(exhausted, nullptr);
    EXPECT_EQ(pool.m_iNumAlloc, 3);
    EXPECT_EQ(static_cast<uint8_t*>(second) - static_cast<uint8_t*>(first), pool.m_iBlockSize);
    EXPECT_EQ(static_cast<uint8_t*>(third) - static_cast<uint8_t*>(second), pool.m_iBlockSize);
}

TEST(CMemPool, AllocWithIndexReportsBlockIndexAndExhaustion)
{
    CMemPool pool;
    pool.Init(4, 2, 4);

    int firstIndex = -99;
    int secondIndex = -99;
    int exhaustedIndex = -99;

    void* first = pool.Alloc(&firstIndex);
    void* second = pool.Alloc(&secondIndex);
    void* exhausted = pool.Alloc(&exhaustedIndex);

    EXPECT_NE(first, nullptr);
    EXPECT_NE(second, nullptr);
    EXPECT_EQ(exhausted, nullptr);
    EXPECT_EQ(firstIndex, 0);
    EXPECT_EQ(secondIndex, 1);
    EXPECT_EQ(exhaustedIndex, -1);
    EXPECT_EQ(pool.Alloc(nullptr), nullptr);
}

TEST(CMemPool, DeAllocReusesReleasedBlockFirst)
{
    CMemPool pool;
    pool.Init(4, 3, 4);

    void* first = pool.Alloc();
    void* second = pool.Alloc();
    void* third = pool.Alloc();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(third, nullptr);

    pool.DeAlloc(second);

    EXPECT_EQ(pool.m_iNumAlloc, 2);
    EXPECT_EQ(pool.m_pAllocTable[2], 1u);
    EXPECT_EQ(pool.Alloc(), second);
    EXPECT_EQ(pool.m_iNumAlloc, 3);
}

TEST(CMemPool, MultipleDeAllocationsAreReusedInLifoOrder)
{
    CMemPool pool;
    pool.Init(4, 4, 4);

    void* first = pool.Alloc();
    void* second = pool.Alloc();
    void* third = pool.Alloc();
    void* fourth = pool.Alloc();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(third, nullptr);
    ASSERT_NE(fourth, nullptr);

    pool.DeAlloc(second);
    pool.DeAlloc(fourth);

    EXPECT_EQ(pool.Alloc(), fourth);
    EXPECT_EQ(pool.Alloc(), second);
    EXPECT_EQ(pool.Alloc(), nullptr);
}

TEST(CMemPool, InitWithExternalPoolUsesProvidedMemoryAndOwnsOnlyAllocTable)
{
    alignas(4) std::array<uint8_t, 32> storage{};
    CMemPool pool;

    pool.Init(5, 4, storage.data(), 4);

    EXPECT_EQ(pool.m_pxPool, storage.data());
    EXPECT_NE(pool.m_pAllocTable, nullptr);
    EXPECT_EQ(pool.m_iEntries, 4);
    EXPECT_EQ(pool.m_iBlockSize, 8);
    EXPECT_FALSE(pool.m_bOwnPool);

    int index = -1;
    void* first = pool.Alloc(&index);

    EXPECT_EQ(first, storage.data());
    EXPECT_EQ(index, 0);
    EXPECT_EQ(pool.Alloc(), storage.data() + 8);
}

TEST(CMemPool, DeInitResetsStateAndCanBeCalledTwice)
{
    CMemPool pool;
    pool.Init(4, 2, 4);

    pool.Alloc();
    pool.DeInit();

    EXPECT_EQ(pool.m_pxPool, nullptr);
    EXPECT_EQ(pool.m_pAllocTable, nullptr);
    EXPECT_EQ(pool.m_iEntries, 0);
    EXPECT_EQ(pool.m_iNumAlloc, 0);

    pool.DeInit();

    EXPECT_EQ(pool.m_pxPool, nullptr);
    EXPECT_EQ(pool.m_pAllocTable, nullptr);
}

TEST(CMemPool, DeInitDoesNotFreeExternalPool)
{
    alignas(4) std::array<uint8_t, 16> storage{};
    CMemPool pool;

    pool.Init(4, 2, storage.data(), 4);
    auto* allocated = static_cast<uint8_t*>(pool.Alloc());
    ASSERT_EQ(allocated, storage.data());
    allocated[0] = 0xAB;

    pool.DeInit();

    EXPECT_EQ(storage[0], 0xABu);
    EXPECT_EQ(pool.m_pxPool, nullptr);
    EXPECT_EQ(pool.m_pAllocTable, nullptr);
}

TEST(CMemPool, DeAllocNullIsNoOp)
{
    CMemPool pool;
    pool.Init(4, 2, 4);

    pool.DeAlloc(nullptr);

    EXPECT_EQ(pool.m_iNumAlloc, 0);
}

TEST(CMemPool, DeAllocBeforeInitIsNoOp)
{
    CMemPool pool;
    int dummy = 0;

    pool.DeAlloc(&dummy);

    EXPECT_EQ(pool.m_iNumAlloc, 0);
    EXPECT_EQ(pool.m_pxPool, nullptr);
    EXPECT_EQ(pool.m_pAllocTable, nullptr);
}
