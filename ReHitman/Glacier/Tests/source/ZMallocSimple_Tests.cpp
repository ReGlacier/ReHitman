#include <Glacier/ZSTL/ZMallocSimple.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>

using namespace Glacier;

namespace
{
    // Matches the PC placement new: ZMallocSimple(mem, 0x800, 4).
    constexpr uint32_t kMaxFreeLinks = 0x800;
    constexpr uint32_t kAlignment = 4;

    static_assert(sizeof(ZMallocSimple) == 0x4010, "ZMallocSimple must match the PC .data block (0x1004 dwords)");

    uint32_t Align4(uint32_t value)
    {
        return (value + 3u) & ~3u;
    }

    class ZMallocSimplePool
    {
    public:
        explicit ZMallocSimplePool(uint32_t lPoolSize)
            : m_Allocator(kMaxFreeLinks, kAlignment)
        {
            m_Pool.fill(0);
            m_Allocator.AddBlock(m_Pool.data(), lPoolSize);
        }

        ZMallocSimple m_Allocator;
        std::array<char, 4096> m_Pool {};
    };
}

TEST(ZMallocSimple, ClassLayoutMatchesPcBinary)
{
    EXPECT_EQ(sizeof(SFreeLink), 0x8u);
    EXPECT_EQ(sizeof(ZMallocSimple), 0x4010u);
    EXPECT_EQ(offsetof(ZMallocSimple, m_lAlignment), 0x8u);
    EXPECT_EQ(offsetof(ZMallocSimple, m_lMaxNrFreeLinks), 0xCu);
}

TEST(ZMallocSimple, AddBlockRegistersWholeBlockAsSingleFreeLink)
{
    ZMallocSimple alloc(kMaxFreeLinks, kAlignment);
    std::array<char, 256> pool {};

    alloc.AddBlock(pool.data(), 200);

    EXPECT_EQ(alloc.m_lNrFreeLinks, 1u);
    EXPECT_EQ(alloc.m_lTotalSize, 200u);
    EXPECT_EQ(alloc.m_lAlignment, 4u);
    EXPECT_EQ(alloc.GetFreeTotal(), 200u);
    EXPECT_EQ(alloc.m_pFreeLinks[0].m_pFreeRam, pool.data());
    EXPECT_EQ(alloc.m_pFreeLinks[0].m_lFreeSize, 200u);
    EXPECT_TRUE(alloc.CheckAllFreed());
}

TEST(ZMallocSimple, AllocZeroSizeReturnsNullAndKeepsPoolUntouched)
{
    ZMallocSimplePool pool(256);

    char* pStart = nullptr;
    char* pEnd = nullptr;
    EXPECT_EQ(pool.m_Allocator.Alloc(0, pStart, pEnd), nullptr);
    EXPECT_EQ(pStart, nullptr);
    EXPECT_EQ(pEnd, nullptr);
    EXPECT_EQ(pool.m_Allocator.GetFreeTotal(), 256u);
    EXPECT_TRUE(pool.m_Allocator.CheckAllFreed());
}

TEST(ZMallocSimple, AllocFromEmptyPoolReturnsNull)
{
    ZMallocSimple alloc(kMaxFreeLinks, kAlignment);

    char* pStart = nullptr;
    char* pEnd = nullptr;
    EXPECT_EQ(alloc.Alloc(16, pStart, pEnd), nullptr);
    EXPECT_EQ(alloc.m_lNrFreeLinks, 0u);
}

