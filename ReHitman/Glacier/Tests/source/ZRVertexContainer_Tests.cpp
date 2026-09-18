#include <Glacier/Render/ZRVertexContainer.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZRVertexContainer) == 0x18);
    static_assert(offsetof(ZRVertexContainer, m_pAllocator) == 0x14);
}

TEST(ZRVertexContainer, CreateWithZeroVerticesReturnsZeroAndKeepsState)
{
    ZRX86AllocIf allocIf(0x1000);
    allocIf.m_Allocator.Reset(0x100, 0x1100);

    ZRVertexContainer container;
    EXPECT_EQ(container.Create(0, 32, &allocIf, 0), 0u);

    EXPECT_EQ(container.m_lNumVertices, 0u);
    EXPECT_EQ(container.m_lVertexOffset, 0u);
    EXPECT_EQ(container.m_lAllocateOffset, 0u);
    EXPECT_EQ(container.m_lAllocateSize, 0u);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000);
}

TEST(ZRVertexContainer, CreateWithoutAlignmentUsesRawOffset)
{
    ZRX86AllocIf allocIf(0x1000);
    allocIf.m_Allocator.Reset(0x100, 0x1100);

    ZRVertexContainer container;
    EXPECT_EQ(container.Create(10, 32, &allocIf, 0), 0x100u);

    EXPECT_EQ(container.m_lNumVertices, 10u);
    EXPECT_EQ(container.m_lVertexSize, 32u);
    EXPECT_EQ(container.m_lAllocateSize, 320u);
    EXPECT_EQ(container.m_lAllocateOffset, 0x100u);
    EXPECT_EQ(container.m_lVertexOffset, 0x100u);
    EXPECT_EQ(container.m_pAllocator, &allocIf);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000 - 320);

    container.Release();
}

TEST(ZRVertexContainer, CreateWithAlignmentAlignsVertexOffset)
{
    ZRX86AllocIf allocIf(0x1000);
    allocIf.m_Allocator.Reset(0x108, 0x1108);

    ZRVertexContainer container;
    // PC quirk: the returned value is allocateOffset / lAlignment,
    // not the aligned vertex offset stored in m_lVertexOffset.
    EXPECT_EQ(container.Create(4, 16, &allocIf, 0x40), 0x108u / 0x40u);

    EXPECT_EQ(container.m_lNumVertices, 4u);
    EXPECT_EQ(container.m_lAllocateSize, 0x40u + 64u);
    EXPECT_EQ(container.m_lAllocateOffset, 0x108u);
    EXPECT_EQ(container.m_lVertexOffset, 0x140u); // 0x40 + 0x108 - (0x108 % 0x40)

    container.Release();
}

TEST(ZRVertexContainer, CreateWithAlignedOffsetAdvancesByFullAlignment)
{
    // PC quirk: even when the allocate offset is already aligned,
    // the vertex offset advances by a whole alignment slot.
    ZRX86AllocIf allocIf(0x1000);
    allocIf.m_Allocator.Reset(0x100, 0x1100);

    ZRVertexContainer container;
    EXPECT_EQ(container.Create(4, 16, &allocIf, 0x40), 0x100u / 0x40u);

    EXPECT_EQ(container.m_lAllocateOffset, 0x100u);
    EXPECT_EQ(container.m_lVertexOffset, 0x140u);

    container.Release();
}

TEST(ZRVertexContainer, CreateResetsStateWhenAllocatorIsFull)
{
    ZRX86AllocIf allocIf(0x1000);
    allocIf.m_Allocator.Reset(0x100, 0x110); // only 16 bytes available

    ZRVertexContainer container;
    EXPECT_EQ(container.Create(4, 16, &allocIf, 0), 0xFFFFFFFFu);

    EXPECT_EQ(container.m_lNumVertices, 0u);
    EXPECT_EQ(container.m_lAllocateOffset, 0u);
    EXPECT_EQ(container.m_lAllocateSize, 0u);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 16);
}

TEST(ZRVertexContainer, CreateOnUsedContainerAsserts)
{
    ZRX86AllocIf allocIf(0x1000);
    allocIf.m_Allocator.Reset(0x100, 0x1100);

    ZRVertexContainer container;
    ASSERT_NE(container.Create(10, 32, &allocIf, 0), 0xFFFFFFFFu);

    EXPECT_THROW(container.Create(10, 32, &allocIf, 0), std::runtime_error);

    container.Release();
}

TEST(ZRVertexContainer, ReleaseReturnsMemoryToAllocatorAndClearsState)
{
    ZRX86AllocIf allocIf(0x1000);
    allocIf.m_Allocator.Reset(0x100, 0x1100);

    ZRVertexContainer container;
    ASSERT_EQ(container.Create(10, 32, &allocIf, 0), 0x100u);
    ASSERT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000 - 320);

    container.Release();

    EXPECT_EQ(container.m_lNumVertices, 0u);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000);

    // Double release is a no-op.
    container.Release();
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000);
}

TEST(ZRVertexContainer, ReleaseOnEmptyContainerIsNoOp)
{
    ZRX86AllocIf allocIf(0x1000);
    allocIf.m_Allocator.Reset(0x100, 0x1100);

    ZRVertexContainer container;
    container.Release();

    EXPECT_EQ(container.m_lNumVertices, 0u);
    EXPECT_EQ(allocIf.m_Allocator.GetFreeTotal(), 0x1000);
}
