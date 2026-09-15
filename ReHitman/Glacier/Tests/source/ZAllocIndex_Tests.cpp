#include <Glacier/GlacierFWD.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZSTL/ZAllocIndex.h>
#include <gtest/gtest.h>

#include <cstdint>

using namespace Glacier;

namespace
{
    constexpr uint32_t BitsOf(uint32_t capacity)
    {
        uint32_t bits = 0;
        while ((1u << bits) < capacity)
            ++bits;
        return bits;
    }

    uint32_t GenerationOf(const ZAllocIndex& alloc, uint32_t slot)
    {
        return alloc.m_pArray[slot] / static_cast<uint32_t>(alloc.m_lNrElements);
    }
}

TEST(ZAllocIndex, ConstructorCreatesEmptyPowerOfTwoPool)
{
    ZAllocIndex alloc(4);

    EXPECT_EQ(alloc.Count(), 0u);
    EXPECT_EQ(alloc.m_lNrElements, 16);
    EXPECT_EQ(alloc.m_lFirstFreeIndex, 0u);
    EXPECT_EQ(alloc.m_lActiveCount, 0u);
    ASSERT_NE(alloc.m_pArray, nullptr);
}

TEST(ZAllocIndex, NewHandsOutSlotsInOrder)
{
    ZAllocIndex alloc(2); // capacity 4

    EXPECT_EQ(alloc.New(), 0u);
    EXPECT_EQ(alloc.New(), 1u);
    EXPECT_EQ(alloc.New(), 2u);

    EXPECT_EQ(alloc.Count(), 3u);
}

TEST(ZAllocIndex, NewReturnsBareIndexAndStoresGenerationInSlot)
{
    ZAllocIndex alloc(4); // capacity 16

    const uint32_t first = alloc.New();
    const uint32_t second = alloc.New();

    EXPECT_EQ(first, 0u);
    EXPECT_EQ(second, 1u);

    // Slot word packs (generation << bits) | slot; first allocation has generation 1.
    EXPECT_EQ(alloc.m_pArray[first], 16u + first);
    EXPECT_EQ(alloc.m_pArray[second], 16u + second);
    EXPECT_EQ(GenerationOf(alloc, first), 1u);
}

TEST(ZAllocIndex, FreePushesSlotBackToHeadLIFO)
{
    ZAllocIndex alloc(3); // capacity 8

    const uint32_t first = alloc.New();
    const uint32_t second = alloc.New();
    alloc.New();

    alloc.Free(second);
    alloc.Free(first);

    EXPECT_EQ(alloc.Count(), 1u);
    EXPECT_EQ(alloc.New(), first);
    EXPECT_EQ(alloc.New(), second);
    EXPECT_EQ(alloc.Count(), 3u);
}

TEST(ZAllocIndex, FreePreservesAndBumpsGenerationOnReuse)
{
    ZAllocIndex alloc(4); // capacity 16

    const uint32_t slot = alloc.New();

    ASSERT_EQ(GenerationOf(alloc, slot), 1u);

    alloc.Free(slot);

    EXPECT_EQ(alloc.New(), slot);
    EXPECT_EQ(GenerationOf(alloc, slot), 2u);
    EXPECT_EQ(alloc.m_pArray[slot], 2u * 16u + slot);
}

TEST(ZAllocIndex, FreeMasksAwayGenerationBitsOfHandle)
{
    ZAllocIndex alloc(2); // capacity 4

    const uint32_t slot = alloc.New();
    ASSERT_EQ(slot, 0u);

    // Callers may pass a full handle; only the low index bits identify the slot.
    alloc.Free(slot | 0xFFFFFFF0u);

    EXPECT_EQ(alloc.Count(), 0u);
    EXPECT_EQ(alloc.New(), slot);
}

TEST(ZAllocIndex, FullPoolMarksEmptyFreeListAndNewAsserts)
{
    ZAllocIndex alloc(2); // capacity 4

    alloc.New();
    alloc.New();
    alloc.New();
    alloc.New();

    EXPECT_EQ(alloc.Count(), 4u);
    EXPECT_EQ(alloc.m_lFirstFreeIndex, 0xFFFFFFFFu);

    EXPECT_THROW(alloc.New(), std::runtime_error);
}

TEST(ZAllocIndex, FreeWithoutActiveSlotsAsserts)
{
    ZAllocIndex alloc(2);

    EXPECT_THROW(alloc.Free(0), std::runtime_error);
}

TEST(ZAllocIndex, CleanRestoresInitialState)
{
    ZAllocIndex alloc(3); // capacity 8

    const uint32_t slot = alloc.New();
    alloc.New();
    alloc.Free(slot);

    alloc.Clean();

    EXPECT_EQ(alloc.Count(), 0u);
    EXPECT_EQ(alloc.m_lFirstFreeIndex, 0u);

    EXPECT_EQ(alloc.New(), 0u);
    EXPECT_EQ(alloc.New(), 1u);
    EXPECT_EQ(alloc.New(), 2u);
}

TEST(ZAllocIndex, SingleSlotPoolWorks)
{
    ZAllocIndex alloc(0); // capacity 1

    EXPECT_EQ(alloc.New(), 0u);
    EXPECT_EQ(alloc.Count(), 1u);
    EXPECT_THROW(alloc.New(), std::runtime_error);

    alloc.Free(0);

    EXPECT_EQ(alloc.Count(), 0u);
    EXPECT_EQ(alloc.New(), 0u);
}

TEST(ZAllocIndex, ReusedSlotsNeverCollideWithActiveOnes)
{
    constexpr uint32_t capacity = 32;
    ZAllocIndex alloc(BitsOf(capacity));

    uint32_t slots[capacity];
    for (uint32_t i = 0; i < capacity; ++i)
        slots[i] = alloc.New();

    // Free every second slot, then reallocate: must get exactly the freed ones back.
    for (uint32_t i = 0; i < capacity; i += 2)
        alloc.Free(slots[i]);

    bool active[capacity] = {};
    for (uint32_t i = 1; i < capacity; i += 2)
        active[slots[i]] = true;

    for (uint32_t i = 0; i < capacity / 2; ++i)
    {
        const uint32_t reused = alloc.New();
        ASSERT_LT(reused, capacity);
        EXPECT_EQ(reused % 2, 0u); // only even slots were freed
        EXPECT_FALSE(active[reused]);
        active[reused] = true;
    }

    EXPECT_EQ(alloc.Count(), capacity);
    EXPECT_THROW(alloc.New(), std::runtime_error);
}