TEST(ZMallocSimple, AllocLaysOutHeaderPayloadAndBlockRange)
{
    ZMallocSimplePool pool(256);

    char* pStart = nullptr;
    char* pEnd = nullptr;
    char* pPayload = pool.m_Allocator.Alloc(12, pStart, pEnd);

    // 12 payload bytes + 4 header bytes, aligned up to 4 -> 16 byte block.
    ASSERT_NE(pPayload, nullptr);
    EXPECT_EQ(pStart, pool.m_Pool.data());
    EXPECT_EQ(pPayload, pStart + 4);
    EXPECT_EQ(pEnd, pStart + 16);
    EXPECT_EQ(*reinterpret_cast<uint32_t*>(pStart), 16u);
    EXPECT_EQ(pool.m_Allocator.AllocSize(pPayload), 12u);
    EXPECT_EQ(pool.m_Allocator.GetFreeTotal(), 240u);
    EXPECT_FALSE(pool.m_Allocator.CheckAllFreed());
}

TEST(ZMallocSimple, AllocAbsorbsRemainderSmallerThanEightBytes)
{
    ZMallocSimplePool pool(256);

    char* pStart = nullptr;
    char* pEnd = nullptr;
    // 248 payload + 4 header = 252; remainder 256 - 252 = 4 (< 8) is absorbed.
    char* pPayload = pool.m_Allocator.Alloc(248, pStart, pEnd);

    ASSERT_NE(pPayload, nullptr);
    EXPECT_EQ(pEnd - pStart, 256);
    EXPECT_EQ(*reinterpret_cast<uint32_t*>(pStart), 256u);
    EXPECT_EQ(pool.m_Allocator.AllocSize(pPayload), 252u);
    EXPECT_EQ(pool.m_Allocator.GetFreeTotal(), 0u);
    EXPECT_EQ(pool.m_Allocator.m_lNrFreeLinks, 0u);
}

TEST(ZMallocSimple, AllocKeepsLargerRemainderInSameFreeLink)
{
    ZMallocSimplePool pool(256);

    char* pStart = nullptr;
    char* pEnd = nullptr;
    // 240 payload + 4 header = 244; remainder 256 - 244 = 12 (>= 8) stays free.
    char* pPayload = pool.m_Allocator.Alloc(240, pStart, pEnd);

    ASSERT_NE(pPayload, nullptr);
    EXPECT_EQ(pEnd - pStart, 244);
    EXPECT_EQ(pool.m_Allocator.m_lNrFreeLinks, 1u);
    EXPECT_EQ(pool.m_Allocator.m_pFreeLinks[0].m_pFreeRam, pEnd);
    EXPECT_EQ(pool.m_Allocator.m_pFreeLinks[0].m_lFreeSize, 12u);
    EXPECT_EQ(pool.m_Allocator.GetFreeTotal(), 12u);
}

TEST(ZMallocSimple, AllocPicksBestFittingFreeLink)
{
    ZMallocSimple alloc(kMaxFreeLinks, kAlignment);
    std::array<char, 512> pool {};

    alloc.AddBlock(pool.data(), 100);
    alloc.AddBlock(pool.data() + 100, 20);
    alloc.AddBlock(pool.data() + 120, 60);

    char* pStart = nullptr;
    char* pEnd = nullptr;
    // Needs a 48 byte block; best fit is the 60 byte link, not the 100 byte one.
    char* pPayload = alloc.Alloc(44, pStart, pEnd);

    ASSERT_NE(pPayload, nullptr);
    EXPECT_EQ(pStart, pool.data() + 120);
    EXPECT_EQ(alloc.GetFreeTotal(), 100u + 20u + 12u);
}

TEST(ZMallocSimple, FreeNullptrIsIgnored)
{
    ZMallocSimplePool pool(256);

    pool.m_Allocator.Free(nullptr);

    EXPECT_EQ(pool.m_Allocator.m_lNrFreeLinks, 1u);
    EXPECT_EQ(pool.m_Allocator.GetFreeTotal(), 256u);
}

