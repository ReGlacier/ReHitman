#include <Glacier/GlacierFWD.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZSTL/ZFixedArray.h>
#include <gtest/gtest.h>

#include <cstdint>

using namespace Glacier;

namespace
{
    struct Payload
    {
        uint32_t Id;
        uint32_t Value;
    };

    struct ConstructedPayload
    {
        uint32_t Magic;

        ConstructedPayload() : Magic(0xDEADBEEF) {}
    };

    using UInt32Array4 = ZFixedArray<uint32_t, 4>;
    using PayloadArray3 = ZFixedArray<Payload, 3>;

    static_assert(sizeof(UInt32Array4) == sizeof(uint32_t) * 4 + sizeof(uint16_t) * 2);
    static_assert(offsetof(UInt32Array4, m_Array) == 0);
    static_assert(offsetof(UInt32Array4, m_lCount) == sizeof(uint32_t) * 4);
    static_assert(offsetof(UInt32Array4, m_lFirstFreeEntry) == sizeof(uint32_t) * 4 + sizeof(uint16_t));
    static_assert(sizeof(PayloadArray3) == sizeof(Payload) * 3 + sizeof(uint16_t) * 2);
}

TEST(ZFixedArray, ConstructorResetsToEmptyWithFixedCapacity)
{
    ZFixedArray<Payload, 4> array;

    EXPECT_EQ(array.Count(), 0u);
    EXPECT_EQ(array.TotalNrEntries(), 4u);
    EXPECT_EQ(array.m_lCount, 0u);
    EXPECT_EQ(array.m_lFirstFreeEntry, 0u);
}

TEST(ZFixedArray, AddHandsOutEntriesInOrderAndCountsThem)
{
    ZFixedArray<Payload, 4> array;

    Payload* first = array.Add();
    Payload* second = array.Add();
    Payload* third = array.Add();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(third, nullptr);

    EXPECT_EQ(first, array.GetEntry(0));
    EXPECT_EQ(second, array.GetEntry(1));
    EXPECT_EQ(third, array.GetEntry(2));
    EXPECT_EQ(array.Count(), 3u);
}

TEST(ZFixedArray, AddDefaultConstructsEntryInPlace)
{
    ZFixedArray<ConstructedPayload, 2> array;

    ConstructedPayload* entry = array.Add();

    ASSERT_NE(entry, nullptr);
    EXPECT_EQ(entry->Magic, 0xDEADBEEFu);
}

TEST(ZFixedArray, AddReturnsNullptrWhenFull)
{
    ZFixedArray<Payload, 2> array;

    ASSERT_NE(array.Add(), nullptr);
    ASSERT_NE(array.Add(), nullptr);

    EXPECT_EQ(array.Add(), nullptr);
    EXPECT_EQ(array.Count(), 2u);
}

TEST(ZFixedArray, RemoveDecrementsCountAndReusesSlotLIFO)
{
    ZFixedArray<Payload, 4> array;

    Payload* first = array.Add();
    Payload* second = array.Add();
    array.Add();

    array.Remove(second);

    EXPECT_EQ(array.Count(), 2u);

    Payload* reused = array.Add();

    EXPECT_EQ(reused, second);
    EXPECT_EQ(array.Count(), 3u);

    Payload* next = array.Add();

    EXPECT_EQ(next, array.GetEntry(3));
    EXPECT_NE(next, first);
}

TEST(ZFixedArray, ValidateDetectsAllocatedAndFreeEntries)
{
    ZFixedArray<Payload, 4> array;

    Payload* first = array.Add();
    Payload* second = array.Add();

    EXPECT_TRUE(array.Validate(first));
    EXPECT_TRUE(array.Validate(second));
    EXPECT_FALSE(array.Validate(array.GetEntry(2)));
    EXPECT_FALSE(array.Validate(array.GetEntry(3)));

    array.Remove(first);

    EXPECT_FALSE(array.Validate(first));
    EXPECT_TRUE(array.Validate(second));
}

TEST(ZFixedArray, GetIndexAndGetEntryRoundTrip)
{
    ZFixedArray<Payload, 4> array;

    Payload* first = array.Add();
    Payload* second = array.Add();

    EXPECT_EQ(array.GetIndex(first), 0u);
    EXPECT_EQ(array.GetIndex(second), 1u);
    EXPECT_EQ(array.GetEntry(array.GetIndex(second)), second);
}

TEST(ZFixedArray, ResetLinksAllEntriesBackToFreeList)
{
    ZFixedArray<Payload, 3> array;

    array.Add();
    array.Add();
    array.Remove(array.GetEntry(0));

    array.Reset();

    EXPECT_EQ(array.Count(), 0u);
    EXPECT_EQ(array.TotalNrEntries(), 3u);

    EXPECT_EQ(array.Add(), array.GetEntry(0));
    EXPECT_EQ(array.Add(), array.GetEntry(1));
    EXPECT_EQ(array.Add(), array.GetEntry(2));
    EXPECT_EQ(array.Add(), nullptr);
    EXPECT_EQ(array.Count(), 3u);
}

TEST(ZFixedArray, StoredValuesSurviveRemoveOfOtherEntries)
{
    ZFixedArray<Payload, 3> array;

    Payload* first = array.Add();
    Payload* second = array.Add();

    first->Id = 1;
    first->Value = 10;
    second->Id = 2;
    second->Value = 20;

    array.Remove(first);

    EXPECT_EQ(second->Id, 2u);
    EXPECT_EQ(second->Value, 20u);
    EXPECT_TRUE(array.Validate(second));
}

TEST(ZFixedArray, InvalidOperationsAssert)
{
    ZFixedArray<Payload, 2> array;

    Payload* entry = array.Add();

    EXPECT_THROW(array.GetEntry(2), std::runtime_error);

    auto* misaligned = reinterpret_cast<Payload*>(reinterpret_cast<char*>(entry) + 1);

    EXPECT_THROW(array.GetIndex(misaligned), std::runtime_error);
    EXPECT_THROW(array.Validate(misaligned), std::runtime_error);
}
