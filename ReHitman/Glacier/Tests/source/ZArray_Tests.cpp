#include <Glacier/ZSTL/ZArray.h>
#include <gtest/gtest.h>

#include <cstdint>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZArray<int>) == 0x1C);

    struct TrackedValue
    {
        static int LiveCount;
        static int ConstructorCount;
        static int DestructorCount;
        static int AssignmentCount;

        int Value = 0;

        TrackedValue()
        {
            ++LiveCount;
            ++ConstructorCount;
        }

        explicit TrackedValue(int value) : Value(value)
        {
            ++LiveCount;
            ++ConstructorCount;
        }

        TrackedValue(const TrackedValue& other) : Value(other.Value)
        {
            ++LiveCount;
            ++ConstructorCount;
        }

        ~TrackedValue()
        {
            --LiveCount;
            ++DestructorCount;
        }

        TrackedValue& operator=(const TrackedValue& other)
        {
            Value = other.Value;
            ++AssignmentCount;
            return *this;
        }

        static void ResetCounters()
        {
            LiveCount = 0;
            ConstructorCount = 0;
            DestructorCount = 0;
            AssignmentCount = 0;
        }
    };

    int TrackedValue::LiveCount = 0;
    int TrackedValue::ConstructorCount = 0;
    int TrackedValue::DestructorCount = 0;
    int TrackedValue::AssignmentCount = 0;
}

TEST(ZArray, ConstructorInitializesEmptyBlockArray)
{
    ZArray<int> array;

    EXPECT_EQ(array.Count(), 0u);
    EXPECT_EQ(array.GetSize(), 0u);
    EXPECT_EQ(array.m_ItemSize, sizeof(int));
    EXPECT_EQ(array.m_BlockSizeExhibitor, 5u);
    EXPECT_EQ(array.m_SubIndexMask, 31u);
    EXPECT_EQ(array.m_PrimaryArrayCapacity, 0u);
    EXPECT_EQ(array.m_PrimaryArray, nullptr);
}

TEST(ZArray, AddAppendsItemsAndReturnsStoredReference)
{
    ZArray<int> array;

    int& first = array.Add(10);
    int& second = array.Add(20);
    int& third = array.Add(30);

    EXPECT_EQ(array.Count(), 3u);
    EXPECT_EQ(first, 10);
    EXPECT_EQ(second, 20);
    EXPECT_EQ(third, 30);
    EXPECT_EQ(array[0], 10);
    EXPECT_EQ(array[1], 20);
    EXPECT_EQ(array[2], 30);
}

TEST(ZArray, AddAllocatesMultipleBlocksAcrossDefaultBlockBoundary)
{
    ZArray<int> array;

    for (int i = 0; i < 33; ++i)
    {
        array.Add(i * 10);
    }

    ASSERT_EQ(array.Count(), 33u);
    EXPECT_GE(array.m_PrimaryArrayCapacity, 2u);
    ASSERT_NE(array.m_PrimaryArray, nullptr);
    EXPECT_NE(array.m_PrimaryArray[0], nullptr);
    EXPECT_NE(array.m_PrimaryArray[1], nullptr);
    EXPECT_EQ(array[0], 0);
    EXPECT_EQ(array[31], 310);
    EXPECT_EQ(array[32], 320);
}

TEST(ZArray, IndexOperatorExpandsArrayAndDefaultConstructsGap)
{
    ZArray<int> array;

    array[5] = 55;

    EXPECT_EQ(array.Count(), 6u);
    EXPECT_EQ(array[0], 0);
    EXPECT_EQ(array[4], 0);
    EXPECT_EQ(array[5], 55);
}

TEST(ZArray, ConstIndexOperatorReadsExistingItems)
{
    ZArray<int> array;
    array.Add(11);
    array.Add(22);

    const ZArray<int>& constArray = array;

    EXPECT_EQ(constArray[0], 11);
    EXPECT_EQ(constArray[1], 22);
}

TEST(ZArray, RemoveByIdxShiftsItemsLeftAndShrinks)
{
    ZArray<int> array;

    array.Add(10);
    array.Add(20);
    array.Add(30);
    array.Add(40);

    array.RemoveByIdx(1);

    ASSERT_EQ(array.Count(), 3u);
    EXPECT_EQ(array[0], 10);
    EXPECT_EQ(array[1], 30);
    EXPECT_EQ(array[2], 40);
}

TEST(ZArray, RemoveLastOnlyShrinks)
{
    ZArray<int> array;

    array.Add(10);
    array.Add(20);
    array.RemoveByIdx(1);

    ASSERT_EQ(array.Count(), 1u);
    EXPECT_EQ(array[0], 10);
}

TEST(ZArray, RemoveByIdxRejectsOutOfRangeIndex)
{
    ZArray<int> array;
    array.Add(10);

    EXPECT_THROW(array.RemoveByIdx(1), std::runtime_error);
}

TEST(ZArray, ResizeDownDestructsRemovedItems)
{
    TrackedValue::ResetCounters();

    {
        ZArray<TrackedValue> array;
        TrackedValue value1(1);
        TrackedValue value2(2);
        TrackedValue value3(3);

        array.Add(value1);
        array.Add(value2);
        array.Add(value3);

        EXPECT_EQ(array.Count(), 3u);
        EXPECT_EQ(TrackedValue::LiveCount, 6);

        array.Resize(1);

        EXPECT_EQ(array.Count(), 1u);
        EXPECT_EQ(TrackedValue::LiveCount, 4);
        EXPECT_GE(TrackedValue::DestructorCount, 2);
        EXPECT_EQ(array[0].Value, 1);
    }

    EXPECT_EQ(TrackedValue::LiveCount, 0);
}

TEST(ZArray, DestructorCleansUpConstructedItems)
{
    TrackedValue::ResetCounters();

    {
        ZArray<TrackedValue> array;
        TrackedValue value1(1);
        TrackedValue value2(2);

        array.Add(value1);
        array.Add(value2);

        EXPECT_EQ(TrackedValue::LiveCount, 4);
    }

    EXPECT_EQ(TrackedValue::LiveCount, 0);
    EXPECT_EQ(TrackedValue::ConstructorCount, TrackedValue::DestructorCount);
}

TEST(ZArray, CleanupAllowsReuse)
{
    ZArray<int> array;

    array.Add(10);
    array.Add(20);
    array.Cleanup();

    EXPECT_EQ(array.Count(), 0u);
    EXPECT_EQ(array.m_PrimaryArray, nullptr);

    array.Add(30);

    EXPECT_EQ(array.Count(), 1u);
    EXPECT_EQ(array[0], 30);
}
