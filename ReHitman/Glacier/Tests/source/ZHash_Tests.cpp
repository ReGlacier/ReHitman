#include <Glacier/ZSTL/ZHash.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <string>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZHashBase) == 0x18);
    static_assert(sizeof(ZIntHash) == 0x20);

    struct CollidingIntHash : ZHash<int, unsigned int>
    {
        using ZHash<int, unsigned int>::ZHash;

        void IntValue(const int*, unsigned int* pSeed1, unsigned int* pSeed2) override
        {
            *pSeed1 = 0;
            *pSeed2 = 0;
        }
    };
}

TEST(ZHash, ConstructorClampsMinimumSizeAndAllocatesStorage)
{
    ZIntHash hash(1);

    EXPECT_EQ(hash.m_lMinSize, 32u);
    EXPECT_EQ(hash.m_lSize, 32u);
    EXPECT_EQ(hash.Count(), 0u);
    EXPECT_NE(hash.m_pArray, nullptr);
    EXPECT_NE(hash.m_aTaken, nullptr);
    EXPECT_EQ(hash.NodeSize(), sizeof(_SHashNode<int, unsigned int>));
    EXPECT_EQ(hash.GetArray(), hash.m_pArray);
}

TEST(ZHash, ConstructorRoundsMinimumSizeUpToPowerOfTwo)
{
    ZIntHash hash(33);

    EXPECT_EQ(hash.m_lMinSize, 64u);
    EXPECT_EQ(hash.m_lSize, 64u);
}

TEST(ZHash, PutGetAndFindStoreValuesByKey)
{
    ZIntHash hash(32);
    int key = 42;

    EXPECT_TRUE(hash.Put(&key, 100u));

    auto* value = hash.Get(&key);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 100u);

    auto* node = hash.Find(&key);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->m_Key, key);
    EXPECT_EQ(node->m_Value, 100u);
    EXPECT_EQ(hash.Count(), 1u);
}

TEST(ZHash, ReferenceOverloadsWorkForCommonOperations)
{
    ZIntHash hash(32);
    int key = 14;

    EXPECT_TRUE(hash.Put(key, 140u));
    ASSERT_NE(hash.Get(key), nullptr);
    EXPECT_EQ(*hash.Get(key), 140u);
    ASSERT_NE(hash.Find(key), nullptr);
    EXPECT_TRUE(hash.Contains(key));

    hash.Remove(key);

    EXPECT_FALSE(hash.Contains(key));
    EXPECT_EQ(hash.Get(key), nullptr);
}

TEST(ZHash, ConstLookupDoesNotMutateLoadStatistic)
{
    ZIntHash hash(32);
    int key = 22;
    hash.Put(key, 220u);
    hash.m_fLoad = 12.5f;

    const ZIntHash& constHash = hash;
    auto* value = constHash.Get(key);

    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 220u);
    EXPECT_TRUE(constHash.Contains(key));
    EXPECT_FLOAT_EQ(hash.m_fLoad, 12.5f);
}

TEST(ZHash, PutUpdatesExistingKeyWithoutIncreasingCount)
{
    ZIntHash hash(32);
    int key = 7;

    EXPECT_TRUE(hash.Put(&key, 10u));
    EXPECT_TRUE(hash.Put(&key, 20u));

    ASSERT_NE(hash.Get(&key), nullptr);
    EXPECT_EQ(*hash.Get(&key), 20u);
    EXPECT_EQ(hash.Count(), 1u);
}

TEST(ZHash, RemoveClearsEntry)
{
    ZIntHash hash(32);
    int key = 9;

    hash.Put(&key, 90u);
    ASSERT_NE(hash.Get(&key), nullptr);

    hash.Remove(&key);

    EXPECT_EQ(hash.Get(&key), nullptr);
    EXPECT_EQ(hash.Count(), 0u);
    EXPECT_EQ(hash.m_lLongestSequence, 0u);
}

TEST(ZHash, FindContinuesPastRemovedCollisionSlot)
{
    CollidingIntHash hash(32);
    int first = 1;
    int second = 2;
    int third = 3;

    ASSERT_TRUE(hash.Put(&first, 10u));
    ASSERT_TRUE(hash.Put(&second, 20u));
    ASSERT_TRUE(hash.Put(&third, 30u));
    ASSERT_GE(hash.m_lLongestSequence, 2u);

    hash.Remove(&second);

    EXPECT_EQ(hash.Get(&second), nullptr);
    ASSERT_NE(hash.Get(&third), nullptr);
    EXPECT_EQ(*hash.Get(&third), 30u);
}

TEST(ZHash, PutWithoutGrowthFailsWhenProbeSequenceExhaustsTable)
{
    CollidingIntHash hash(32);
    int keys[33]{};

    for (int i = 0; i < 32; ++i)
    {
        keys[i] = i + 1;
        ASSERT_TRUE(hash.Put(&keys[i], static_cast<unsigned int>(i), false));
    }

    keys[32] = 33;
    EXPECT_FALSE(hash.Put(&keys[32], 32u, false));
    EXPECT_EQ(hash.Count(), 32u);
    EXPECT_EQ(hash.m_lSize, 32u);
}

TEST(ZHash, IterationReturnsAllOccupiedEntries)
{
    ZIntHash hash(32);
    int keys[] = { 1, 2, 3 };

    hash.Put(&keys[0], 10u);
    hash.Put(&keys[1], 20u);
    hash.Put(&keys[2], 30u);

    int* pKey = nullptr;
    unsigned int* pValue = nullptr;
    uintptr_t next = hash.First(&pKey, &pValue);
    uint32_t seenMask = 0;

    while (next)
    {
        ASSERT_NE(pKey, nullptr);
        ASSERT_NE(pValue, nullptr);
        seenMask |= 1u << (*pKey - 1);
        EXPECT_EQ(*pValue, static_cast<unsigned int>(*pKey * 10));
        next = hash.Next(static_cast<uint32_t>(next), &pKey, &pValue);
    }

    EXPECT_EQ(seenMask, 0b111u);
}

