#include <Glacier/Render/ZRIndexContainer.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZRIndexSubRange) == 0x8);
    static_assert(sizeof(ZRIndexContainer) == 0x1C);

    // Packed index stream layout: [numSubRanges][count0][indices0...][count1][indices1...]...
    // The pool is reset to a non-zero base because the original Release()
    // frees memory only when the first sub-range offset is non-zero.
    void ResetPoolToNonZeroBase(ZRX86AllocIf& allocIf, uint32_t lStartOffset, uint32_t lEndOffset)
    {
        allocIf.m_Allocator.Reset(lStartOffset, lEndOffset);
    }
}

TEST(ZRIndexContainer, CreateWithSingleSubRangeUsesInlineStorage)
{
    ZRX86AllocIf allocIf(0x1000);
    ResetPoolToNonZeroBase(allocIf, 0x100, 0x1100);
    const uint16_t packed[] = { 1, 3, 10, 20, 30 };

    ZRIndexContainer container;
    container.Create(packed, &allocIf, 42);

    EXPECT_EQ(container.m_lSubRangeCount, 1u);
    EXPECT_EQ(container.m_lTotalIndexCount, 3u);
    EXPECT_EQ(container.m_lFlags, 42u);
    EXPECT_EQ(container.m_pAllocator, &allocIf);
    EXPECT_EQ(container.m_pSubRanges, &container.m_InlineSubRange);
    EXPECT_EQ(container.m_pSubRanges[0].m_lIndexCount, 3u);
    EXPECT_EQ(container.m_pSubRanges[0].m_lIndexOffset, 0x100u);

    container.Release();
}

TEST(ZRIndexContainer, CreateWithMultipleSubRangesAssignsSequentialOffsets)
{
    ZRX86AllocIf allocIf(0x1000);
    ResetPoolToNonZeroBase(allocIf, 0x100, 0x1100);
    const uint16_t packed[] = { 3, 2, 100, 101, 3, 200, 201, 202, 1, 55 };

    ZRIndexContainer container;
    container.Create(packed, &allocIf, 0);

    EXPECT_EQ(container.m_lSubRangeCount, 3u);
    EXPECT_EQ(container.m_lTotalIndexCount, 6u);
    ASSERT_NE(container.m_pSubRanges, nullptr);
    EXPECT_NE(container.m_pSubRanges, &container.m_InlineSubRange);

    EXPECT_EQ(container.m_pSubRanges[0].m_lIndexCount, 2u);
    EXPECT_EQ(container.m_pSubRanges[0].m_lIndexOffset, 0x100u);
    EXPECT_EQ(container.m_pSubRanges[1].m_lIndexCount, 3u);
    EXPECT_EQ(container.m_pSubRanges[1].m_lIndexOffset, 0x102u);
    EXPECT_EQ(container.m_pSubRanges[2].m_lIndexCount, 1u);
    EXPECT_EQ(container.m_pSubRanges[2].m_lIndexOffset, 0x105u);

    container.Release();
}

TEST(ZRIndexContainer, CreateWithZeroTotalIndicesClearsInlineSubRange)
{
    ZRX86AllocIf allocIf(0x1000);
    ResetPoolToNonZeroBase(allocIf, 0x100, 0x1100);
    const uint16_t packed[] = { 1, 0 };

    ZRIndexContainer container;
    container.Create(packed, &allocIf, 0);

    EXPECT_EQ(container.m_lSubRangeCount, 1u);
    EXPECT_EQ(container.m_lTotalIndexCount, 0u);
    EXPECT_EQ(container.m_pSubRanges, nullptr);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000);
}

TEST(ZRIndexContainer, CreateWithZeroTotalIndicesFreesHeapSubRanges)
{
    ZRX86AllocIf allocIf(0x1000);
    ResetPoolToNonZeroBase(allocIf, 0x100, 0x1100);
    const uint16_t packed[] = { 2, 0, 0 };

    ZRIndexContainer container;
    container.Create(packed, &allocIf, 0);

    EXPECT_EQ(container.m_lSubRangeCount, 2u);
    EXPECT_EQ(container.m_lTotalIndexCount, 0u);
    EXPECT_EQ(container.m_pSubRanges, nullptr);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000);
}

TEST(ZRIndexContainer, CreateClearsSubRangesWhenAllocatorIsFull)
{
    ZRX86AllocIf allocIf(0x1000);
    ResetPoolToNonZeroBase(allocIf, 0x100, 0x104); // only 4 slots available
    const uint16_t packed[] = { 2, 3, 1, 2, 3, 3, 4, 5, 6 }; // total = 6

    ZRIndexContainer container;
    container.Create(packed, &allocIf, 0);

    EXPECT_EQ(container.m_lSubRangeCount, 2u);
    EXPECT_EQ(container.m_lTotalIndexCount, 6u);
    EXPECT_EQ(container.m_pSubRanges, nullptr);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 4);
}

TEST(ZRIndexContainer, CreateOnUsedContainerAsserts)
{
    ZRX86AllocIf allocIf(0x1000);
    ResetPoolToNonZeroBase(allocIf, 0x100, 0x1100);
    const uint16_t packed[] = { 1, 3, 10, 20, 30 };

    ZRIndexContainer container;
    container.Create(packed, &allocIf, 0);

    EXPECT_THROW(container.Create(packed, &allocIf, 0), std::runtime_error);

    container.Release();
}

TEST(ZRIndexContainer, ReleaseReturnsMemoryToAllocatorAndClearsState)
{
    ZRX86AllocIf allocIf(0x1000);
    ResetPoolToNonZeroBase(allocIf, 0x100, 0x1100);
    const uint16_t packed[] = { 3, 2, 100, 101, 3, 200, 201, 202, 1, 55 };

    ZRIndexContainer container;
    container.Create(packed, &allocIf, 0);
    ASSERT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000 - 6);

    container.Release();

    EXPECT_EQ(container.m_pSubRanges, nullptr);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000);

    // Double release is a no-op.
    container.Release();
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000);
}

TEST(ZRIndexContainer, ReleaseOnEmptyContainerIsNoOp)
{
    ZRX86AllocIf allocIf(0x1000);
    ResetPoolToNonZeroBase(allocIf, 0x100, 0x1100);

    ZRIndexContainer container;
    container.Release();

    EXPECT_EQ(container.m_pSubRanges, nullptr);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000);
}

TEST(ZRIndexContainer, ReleaseWithZeroBaseOffsetSkipsAllocatorFree)
{
    // PC quirk: Release frees the index range only when the first sub-range
    // offset is non-zero, so a range allocated at offset 0 is not returned.
    ZRX86AllocIf allocIf(3); // pool starts at offset 0
    const uint16_t packed[] = { 1, 3, 10, 20, 30 };

    ZRIndexContainer container;
    container.Create(packed, &allocIf, 0);
    ASSERT_EQ(container.m_pSubRanges[0].m_lIndexOffset, 0u);

    container.Release();

    EXPECT_EQ(container.m_pSubRanges, nullptr);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0);
}
