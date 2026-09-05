#include <Glacier/ZSTL/ZQElemsBuffer.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZQElemsBuffer) == 0x18);

    uint32_t OffsetOf(void* value)
    {
        return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(value));
    }
}

TEST(ZQElemsBuffer, AddAllocatesAlignedOffsetsFromStaticBlock)
{
    ZQElemsBuffer buffer(64, nullptr);

    EXPECT_EQ(OffsetOf(buffer.Add(16, false)), 0u);
    EXPECT_EQ(OffsetOf(buffer.Add(8, false)), 16u);

    ASSERT_NE(buffer.m_pOffsetAlloc, nullptr);
    EXPECT_EQ(buffer.m_pOffsetAlloc->GetFreeTotal(), 40);
    EXPECT_EQ(buffer.m_pOffsetAlloc->GetNrFreeLinks(), 1u);
    EXPECT_EQ(buffer.m_pOffsetAlloc->m_pLinks[0].m_lOffset, 24u);
    EXPECT_EQ(buffer.m_pOffsetAlloc->m_pLinks[0].m_lSize, 40u);
}

TEST(ZQElemsBuffer, RemoveByOffsetReturnsRangeToAllocator)
{
    ZQElemsBuffer buffer(64, nullptr);

    buffer.Add(16, false);
    buffer.Add(8, false);

    buffer.Remove(0, 16);

    ASSERT_NE(buffer.m_pOffsetAlloc, nullptr);
    EXPECT_EQ(buffer.m_pOffsetAlloc->GetFreeTotal(), 56);
    EXPECT_EQ(buffer.m_pOffsetAlloc->GetNrFreeLinks(), 2u);
    EXPECT_EQ(buffer.m_pOffsetAlloc->GetLargestFreeBlock(), 40u);
}

TEST(ZQElemsBuffer, RemoveByPointerConvertsPointerToOffset)
{
    ZQElemsBuffer buffer(64, nullptr);

    buffer.Add(16, false);
    buffer.Add(8, false);
    void* pSecondAlloc = buffer.GetPtr(16);

    buffer.Remove(pSecondAlloc, 8);

    ASSERT_NE(buffer.m_pOffsetAlloc, nullptr);
    EXPECT_EQ(buffer.m_pOffsetAlloc->GetNrFreeLinks(), 1u);
    EXPECT_EQ(buffer.m_pOffsetAlloc->GetFreeTotal(), 48);
    EXPECT_EQ(buffer.m_pOffsetAlloc->m_pLinks[0].m_lOffset, 16u);
    EXPECT_EQ(buffer.m_pOffsetAlloc->m_pLinks[0].m_lSize, 48u);
}

TEST(ZQElemsBuffer, InvalidRequestsAssert)
{
    ZQElemsBuffer buffer(64, nullptr);

    EXPECT_THROW(buffer.Add(6, false), std::runtime_error);
    EXPECT_THROW(buffer.Add(68, false), std::runtime_error);
    EXPECT_THROW(buffer.Remove(0, 6), std::runtime_error);
    EXPECT_THROW(buffer.Remove(-1, 4), std::runtime_error);
    EXPECT_THROW(buffer.Remove(64, 4), std::runtime_error);
}
