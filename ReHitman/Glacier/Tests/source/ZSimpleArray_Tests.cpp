#include <Glacier/ZSTL/ZSimpleArray.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <vector>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZSimpleArray<void*>) == 0x8);

    struct TrackedValue
    {
        static int LiveCount;
        static int CopyCount;
        static int DestructorCount;

        int Value = 0;

        TrackedValue() { ++LiveCount; }
        explicit TrackedValue(int value) : Value(value) { ++LiveCount; }
        TrackedValue(const TrackedValue& other) : Value(other.Value)
        {
            ++LiveCount;
            ++CopyCount;
        }
        ~TrackedValue()
        {
            --LiveCount;
            ++DestructorCount;
        }

        TrackedValue& operator=(const TrackedValue& other)
        {
            Value = other.Value;
            return *this;
        }

        static void Reset()
        {
            LiveCount = 0;
            CopyCount = 0;
            DestructorCount = 0;
        }
    };

    int TrackedValue::LiveCount = 0;
    int TrackedValue::CopyCount = 0;
    int TrackedValue::DestructorCount = 0;
}

TEST(ZSimpleArray, DefaultConstructorCreatesEmptyArray)
{
    ZSimpleArray<int> array;

    EXPECT_EQ(array.m_Array, nullptr);
    EXPECT_EQ(array.GetSize(), 0u);
    EXPECT_EQ(array.Begin(), nullptr);
    EXPECT_EQ(array.End(), nullptr);
}

TEST(ZSimpleArray, ResizeGrowCopiesExistingValuesAndFillsNewSlots)
{
    ZSimpleArray<int> array;
    int init = 7;

    array.Resize(2, init);
    array[0] = 10;
    array[1] = 20;

    array.Resize(4, init);

    EXPECT_EQ(array.GetSize(), 4u);
    EXPECT_EQ(array[0], 10);
    EXPECT_EQ(array[1], 20);
    EXPECT_EQ(array[2], 7);
    EXPECT_EQ(array[3], 7);
    EXPECT_EQ(array.End(), array.Begin() + 4);
}

TEST(ZSimpleArray, ResizeToSameSizeKeepsExistingStorageAndValues)
{
    ZSimpleArray<int> array;
    int init = 3;

    array.Resize(2, init);
    array[0] = 8;
    array[1] = 9;
    int* original = array.m_Array;

    array.Resize(2, init);

    EXPECT_EQ(array.m_Array, original);
    EXPECT_EQ(array[0], 8);
    EXPECT_EQ(array[1], 9);
}

TEST(ZSimpleArray, ResizeShrinkKeepsPrefix)
{
    ZSimpleArray<int> array;
    int init = 1;

    array.Resize(4, init);
    array[0] = 3;
    array[1] = 4;
    array[2] = 5;
    array[3] = 6;

    array.Resize(2, init);

    EXPECT_EQ(array.GetSize(), 2u);
    EXPECT_EQ(array[0], 3);
    EXPECT_EQ(array[1], 4);
}

TEST(ZSimpleArray, ResizeToZeroFreesArray)
{
    ZSimpleArray<int> array;
    int init = 1;

    array.Resize(3, init);
    ASSERT_NE(array.m_Array, nullptr);

    array.Resize(0, init);

    EXPECT_EQ(array.GetSize(), 0u);
    EXPECT_EQ(array.m_Array, nullptr);
}

TEST(ZSimpleArray, OperatorIndexAssertsOutOfRange)
{
    ZSimpleArray<int> array;
    int init = 0;
    array.Resize(1, init);

    EXPECT_NO_THROW((void)array[0]);
    EXPECT_THROW((void)array[1], std::runtime_error);
}

TEST(ZSimpleArray, AllocateSetsRawStorageAndSize)
{
    ZSimpleArray<int> array;

    array.Allocate(3);

    EXPECT_NE(array.m_Array, nullptr);
    EXPECT_EQ(array.GetSize(), 3u);

    for (uint32_t i = 0; i < array.GetSize(); ++i)
        new (&array[i]) int(static_cast<int>(i + 1));

    EXPECT_EQ(array[2], 3);
}

TEST(ZSimpleArray, RangeForIteratesMutableValues)
{
    ZSimpleArray<int> array;
    int init = 2;
    array.Resize(3, init);

    int next = 1;
    for (auto& value : array)
        value = next++;

    EXPECT_EQ(array[0], 1);
    EXPECT_EQ(array[1], 2);
    EXPECT_EQ(array[2], 3);
}

TEST(ZSimpleArray, RangeForIteratesConstValues)
{
    ZSimpleArray<int> array;
    int init = 4;
    array.Resize(3, init);
    array[1] = 5;

    const auto& constArray = array;
    int sum = 0;

    for (const auto& value : constArray)
        sum += value;

    EXPECT_EQ(sum, 13);
    EXPECT_EQ(constArray.cbegin(), constArray.Begin());
    EXPECT_EQ(constArray.cend(), constArray.End());
}

TEST(ZSimpleArray, EmptyRangeForDoesNotIterate)
{
    ZSimpleArray<int> array;
    int count = 0;

    for (const auto& value : array)
    {
        (void)value;
        ++count;
    }

    EXPECT_EQ(count, 0);
}

TEST(ZSimpleArray, PointerElementsResizeAndIterate)
{
    ZSimpleArray<int*> array;
    int first = 1;
    int second = 2;
    int* init = &first;

    array.Resize(2, init);
    array[1] = &second;

    std::vector<int> values;
    for (int* value : array)
        values.push_back(*value);

    ASSERT_EQ(values.size(), 2u);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
}

TEST(ZSimpleArray, BeginEndSupportStandardIteratorAlgorithms)
{
    ZSimpleArray<int> array;
    int init = 1;
    array.Resize(4, init);

    EXPECT_EQ(std::distance(array.begin(), array.end()), 4);
    EXPECT_EQ(std::distance(array.cbegin(), array.cend()), 4);
}

TEST(ZSimpleArray, NonTrivialElementsAreCopiedAndDestroyed)
{
    TrackedValue::Reset();

    {
        ZSimpleArray<TrackedValue> array;
        TrackedValue init(5);

        array.Resize(2, init);
        EXPECT_EQ(array.GetSize(), 2u);
        EXPECT_EQ(array[0].Value, 5);
        EXPECT_EQ(array[1].Value, 5);
        EXPECT_EQ(TrackedValue::LiveCount, 3);

        array[0].Value = 10;
        array.Resize(3, init);

        EXPECT_EQ(array[0].Value, 10);
        EXPECT_EQ(array[1].Value, 5);
        EXPECT_EQ(array[2].Value, 5);
    }

    EXPECT_EQ(TrackedValue::LiveCount, 0);
    EXPECT_GT(TrackedValue::CopyCount, 0);
    EXPECT_GT(TrackedValue::DestructorCount, 0);
}
