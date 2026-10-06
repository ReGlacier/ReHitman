#include <Glacier/GlacierFWD.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <gtest/gtest.h>

#include <cstdint>

using namespace Glacier;

namespace
{
    struct Payload
    {
        uint32_t Id;
        uint32_t Value;

        bool operator==(const Payload& other) const
        {
            return Id == other.Id && Value == other.Value;
        }
    };

    static_assert(sizeof(ZStackArray<4, uint32_t>) == sizeof(uint32_t) + sizeof(uint32_t) * 4);
    static_assert(sizeof(ZStackArray<3, Payload>) == sizeof(uint32_t) + sizeof(Payload) * 3);
    static_assert(sizeof(ZStackArrayInsert<4, uint32_t, int32_t>) == sizeof(ZStackArray<4, uint32_t>) + sizeof(int32_t) * 4);
    static_assert(sizeof(ZSafeStackArray<4, uint32_t>) == sizeof(ZStackArray<4, uint32_t>));
}

TEST(ZStackArray, ConstructorStartsEmptyWithFixedCapacity)
{
    ZStackArray<4, uint32_t> array;

    EXPECT_EQ(array.Count(), 0u);
    EXPECT_EQ(array.TotalNrEntries(), 4u);
    EXPECT_EQ(array.m_lNrEntries, 0u);
}

TEST(ZStackArray, AddAndPushAppendValuesAndReturnIndices)
{
    ZStackArray<4, uint32_t> array;

    uint32_t first = 10;

    EXPECT_EQ(array.Add(&first), 0u);
    EXPECT_EQ(array.Push(20), 1u);
    EXPECT_EQ(array.Push(30), 2u);

    EXPECT_EQ(array.Count(), 3u);
    EXPECT_EQ(*array.Get(0), 10u);
    EXPECT_EQ(*array.Get(1), 20u);
    EXPECT_EQ(*array.Get(2), 30u);
}

TEST(ZStackArray, GetConstReturnsStoredValue)
{
    ZStackArray<2, uint32_t> array;

    array.Push(42);

    const auto& constArray = array;

    ASSERT_NE(constArray.Get(0), nullptr);
    EXPECT_EQ(*constArray.Get(0), 42u);
}

TEST(ZStackArray, RemoveReplacesRemovedEntryWithLast)
{
    ZStackArray<5, uint32_t> array;

    array.Push(10);
    array.Push(20);
    array.Push(30);
    array.Push(40);

    array.Remove(1);

    EXPECT_EQ(array.Count(), 3u);
    EXPECT_EQ(*array.Get(0), 10u);
    EXPECT_EQ(*array.Get(1), 40u);
    EXPECT_EQ(*array.Get(2), 30u);
}

TEST(ZStackArray, RemoveLastOnlyDecrementsCount)
{
    ZStackArray<3, uint32_t> array;

    array.Push(10);
    array.Push(20);
    array.Remove(1);

    EXPECT_EQ(array.Count(), 1u);
    EXPECT_EQ(*array.Get(0), 10u);
}

TEST(ZStackArray, FindReturnsIndexOrMinusOne)
{
    ZStackArray<3, Payload> array;

    Payload first{1, 10};
    Payload second{2, 20};
    Payload missing{3, 30};

    array.Add(&first);
    array.Add(&second);

    EXPECT_EQ(array.Find(&first), 0);
    EXPECT_EQ(array.Find(&second), 1);
    EXPECT_EQ(array.Find(&missing), -1);
}

TEST(ZStackArray, PeekAndPopUseStackTop)
{
    ZStackArray<3, uint32_t> array;

    array.Push(10);
    array.Push(20);
    array.Push(30);

    EXPECT_EQ(*array.Peek(), 30u);
    EXPECT_EQ(array.Pop(), 30u);
    EXPECT_EQ(array.Count(), 2u);
    EXPECT_EQ(*array.Peek(), 20u);
}

TEST(ZStackArray, ClearResetsCountWithoutChangingCapacity)
{
    ZStackArray<3, uint32_t> array;

    array.Push(10);
    array.Push(20);
    array.Clear();

    EXPECT_EQ(array.Count(), 0u);
    EXPECT_EQ(array.TotalNrEntries(), 3u);

    array.Push(30);
    EXPECT_EQ(array.Count(), 1u);
    EXPECT_EQ(*array.Get(0), 30u);
}

