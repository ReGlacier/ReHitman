#include <Glacier/ZSTL/ZFixedSizeMemoryManager.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

using namespace Glacier;

namespace
{
    struct TestItem
    {
        int value;
        int marker;
    };
}


TEST(ZFixedSizeMemoryManagerBase, ConstructorBuildsFreeListAndRoundsSmallBlocks)
{
    ZFixedSizeMemoryManagerBase manager(1, 0);

    EXPECT_TRUE(manager.m_FreeListValid);
    EXPECT_EQ(manager.m_BlockSize, 4u);
    EXPECT_EQ(manager.GetCapacity(), 1u);
    EXPECT_EQ(manager.Count(), 0u);
    ASSERT_NE(manager.GetBufferPtr(), nullptr);
    ASSERT_NE(manager.m_ID, nullptr);
    EXPECT_EQ(manager.m_ID[0], -1);
    EXPECT_EQ(manager.m_FirstFreeBlock, manager.GetBufferPtr());
    EXPECT_EQ(manager.m_LastFreeBlock, manager.GetBufferPtr());
}

TEST(ZFixedSizeMemoryManagerBase, AllocReturnsBlocksInIndexOrderUntilPoolIsFull)
{
    ZFixedSizeMemoryManagerBase manager(8, 3);

    void* first = manager.Alloc();
    void* second = manager.Alloc();
    void* third = manager.Alloc();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(third, nullptr);
    EXPECT_EQ(first, manager.Index2Address(0));
    EXPECT_EQ(second, manager.Index2Address(1));
    EXPECT_EQ(third, manager.Index2Address(2));
    EXPECT_EQ(manager.Count(), 3u);
    EXPECT_EQ(manager.Alloc(), nullptr);
    EXPECT_EQ(manager.m_FirstFreeBlock, nullptr);
    EXPECT_EQ(manager.m_LastFreeBlock, nullptr);
}

TEST(ZFixedSizeMemoryManagerBase, PtrRefRoundTripAndStaleRefInvalidation)
{
    ZFixedSizeMemoryManagerBase manager(8, 2);

    void* ptr = manager.Alloc();
    uint32_t ref = manager.Ptr2REF(ptr);

    EXPECT_EQ(ref, 0u);
    EXPECT_EQ(manager.REF2Index(ref), 0u);
    EXPECT_EQ(manager.REF2ID(ref), 0u);
    EXPECT_EQ(manager.REF2Ptr(ref), ptr);
    EXPECT_TRUE(manager.IsAllocated(ptr));

    manager.Free(ptr);

    EXPECT_EQ(manager.Count(), 0u);
    EXPECT_FALSE(manager.IsAllocated(ptr));
    EXPECT_EQ(manager.Ptr2REF(ptr), 0xFFFFFFFFu);
    EXPECT_EQ(manager.REF2Ptr(ref), nullptr);

    void* other = manager.Alloc();
    void* reused = manager.Alloc();
    uint32_t newRef = manager.Ptr2REF(reused);

    ASSERT_NE(other, nullptr);
    ASSERT_EQ(reused, ptr);
    EXPECT_EQ(newRef, 2u);
    EXPECT_EQ(manager.REF2ID(newRef), 1u);
    EXPECT_EQ(manager.REF2Ptr(ref), nullptr);
    EXPECT_EQ(manager.REF2Ptr(newRef), reused);
}

TEST(ZFixedSizeMemoryManagerBase, FreeAppendsBlockToFreeListTail)
{
    ZFixedSizeMemoryManagerBase manager(8, 3);
    void* first = manager.Alloc();
    void* second = manager.Alloc();
    void* third = manager.Alloc();

    manager.Free(second);

    EXPECT_EQ(manager.Count(), 2u);
    EXPECT_EQ(manager.Alloc(), second);
    EXPECT_EQ(manager.Alloc(), nullptr);

    manager.Free(first);
    manager.Free(third);

    EXPECT_EQ(manager.Alloc(), first);
    EXPECT_EQ(manager.Alloc(), third);
}

TEST(ZFixedSizeMemoryManagerBase, AllocDirectAllocatesRequestedReferenceAndRebuildsFreeList)
{
    ZFixedSizeMemoryManagerBase manager(8, 4);
    uint32_t directRef = 2u * manager.GetCapacity() + 3u;

    void* direct = manager.AllocDirect(directRef);

    EXPECT_FALSE(manager.m_FreeListValid);
    EXPECT_EQ(direct, manager.Index2Address(3));
    EXPECT_EQ(manager.Ptr2REF(direct), directRef);
    EXPECT_EQ(manager.Count(), 1u);

    void* next = manager.Alloc();

    EXPECT_TRUE(manager.m_FreeListValid);
    EXPECT_EQ(next, manager.Index2Address(0));
    EXPECT_NE(next, direct);
    EXPECT_EQ(manager.Count(), 2u);
}

TEST(ZFixedSizeMemoryManagerBase, InvalidOperationsAssert)
{
    ZFixedSizeMemoryManagerBase manager(8, 1);
    void* ptr = manager.Alloc();

    manager.Free(ptr);

    EXPECT_THROW(manager.Free(ptr), std::runtime_error);
    EXPECT_THROW(manager.Index2Address(1), std::runtime_error);
    EXPECT_THROW(manager.REF2Index(0xFFFFFFFFu), std::runtime_error);
    EXPECT_THROW(manager.Address2Index(static_cast<char*>(manager.GetBufferPtr()) + 1), std::runtime_error);
}

TEST(ZFixedSizeMemoryManager, TypedFacadeReturnsTypedPointers)
{
    ZFixedSizeMemoryManager<TestItem> manager(2);

    TestItem* first = manager.Alloc();
    TestItem* second = manager.Alloc();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(manager.m_BlockSize, sizeof(TestItem));
    EXPECT_EQ(first, manager.GetBufferPtr());
    EXPECT_EQ(second, manager.Index2Address(1));

    first->value = 10;
    second->value = 20;

    uint32_t firstRef = manager.Ptr2REF(first);
    uint32_t secondRef = manager.Ptr2REF(second);
    EXPECT_EQ(manager.REF2Ptr(firstRef)->value, 10);
    EXPECT_EQ(manager.REF2Ptr(secondRef)->value, 20);
}

TEST(ZFixedSizeMemoryManager, IteratorSkipsFreeSlotsAndReturnsTypedObjects)
{
    ZFixedSizeMemoryManager<TestItem> manager(4);

    TestItem* first = manager.Alloc();
    TestItem* second = manager.Alloc();
    TestItem* third = manager.Alloc();
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(third, nullptr);

    first->value = 1;
    second->value = 2;
    third->value = 3;

    manager.Free(second);

    std::vector<int> values;
    for (auto it = manager.Begin(); it != manager.End(); ++it)
    {
        values.push_back((*it)->value);
        EXPECT_EQ(it->marker, (*it)->marker);
    }

    ASSERT_EQ(values.size(), 2u);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 3);
}