TEST(ZMallocSimple, FreeNonAdjacentBlockCreatesNewFreeLink)
{
    ZMallocSimplePool pool(256);

    char* pStartA = nullptr;
    char* pStartB = nullptr;
    char* pEnd = nullptr;
    char* pA = pool.m_Allocator.Alloc(28, pStartA, pEnd); // [0, 32)
    char* pB = pool.m_Allocator.Alloc(28, pStartB, pEnd); // [32, 64)

    ASSERT_NE(pA, nullptr);
    ASSERT_NE(pB, nullptr);
    pool.m_Allocator.Free(pA);

    EXPECT_EQ(pool.m_Allocator.m_lNrFreeLinks, 2u);
    EXPECT_EQ(pool.m_Allocator.GetFreeTotal(), 224u);
    EXPECT_FALSE(pool.m_Allocator.CheckAllFreed());
}

TEST(ZMallocSimple, FreeCoalescesWithFollowingFreeLink)
{
    ZMallocSimplePool pool(256);

    char* pStartA = nullptr;
    char* pEnd = nullptr;
    char* pA = pool.m_Allocator.Alloc(28, pStartA, pEnd); // [0, 32)

    ASSERT_NE(pA, nullptr);
    pool.m_Allocator.Free(pA);

    EXPECT_EQ(pool.m_Allocator.m_lNrFreeLinks, 1u);
    EXPECT_EQ(pool.m_Allocator.m_pFreeLinks[0].m_pFreeRam, pool.m_Pool.data());
    EXPECT_EQ(pool.m_Allocator.m_pFreeLinks[0].m_lFreeSize, 256u);
    EXPECT_TRUE(pool.m_Allocator.CheckAllFreed());
}

TEST(ZMallocSimple, FreeCoalescesWithPrecedingFreeLink)
{
    ZMallocSimplePool pool(256);

    char* pStartA = nullptr;
    char* pStartB = nullptr;
    char* pEnd = nullptr;
    char* pA = pool.m_Allocator.Alloc(28, pStartA, pEnd); // [0, 32)
    char* pB = pool.m_Allocator.Alloc(28, pStartB, pEnd); // [32, 64)

    ASSERT_NE(pA, nullptr);
    ASSERT_NE(pB, nullptr);

    // Free A first: cannot merge (B sits between A and the free tail).
    pool.m_Allocator.Free(pA);
    ASSERT_EQ(pool.m_Allocator.m_lNrFreeLinks, 2u);

    // Free B: adjacent to both A and the tail -> all three merge into one.
    pool.m_Allocator.Free(pB);

    EXPECT_EQ(pool.m_Allocator.m_lNrFreeLinks, 1u);
    EXPECT_EQ(pool.m_Allocator.m_pFreeLinks[0].m_pFreeRam, pool.m_Pool.data());
    EXPECT_EQ(pool.m_Allocator.m_pFreeLinks[0].m_lFreeSize, 256u);
    EXPECT_TRUE(pool.m_Allocator.CheckAllFreed());
}

TEST(ZMallocSimple, AllocAfterFreeReusesCoalescedRange)
{
    ZMallocSimplePool pool(256);

    char* pStartA = nullptr;
    char* pEnd = nullptr;
    char* pA = pool.m_Allocator.Alloc(28, pStartA, pEnd);
    ASSERT_NE(pA, nullptr);
    pool.m_Allocator.Free(pA);

    char* pStartB = nullptr;
    char* pB = pool.m_Allocator.Alloc(60, pStartB, pEnd);

    ASSERT_NE(pB, nullptr);
    EXPECT_EQ(pStartB, pool.m_Pool.data());
    EXPECT_EQ(pool.m_Allocator.GetFreeTotal(), 192u);
}

TEST(ZMallocSimple, RemoveBlockDropsUntouchedBlock)
{
    ZMallocSimple alloc(kMaxFreeLinks, kAlignment);
    std::array<char, 256> poolA {};
    std::array<char, 128> poolB {};

    alloc.AddBlock(poolA.data(), 200);
    alloc.AddBlock(poolB.data(), 56);
    alloc.RemoveBlock(poolA.data());

    EXPECT_EQ(alloc.m_lNrFreeLinks, 1u);
    EXPECT_EQ(alloc.m_lTotalSize, 56u);
    EXPECT_EQ(alloc.GetFreeTotal(), 56u);
    EXPECT_EQ(alloc.m_pFreeLinks[0].m_pFreeRam, poolB.data());
}