TEST(ZStackArray, InvalidOperationsAssert)
{
    ZStackArray<1, uint32_t> array;

    EXPECT_THROW(array.Get(0), std::runtime_error);
    EXPECT_THROW(array.Peek(), std::runtime_error);
    EXPECT_THROW(array.Pop(), std::runtime_error);

    array.Push(10);

    EXPECT_THROW(array.Push(20), std::runtime_error);
    EXPECT_THROW(array.Get(1), std::runtime_error);
    EXPECT_THROW(array.Remove(1), std::runtime_error);
}

TEST(ZStackArrayInsert, AddKeepsElementsSortedByDescendingKey)
{
    ZStackArrayInsert<5, uint32_t, int32_t> array;

    uint32_t low = 10;
    uint32_t high = 30;
    uint32_t middle = 20;
    uint32_t equal = 21;

    EXPECT_EQ(array.Add(&low, 10), 0u);
    EXPECT_EQ(array.Add(&high, 30), 0u);
    EXPECT_EQ(array.Add(&middle, 20), 1u);
    EXPECT_EQ(array.Add(&equal, 20), 2u);

    EXPECT_EQ(array.Count(), 4u);
    EXPECT_EQ(*array.Get(0), 30u);
    EXPECT_EQ(*array.Get(1), 20u);
    EXPECT_EQ(*array.Get(2), 21u);
    EXPECT_EQ(*array.Get(3), 10u);

    EXPECT_EQ(array.GetSortValue(0), 30);
    EXPECT_EQ(array.GetSortValue(1), 20);
    EXPECT_EQ(array.GetSortValue(2), 20);
    EXPECT_EQ(array.GetSortValue(3), 10);
}

TEST(ZStackArrayInsert, RemovePreservesOrderAndSortValues)
{
    ZStackArrayInsert<5, uint32_t, int32_t> array;

    uint32_t a = 10;
    uint32_t b = 20;
    uint32_t c = 30;
    uint32_t d = 40;

    array.Add(&a, 10);
    array.Add(&b, 20);
    array.Add(&c, 30);
    array.Add(&d, 40);

    array.Remove(1);

    EXPECT_EQ(array.Count(), 3u);
    EXPECT_EQ(*array.Get(0), 40u);
    EXPECT_EQ(*array.Get(1), 20u);
    EXPECT_EQ(*array.Get(2), 10u);
    EXPECT_EQ(array.GetSortValue(0), 40);
    EXPECT_EQ(array.GetSortValue(1), 20);
    EXPECT_EQ(array.GetSortValue(2), 10);
}

TEST(ZStackArrayInsert, InvalidOperationsAssert)
{
    ZStackArrayInsert<1, uint32_t, int32_t> array;
    uint32_t value = 10;

    EXPECT_THROW(array.GetSortValue(0), std::runtime_error);
    EXPECT_THROW(array.Remove(0), std::runtime_error);

    array.Add(&value, 1);

    EXPECT_THROW(array.Add(&value, 2), std::runtime_error);
    EXPECT_THROW(array.GetSortValue(1), std::runtime_error);
}

TEST(ZSafeStackArray, PopSafeKeepsLastElement)
{
    ZSafeStackArray<3, uint32_t> array;

    array.Push(10);

    EXPECT_EQ(array.PopSafe(), 10u);
    EXPECT_EQ(array.Count(), 1u);
    EXPECT_EQ(*array.Peek(), 10u);
}

TEST(ZSafeStackArray, PopSafePopsWhenMoreThanOneElement)
{
    ZSafeStackArray<3, uint32_t> array;

    array.Push(10);
    array.Push(20);
    array.Push(30);

    EXPECT_EQ(array.PopSafe(), 30u);
    EXPECT_EQ(array.Count(), 2u);
    EXPECT_EQ(array.PopSafe(), 20u);
    EXPECT_EQ(array.Count(), 1u);
    EXPECT_EQ(array.PopSafe(), 10u);
    EXPECT_EQ(array.Count(), 1u);
}
