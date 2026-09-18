#include <Glacier/ZSTL/ZStaticVector.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <vector>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZStaticVector<uint16_t, 8>) == 0x14);
    static_assert(sizeof(ZStaticVector<void*, 8>) == 0x24);
}

TEST(ZStaticVector, ConstructorInitializesEmptyVector)
{
    ZStaticVector<uint16_t, 8> vector;

    EXPECT_EQ(vector.size(), 0u);
    EXPECT_TRUE(vector.empty());
    EXPECT_TRUE(vector.IsEmpty());
    EXPECT_EQ(vector.begin(), vector.end());
    EXPECT_EQ(vector.capacity(), 8u);
}

TEST(ZStaticVector, PushBackAppendsValues)
{
    ZStaticVector<uint16_t, 8> vector;
    uint16_t first = 10;
    uint16_t second = 20;

    vector.push_back(first);
    vector.push_back(second);

    EXPECT_EQ(vector.size(), 2u);
    EXPECT_EQ(vector[0], 10u);
    EXPECT_EQ(vector[1], 20u);
    EXPECT_EQ(vector.end(), vector.begin() + 2);
}

TEST(ZStaticVector, PushBackAssertsWhenFull)
{
    ZStaticVector<int, 2> vector;

    vector.push_back(1);
    vector.push_back(2);

    EXPECT_THROW(vector.push_back(3), std::runtime_error);
}

TEST(ZStaticVector, PopBackReturnsOldSizeAndRemovesLast)
{
    ZStaticVector<int, 4> vector;
    vector.push_back(1);
    vector.push_back(2);

    EXPECT_EQ(vector.pop_back(), 2u);
    EXPECT_EQ(vector.size(), 1u);
    EXPECT_EQ(vector[0], 1);
    EXPECT_EQ(vector.pop_back(), 1u);
    EXPECT_EQ(vector.pop_back(), 0u);
    EXPECT_TRUE(vector.empty());
}

TEST(ZStaticVector, ClearResetsSizeOnly)
{
    ZStaticVector<int, 4> vector;
    vector.push_back(5);
    vector.push_back(6);

    vector.clear();

    EXPECT_EQ(vector.size(), 0u);
    EXPECT_TRUE(vector.empty());
}

TEST(ZStaticVector, EraseByIndexShiftsTail)
{
    ZStaticVector<int, 5> vector;
    for (int i = 0; i < 5; ++i)
        vector.push_back(i + 1);

    int* next = vector.erase(1);

    EXPECT_EQ(next, &vector[1]);
    ASSERT_EQ(vector.size(), 4u);
    EXPECT_EQ(vector[0], 1);
    EXPECT_EQ(vector[1], 3);
    EXPECT_EQ(vector[2], 4);
    EXPECT_EQ(vector[3], 5);
}

TEST(ZStaticVector, EraseEndReturnsEndWithoutChangingSize)
{
    ZStaticVector<int, 3> vector;
    vector.push_back(1);

    int* result = vector.erase(vector.size());

    EXPECT_EQ(result, vector.end());
    EXPECT_EQ(vector.size(), 1u);
}

TEST(ZStaticVector, EraseRangeRemovesMultipleElements)
{
    ZStaticVector<int, 6> vector;
    for (int i = 0; i < 6; ++i)
        vector.push_back(i + 1);

    int* next = vector.erase(vector.begin() + 1, vector.begin() + 4);

    EXPECT_EQ(next, &vector[1]);
    ASSERT_EQ(vector.size(), 3u);
    EXPECT_EQ(vector[0], 1);
    EXPECT_EQ(vector[1], 5);
    EXPECT_EQ(vector[2], 6);
}

TEST(ZStaticVector, RangeForIteratesElements)
{
    ZStaticVector<int, 4> vector;
    vector.push_back(1);
    vector.push_back(2);
    vector.push_back(3);

    int sum = 0;
    for (const auto& value : vector)
        sum += value;

    EXPECT_EQ(sum, 6);
}

TEST(ZStaticVector, CopyConstructorCopiesOnlyUsedElements)
{
    ZStaticVector<int, 4> source;
    source.push_back(7);
    source.push_back(8);

    ZStaticVector<int, 4> copy(source);

    EXPECT_EQ(copy.size(), 2u);
    EXPECT_EQ(copy[0], 7);
    EXPECT_EQ(copy[1], 8);
}

TEST(ZStaticVector, StdVectorSAliasMatchesZStaticVector)
{
    std::vectorS<int, 3> vector;
    vector.push_back(11);

    EXPECT_EQ(vector.size(), 1u);
    EXPECT_EQ(vector[0], 11);
}

TEST(ZStaticVector, PointerElementsAreStoredInline)
{
    ZStaticVector<int*, 2> vector;
    int first = 1;
    int second = 2;

    vector.push_back(&first);
    vector.push_back(&second);

    std::vector<int> values;
    for (int* value : vector)
        values.push_back(*value);

    ASSERT_EQ(values.size(), 2u);
    EXPECT_EQ(values[0], 1);
    EXPECT_EQ(values[1], 2);
}
