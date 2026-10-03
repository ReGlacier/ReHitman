#include <Glacier/ZSTL/ZPoolAllocRefTab.h>
#include <gtest/gtest.h>
#include <array>

using namespace Glacier;


TEST(ZPoolAllocator, ConstructCreatesSingleFreeBlock)
{
    std::array<uint8_t, 1024> buffer{};

    ZPoolAllocator pool(reinterpret_cast<char*>(buffer.data()), buffer.size(), "Test", false);

    EXPECT_EQ(pool.m_iAllocated, 0u);
    EXPECT_EQ(pool.m_iHighWaterMark, 0u);
    EXPECT_EQ(pool.m_pBlockDataBegin->pointer, reinterpret_cast<char*>(buffer.data()));
    EXPECT_EQ(pool.m_pBlockDataBegin->size, buffer.size() - sizeof(ZPoolAllocator::ZBlockHeader));
    EXPECT_EQ(pool.m_pBlockDataBegin->pointer + pool.m_pBlockDataBegin->size, reinterpret_cast<char*>(pool.m_pBlockDataBegin));
}

TEST(ZPoolAllocator, AllocConsumesBeginningOfFreeBlock)
{
    std::array<uint8_t, 1024> buffer{};
    ZPoolAllocator pool(reinterpret_cast<char*>(buffer.data()), buffer.size(), "Test", false);

    void* p = pool.Alloc(16);

    ASSERT_NE(p, nullptr);
    EXPECT_EQ(pool.m_iAllocated, 20u); // 16 + header
    EXPECT_EQ(pool.m_pBlockDataBegin->pointer, reinterpret_cast<char*>(buffer.data()) + 20);
    EXPECT_EQ(pool.m_pBlockDataBegin->size, buffer.size() - sizeof(ZPoolAllocator::ZBlockHeader) - 20);
}

TEST(ZPoolAllocator, AllocFreeRestoresWholePool)
{
    std::array<uint8_t, 1024> buffer{};
    ZPoolAllocator pool(reinterpret_cast<char*>(buffer.data()), buffer.size(), "Test", false);

    void* p = pool.Alloc(64);
    ASSERT_NE(p, nullptr);

    pool.Free(static_cast<char*>(p));

    EXPECT_EQ(pool.m_iAllocated, 0u);
    EXPECT_EQ(pool.m_pBlockDataBegin->pointer, reinterpret_cast<char*>(buffer.data()));
    EXPECT_EQ(pool.m_pBlockDataBegin->pointer + pool.m_pBlockDataBegin->size, reinterpret_cast<char*>(pool.m_pBlockDataBegin));
}

TEST(ZPoolAllocator, MultipleAllocationsAdvancePointer)
{
    std::array<uint8_t, 1024> buffer{};
    ZPoolAllocator pool(reinterpret_cast<char*>(buffer.data()), buffer.size(), "Test", false);

    void* p1 = pool.Alloc(16);
    void* p2 = pool.Alloc(32);
    void* p3 = pool.Alloc(64);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);

    EXPECT_LT(p1, p2);
    EXPECT_LT(p2, p3);

    EXPECT_EQ(pool.m_iAllocated, (16 + 4) + (32 + 4) + (64 + 4));
    EXPECT_EQ(pool.m_pBlockDataBegin->pointer + pool.m_pBlockDataBegin->size, reinterpret_cast<char*>(pool.m_pBlockDataBegin));
}

TEST(ZPoolAllocator, AllocationIsAlignedToFourBytes)
{
    std::array<uint8_t, 1024> buffer{};
    ZPoolAllocator pool(reinterpret_cast<char*>(buffer.data()), buffer.size(), "Test", false);

    auto* p1 = static_cast<char*>(pool.Alloc(1));
    auto* p2 = static_cast<char*>(pool.Alloc(2));
    auto* p3 = static_cast<char*>(pool.Alloc(3));

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    ASSERT_NE(p3, nullptr);

    auto* h1 = reinterpret_cast<ZPoolAllocator::ZAllocHeader*>(p1) - 1;
    auto* h2 = reinterpret_cast<ZPoolAllocator::ZAllocHeader*>(p2) - 1;
    auto* h3 = reinterpret_cast<ZPoolAllocator::ZAllocHeader*>(p3) - 1;

    EXPECT_EQ(h1->size, 4u);
    EXPECT_EQ(h2->size, 4u);
    EXPECT_EQ(h3->size, 4u);
}

TEST(ZPoolAllocator, AllocTooLargeReturnsNull)
{
    std::array<uint8_t, 128> buffer{};
    ZPoolAllocator pool(reinterpret_cast<char*>(buffer.data()), buffer.size(), "Test", false);

    EXPECT_EQ(pool.Alloc(1024), nullptr);
}

TEST(ZPoolAllocator, ResetRestoresInitialState)
{
    std::array<uint8_t, 1024> buffer{};
    ZPoolAllocator pool(reinterpret_cast<char*>(buffer.data()), buffer.size(), "Test", false);

    ASSERT_NE(pool.Alloc(32), nullptr);
    ASSERT_NE(pool.Alloc(64), nullptr);

    pool.Reset(nullptr, 0, false);

    EXPECT_EQ(pool.m_iAllocated, 0u);
    EXPECT_EQ(pool.m_pBlockDataBegin->pointer, reinterpret_cast<char*>(buffer.data()));
    EXPECT_EQ(pool.m_pBlockDataBegin->pointer + pool.m_pBlockDataBegin->size, reinterpret_cast<char*>(pool.m_pBlockDataBegin));
}