TEST(ZHash, LowLevelNextAdvancesIndexPastReturnedNode)
{
    ZIntHash hash(32);
    int key = 11;
    hash.Put(&key, 110u);

    uint32_t index = 0;
    auto* node = hash.Next(&index);

    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->m_Key, key);
    EXPECT_EQ(node->m_Value, 110u);
    EXPECT_GT(index, 0u);
    EXPECT_EQ(hash.Next(&index), nullptr);
}

TEST(ZHash, ConstIterationReturnsReadOnlyEntries)
{
    ZIntHash hash(32);
    int key = 12;
    hash.Put(key, 120u);

    const ZIntHash& constHash = hash;
    const int* pKey = nullptr;
    const unsigned int* pValue = nullptr;

    uintptr_t next = constHash.First(&pKey, &pValue);

    ASSERT_NE(next, 0u);
    ASSERT_NE(pKey, nullptr);
    ASSERT_NE(pValue, nullptr);
    EXPECT_EQ(*pKey, key);
    EXPECT_EQ(*pValue, 120u);
    EXPECT_EQ(constHash.Next(static_cast<uint32_t>(next), &pKey, &pValue), 0u);
}

TEST(ZHash, GrowDoublesTableAndPreservesEntries)
{
    ZIntHash hash(32);
    int keys[32]{};

    for (int i = 0; i < 32; ++i)
    {
        keys[i] = i + 1;
        ASSERT_TRUE(hash.Put(&keys[i], static_cast<unsigned int>(keys[i] * 100)));
    }

    EXPECT_EQ(hash.m_lSize, 32u);

    int extraKey = 100;
    ASSERT_TRUE(hash.Put(&extraKey, 1234u));

    EXPECT_EQ(hash.m_lSize, 64u);
    EXPECT_EQ(hash.Count(), 33u);
    EXPECT_EQ(*hash.Get(&keys[0]), 100u);
    EXPECT_EQ(*hash.Get(&extraKey), 1234u);
}

TEST(ZHash, ClearResetsCountAndMinimumStorage)
{
    ZIntHash hash(64);
    int key = 5;

    hash.Put(&key, 50u);
    hash.Grow();
    ASSERT_EQ(hash.m_lSize, 128u);

    hash.Clear();

    EXPECT_EQ(hash.Count(), 0u);
    EXPECT_EQ(hash.m_lSize, hash.m_lMinSize);
    EXPECT_EQ(hash.Get(&key), nullptr);
}

TEST(ZHash, SetSizeDoesNotShrinkBelowElementCount)
{
    ZIntHash hash(32);
    int keys[4]{};

    for (int i = 0; i < 4; ++i)
    {
        keys[i] = i + 1;
        hash.Put(&keys[i], static_cast<unsigned int>(i + 10));
    }

    hash.SetSize(2);

    EXPECT_EQ(hash.m_lSize, 32u);
    EXPECT_EQ(hash.Count(), 4u);
}

TEST(ZHash, RemoveShrinksTableWhenSparseButNotBelowMinimum)
{
    ZIntHash hash(32);
    int keys[33]{};

    for (int i = 0; i < 33; ++i)
    {
        keys[i] = i + 1;
        hash.Put(&keys[i], static_cast<unsigned int>(i));
    }

    ASSERT_EQ(hash.m_lSize, 64u);

    for (int i = 0; i < 18; ++i)
        hash.Remove(&keys[i]);

    EXPECT_EQ(hash.m_lSize, 32u);
    EXPECT_EQ(hash.m_lMinSize, 32u);
}

TEST(ZHash, TakenBitsReflectInsertedAndRemovedSlots)
{
    ZIntHash hash(32);
    int key = 123;

    hash.Put(&key, 456u);
    auto* node = hash.Find(&key);
    ASSERT_NE(node, nullptr);

    uint32_t index = static_cast<uint32_t>(node - hash.m_pArray);
    EXPECT_TRUE(hash.IsTaken(index));

    hash.Remove(&key);
    EXPECT_FALSE(hash.IsTaken(index));
}

TEST(ZPStrHash, UsesStringContentsForHashAndEquality)
{
    ZPStrHash<unsigned int> hash(32);
    const char keyStorage[] = "door";
    const char* key = keyStorage;
    std::string sameText = "door";
    const char* equivalentKey = sameText.c_str();

    ASSERT_NE(key, equivalentKey);
    EXPECT_TRUE(hash.Put(&key, 77u));

    auto* value = hash.Get(&equivalentKey);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 77u);
}

TEST(ZPStrHash, PutUpdatesExistingStringWithDifferentPointer)
{
    ZPStrHash<unsigned int> hash(32);
    const char firstStorage[] = "same";
    std::string secondStorage = "same";
    const char* first = firstStorage;
    const char* second = secondStorage.c_str();

    ASSERT_NE(first, second);
    EXPECT_TRUE(hash.Put(&first, 1u));
    EXPECT_TRUE(hash.Put(&second, 2u));

    EXPECT_EQ(hash.Count(), 1u);
    ASSERT_NE(hash.Get(&first), nullptr);
    EXPECT_EQ(*hash.Get(&first), 2u);
}

TEST(ZPStrHash, SupportsNonUnsignedValueType)
{
    ZPStrHash<int> hash(32);
    const char* key = "signed-value";

    EXPECT_TRUE(hash.Put(&key, -42));

    auto* value = hash.Get(&key);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, -42);
}