TEST(ZMallocSimple, CheckAllFreedTracksWholePoolLifetime)
{
    ZMallocSimplePool pool(256);
    EXPECT_TRUE(pool.m_Allocator.CheckAllFreed());

    char* pStart = nullptr;
    char* pEnd = nullptr;
    char* pA = pool.m_Allocator.Alloc(28, pStart, pEnd);
    char* pB = pool.m_Allocator.Alloc(28, pStart, pEnd);
    ASSERT_NE(pA, nullptr);
    ASSERT_NE(pB, nullptr);
    EXPECT_FALSE(pool.m_Allocator.CheckAllFreed());

    pool.m_Allocator.Free(pA);
    EXPECT_FALSE(pool.m_Allocator.CheckAllFreed());

    pool.m_Allocator.Free(pB);
    EXPECT_TRUE(pool.m_Allocator.CheckAllFreed());
}

TEST(ZMallocSimple, ShrinkIsUnsupported)
{
    ZMallocSimplePool pool(256);

    char* pStart = nullptr;
    char* pEnd = nullptr;
    char* pPayload = pool.m_Allocator.Alloc(28, pStart, pEnd);
    ASSERT_NE(pPayload, nullptr);

    EXPECT_FALSE(pool.m_Allocator.Shrink(pPayload, 8));
}

TEST(ZMallocSimple, AllocFreeLinkThrowsWhenTableIsFull)
{
    ZMallocSimple alloc(2, kAlignment);

    SFreeLink* pFirst = alloc.AllocFreeLink();
    SFreeLink* pSecond = alloc.AllocFreeLink();
    EXPECT_EQ(pFirst, &alloc.m_pFreeLinks[0]);
    EXPECT_EQ(pSecond, &alloc.m_pFreeLinks[1]);
    EXPECT_EQ(alloc.m_lNrFreeLinks, 2u);
    EXPECT_THROW(alloc.AllocFreeLink(), std::runtime_error);
}

TEST(ZMallocSimple, FreeFreeLinkSwapRemovesWithLastLink)
{
    ZMallocSimple alloc(kMaxFreeLinks, kAlignment);
    std::array<char, 64> pool {};

    alloc.AddBlock(pool.data(), 10);
    alloc.AddBlock(pool.data() + 16, 20);
    alloc.AddBlock(pool.data() + 40, 30);

    alloc.FreeFreeLink(&alloc.m_pFreeLinks[0]);

    ASSERT_EQ(alloc.m_lNrFreeLinks, 2u);
    EXPECT_EQ(alloc.m_pFreeLinks[0].m_lFreeSize, 30u);
    EXPECT_EQ(alloc.m_pFreeLinks[1].m_lFreeSize, 20u);
    EXPECT_EQ(alloc.GetFreeTotal(), 50u);
}

TEST(ZMallocSimple, FreeFreeLinkOnLastLinkJustShrinksUsedRange)
{
    ZMallocSimple alloc(kMaxFreeLinks, kAlignment);
    std::array<char, 64> pool {};

    alloc.AddBlock(pool.data(), 10);
    alloc.AddBlock(pool.data() + 16, 20);

    alloc.FreeFreeLink(&alloc.m_pFreeLinks[1]);

    ASSERT_EQ(alloc.m_lNrFreeLinks, 1u);
    EXPECT_EQ(alloc.m_pFreeLinks[0].m_lFreeSize, 10u);
}

TEST(ZMallocSimple, FreeFreeLinkThrowsWhenNoLinksUsed)
{
    ZMallocSimple alloc(kMaxFreeLinks, kAlignment);

    EXPECT_THROW(alloc.FreeFreeLink(&alloc.m_pFreeLinks[0]), std::runtime_error);
}
