#include <gtest/gtest.h>
#include <Glacier/ZSTL/STATICREFTAB.h>

using namespace Glacier;

TEST(STATICREFTAB_Tests, BasicPoolAllocation)
{
    STATICREFTAB pool(2, 0);

    EXPECT_EQ(pool.Count(), 0);
    EXPECT_EQ(pool.Size(), 2);

    uint32_t* pPayload1 = pool.Add(0xAAAAA111);
    ASSERT_NE(pPayload1, nullptr);
    
    EXPECT_EQ(*(pPayload1 - 1), 0xAAAAA111);
    EXPECT_EQ(pool.Count(), 1);

    uint32_t* pPayload2 = pool.Add(0xBBBBB222);
    ASSERT_NE(pPayload2, nullptr);
    EXPECT_EQ(*(pPayload2 - 1), 0xBBBBB222);
    EXPECT_EQ(pool.Count(), 2);

    pool.DelRefPtr(pPayload1);
    EXPECT_EQ(pool.Count(), 1);
}

TEST(STATICREFTAB_Tests, PointerStabilityAndFreeStackReuse)
{
    STATICREFTAB pool(3, 0);

    uint32_t* pRef1 = pool.Add(10);
    uint32_t* pRef2 = pool.Add(20);
    uint32_t* pRef3 = pool.Add(30);

    uint32_t* pAddrRef1 = pRef1;
    uint32_t* pAddrRef3 = pRef3;

    pool.DelRefPtr(pRef2);
    EXPECT_EQ(pool.Count(), 2);

    EXPECT_EQ(*(pAddrRef1 - 1), 10);
    EXPECT_EQ(*(pAddrRef3 - 1), 30);

    uint32_t* pRefNew = pool.Add(99);
    EXPECT_EQ(pool.Count(), 3);

    uint32_t* pBlockStart = pAddrRef1 - 1;
    uint32_t* pSlotStartNew = pRefNew - 1;
    
    EXPECT_GE(pSlotStartNew, pBlockStart);
    EXPECT_LT(pSlotStartNew, pBlockStart + (pool.Size() * pool.PoolSize()));
}

TEST(STATICREFTAB_Tests, BlockAllocationAndDeallocation)
{
    STATICREFTAB pool(2, 0);

    uint32_t* pSlotA = pool.Add(111);
    uint32_t* pSlotB = pool.Add(222);

    uint32_t* pSlotC = pool.Add(333);
    EXPECT_EQ(pool.Count(), 3);

    pool.DelRefPtr(pSlotB);
    EXPECT_EQ(pool.Count(), 2);

    pool.DelRefPtr(pSlotA);
    EXPECT_EQ(pool.Count(), 1);
    EXPECT_EQ(*(pSlotC - 1), 333);
}

TEST(STATICREFTAB_Tests, CustomUserDataHandling)
{
    STATICREFTAB pool(2, 2);

    EXPECT_EQ(pool.Size(), 4);

    uint32_t* pPayload = pool.Add(0xABC);
    
    pPayload[0] = 0x11111111;
    pPayload[1] = 0x22222222;

    EXPECT_EQ(*(pPayload - 1), 0xABC);
    EXPECT_EQ(pPayload[0], 0x11111111);
    EXPECT_EQ(pPayload[1], 0x22222222);

    uint32_t* pPayloadNext = pool.Add(0xABC);
    uint32_t* pSlotStart1 = pPayload - 1;
    uint32_t* pSlotStart2 = pPayloadNext - 1;

    EXPECT_EQ(pSlotStart1 + pool.Size(), pSlotStart2);

    pool.DelRefPtr(pPayload);
}

TEST(STATICREFTAB_Tests, ClearOperations)
{
    STATICREFTAB pool(4, 0);

    pool.Add(10);
    pool.Add(20);
    pool.Add(30);
    EXPECT_EQ(pool.Count(), 3);

    pool.Clear();
    EXPECT_EQ(pool.Count(), 0);
}