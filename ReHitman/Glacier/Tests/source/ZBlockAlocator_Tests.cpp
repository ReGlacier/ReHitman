#include <Glacier/PF4/ZData.h>
#include <gtest/gtest.h>

#include <cstdint>

using namespace Glacier::PF4;

namespace
{
    struct AlocatorFixture
    {
        static constexpr int kBlockSize = 2;
        static constexpr int kMaxBlocks = 3;
        static constexpr int kPoolRefs = kBlockSize * kMaxBlocks;

        int16_t Stack[kMaxBlocks]{};
        ZDataRef Pool[kPoolRefs]{};
        ZBlockAlocator Alloc;

        AlocatorFixture()
        {
            Alloc.m_BlockSize = kBlockSize;
            Alloc.m_MaxBlocks = kMaxBlocks;
            Alloc.Init(Stack, Pool);
        }
    };

    void ExpectAllFreed(AlocatorFixture& fixture)
    {
        EXPECT_EQ(fixture.Alloc.m_Count, AlocatorFixture::kMaxBlocks);
        EXPECT_EQ(fixture.Alloc.m_Data, fixture.Pool);
        EXPECT_EQ(fixture.Alloc.m_Stack, fixture.Stack);
    }
}

TEST(ZBlockAlocator, InitWiresDataAndFillsFreeStack)
{
    AlocatorFixture fixture;

    ExpectAllFreed(fixture);
    for (int i = 0; i < AlocatorFixture::kMaxBlocks; ++i)
    {
        EXPECT_EQ(fixture.Stack[i], i);
    }
}

TEST(ZBlockAlocator, AllocHandsOutEveryBlockAndThenReturnsNull)
{
    AlocatorFixture fixture;

    ZDataRef* pBlock2 = fixture.Alloc.Alloc();
    ZDataRef* pBlock1 = fixture.Alloc.Alloc();
    ZDataRef* pBlock0 = fixture.Alloc.Alloc();

    // Free stack is popped from the end: block 2, then 1, then 0.
    EXPECT_EQ(pBlock2, &fixture.Pool[2 * 2]);
    EXPECT_EQ(pBlock1, &fixture.Pool[1 * 2]);
    EXPECT_EQ(pBlock0, &fixture.Pool[0]);

    EXPECT_EQ(fixture.Alloc.Alloc(), nullptr);
    EXPECT_EQ(fixture.Alloc.m_Count, 0);
}

TEST(ZBlockAlocator, FreeReturnsBlockAndAllocIsLifo)
{
    AlocatorFixture fixture;

    ZDataRef* pBlock2 = fixture.Alloc.Alloc();
    ZDataRef* pBlock1 = fixture.Alloc.Alloc();
    ZDataRef* pBlock0 = fixture.Alloc.Alloc();

    fixture.Alloc.Free(pBlock2);
    fixture.Alloc.Free(pBlock1);
    fixture.Alloc.Free(pBlock0);

    ExpectAllFreed(fixture);

    // Blocks are handed out again in LIFO order.
    EXPECT_EQ(fixture.Alloc.Alloc(), pBlock0);
    EXPECT_EQ(fixture.Alloc.Alloc(), pBlock1);
    EXPECT_EQ(fixture.Alloc.Alloc(), pBlock2);
    EXPECT_EQ(fixture.Alloc.Alloc(), nullptr);
}

TEST(ZBlockAlocator, BelongsToOnlyOwnsItsRegion)
{
    AlocatorFixture fixture;
    ZDataRef otherPool[AlocatorFixture::kPoolRefs]{};

    ZDataRef* pBlock = fixture.Alloc.Alloc();
    EXPECT_TRUE(fixture.Alloc.BelongsTo(pBlock));
    // Interior refs of the region also belong even if not block starts.
    EXPECT_TRUE(fixture.Alloc.BelongsTo(&fixture.Pool[AlocatorFixture::kPoolRefs - 1]));

    EXPECT_FALSE(fixture.Alloc.BelongsTo(otherPool));
}

TEST(ZBlockAlocator, BlocksAreSpacedByBlockSizeRefs)
{
    AlocatorFixture fixture;

    ZDataRef* pBlock2 = fixture.Alloc.Alloc();
    ZDataRef* pBlock1 = fixture.Alloc.Alloc();

    const auto* pBytes2 = reinterpret_cast<const char*>(pBlock2);
    const auto* pBytes1 = reinterpret_cast<const char*>(pBlock1);
    EXPECT_EQ(pBytes2 - pBytes1, AlocatorFixture::kBlockSize * static_cast<ptrdiff_t>(sizeof(ZDataRef)));
}
