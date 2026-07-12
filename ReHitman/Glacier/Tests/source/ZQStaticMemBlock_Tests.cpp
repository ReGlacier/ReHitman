#include <Glacier/ZSTL/ZQStaticMemBlock.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <gtest/gtest.h>

#include <cstdint>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZQStaticMemBlock) == 0x18);
}

TEST(ZQStaticMemBlock, ConstructorWithZeroSizeLeavesStorageEmpty)
{
    ZQStaticMemBlock block(0, nullptr);

    EXPECT_EQ(block.m_pStack, nullptr);
    EXPECT_EQ(block.m_pStart, nullptr);
    EXPECT_EQ(block.m_pOffsetAlloc, nullptr);
    EXPECT_EQ(block.m_lDynamicStart, 0u);
    EXPECT_EQ(block.m_lBufferSize, 0u);
    EXPECT_EQ(block.GetSize(), 0u);
    EXPECT_EQ(block.GetBufferPtr(), nullptr);
}

TEST(ZQStaticMemBlock, ConstructorAllocatesBufferAndOffsetAllocator)
{
    ZQStaticMemBlock block(256, nullptr);

    ASSERT_NE(block.m_pStart, nullptr);
    ASSERT_NE(block.m_pOffsetAlloc, nullptr);
    EXPECT_EQ(block.m_pStack, nullptr);
    EXPECT_EQ(block.m_lDynamicStart, 0u);
    EXPECT_EQ(block.m_lBufferSize, 256u);
    EXPECT_EQ(block.GetSize(), 256u);
    EXPECT_EQ(block.GetBufferPtr(), block.m_pStart);

    EXPECT_EQ(block.m_pOffsetAlloc->GetNrFreeLinks(), 1u);
    EXPECT_EQ(block.m_pOffsetAlloc->GetFreeTotal(), 256);
    EXPECT_EQ(block.m_pOffsetAlloc->GetLargestFreeBlock(), 256u);
    EXPECT_EQ(block.m_pOffsetAlloc->m_pLinks[0].m_lOffset, 0u);
    EXPECT_EQ(block.m_pOffsetAlloc->m_pLinks[0].m_lSize, 256u);
}

TEST(ZQStaticMemBlock, GetPtrReturnsBufferPointerPlusOffset)
{
    ZQStaticMemBlock block(64, nullptr);

    auto* buffer = static_cast<char*>(block.GetBufferPtr());
    ASSERT_NE(buffer, nullptr);

    EXPECT_EQ(block.GetPtr(0), buffer);
    EXPECT_EQ(block.GetPtr(16), buffer + 16);
    EXPECT_EQ(block.GetPtr(63), buffer + 63);
}
