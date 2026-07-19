#include <Glacier/Geom/ZBaseGeomLists.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(SBaseGeomListHeader) == 0xC);
    static_assert(sizeof(ZBaseGeomLists) == 0x18);

    constexpr uintptr_t QLISTGEOMTYPE = 7;

    uintptr_t Pack(ZBaseGeom* pGeom, uint32_t lType)
    {
        return reinterpret_cast<uintptr_t>(pGeom) | lType;
    }

    SBaseGeomListHeader* Header(uintptr_t pList)
    {
        return reinterpret_cast<SBaseGeomListHeader*>(pList);
    }

    uintptr_t Entry(SBaseGeomListHeader* pHeader, uint8_t index)
    {
        uintptr_t value = 0;
        std::memcpy(&value, reinterpret_cast<char*>(pHeader + 1) + sizeof(uintptr_t) * index, sizeof(value));
        return value;
    }
}

TEST(ZBaseGeomLists, AddBaseGeomsCreatesListNode)
{
    ZBaseGeomLists lists(256, nullptr);
    alignas(8) ZBaseGeom geoms[2]{};
    ZBaseGeom parent{};
    geoms[0].m_pParent = &parent;
    geoms[1].m_pParent = &parent;

    uintptr_t pList = lists.AddBaseGeoms(0, &geoms[0], &geoms[1], 2, 4);

    ASSERT_NE(pList, 0u);
    auto* header = Header(pList);
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(header->m_cUsedLen, 2u);
    EXPECT_EQ(header->m_cPoolLen, 8u);
    EXPECT_EQ(header->m_cTypeLens[2], 1u);
    EXPECT_EQ(Entry(header, 0), Pack(&geoms[0], 2));
    EXPECT_EQ(Entry(header, 1), Pack(&geoms[1], 2));
    EXPECT_TRUE(lists.Exists(pList, &geoms[0], 2));
    EXPECT_TRUE(lists.Exists(pList, &geoms[1], 2));
}

TEST(ZBaseGeomLists, AddBaseGeomsMergesAdjacentRangesWithSameParentAndType)
{
    ZBaseGeomLists lists(256, nullptr);
    alignas(8) ZBaseGeom geoms[4]{};
    ZBaseGeom parent{};
    for (auto& geom : geoms)
        geom.m_pParent = &parent;

    uintptr_t pList = lists.AddBaseGeoms(0, &geoms[0], &geoms[1], 3, 4);
    uintptr_t pMergedList = lists.AddBaseGeoms(pList, &geoms[2], &geoms[3], 3, 4);

    EXPECT_EQ(pMergedList, pList);

    auto* header = Header(pList);
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(header->m_cUsedLen, 2u);
    EXPECT_EQ(header->m_cTypeLens[3], 1u);
    EXPECT_EQ(Entry(header, 0), Pack(&geoms[0], 3));
    EXPECT_EQ(Entry(header, 1), Pack(&geoms[3], 3));
}

TEST(ZBaseGeomLists, AddBaseGeomsDoesNotMergeDifferentParents)
{
    ZBaseGeomLists lists(256, nullptr);
    alignas(8) ZBaseGeom geoms[4]{};
    ZBaseGeom firstParent{};
    ZBaseGeom secondParent{};
    geoms[0].m_pParent = &firstParent;
    geoms[1].m_pParent = &firstParent;
    geoms[2].m_pParent = &secondParent;
    geoms[3].m_pParent = &secondParent;

    uintptr_t pList = lists.AddBaseGeoms(0, &geoms[0], &geoms[1], 1, 4);
    pList = lists.AddBaseGeoms(pList, &geoms[2], &geoms[3], 1, 4);

    auto* header = Header(pList);
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(header->m_cUsedLen, 4u);
    EXPECT_EQ(header->m_cTypeLens[1], 2u);
}

TEST(ZBaseGeomLists, RemoveBaseGeomsRemovesWholeRangeAndFreesEmptyList)
{
    ZBaseGeomLists lists(256, nullptr);
    alignas(8) ZBaseGeom geoms[2]{};

    uintptr_t pList = lists.AddBaseGeoms(0, &geoms[0], &geoms[1], 4, 4);
    ASSERT_NE(pList, 0u);
    ASSERT_NE(lists.m_pOffsetAlloc, nullptr);
    EXPECT_LT(lists.m_pOffsetAlloc->GetFreeTotal(), 256);

    uintptr_t pResult = lists.RemoveBaseGeoms(pList, &geoms[0], &geoms[1], 4);

    EXPECT_EQ(pResult, 0u);
    EXPECT_EQ(lists.m_pOffsetAlloc->GetFreeTotal(), 256);
}

TEST(ZBaseGeomLists, RemoveBaseGeomsShrinksRangeFromStartAndEnd)
{
    ZBaseGeomLists lists(256, nullptr);
    alignas(8) ZBaseGeom geoms[4]{};

    uintptr_t pList = lists.AddBaseGeoms(0, &geoms[0], &geoms[3], 5, 4);

    pList = lists.RemoveBaseGeoms(pList, &geoms[0], &geoms[0], 5);
    auto* header = Header(pList);
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(Entry(header, 0), Pack(&geoms[1], 5));
    EXPECT_EQ(Entry(header, 1), Pack(&geoms[3], 5));

    pList = lists.RemoveBaseGeoms(pList, &geoms[3], &geoms[3], 5);
    header = Header(pList);
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(Entry(header, 0), Pack(&geoms[1], 5));
    EXPECT_EQ(Entry(header, 1), Pack(&geoms[2], 5));
}

TEST(ZBaseGeomLists, RemoveBaseGeomsSplitsRange)
{
    ZBaseGeomLists lists(256, nullptr);
    alignas(8) ZBaseGeom geoms[4]{};

    uintptr_t pList = lists.AddBaseGeoms(0, &geoms[0], &geoms[3], 6, 4);
    pList = lists.RemoveBaseGeoms(pList, &geoms[1], &geoms[1], 6);

    auto* header = Header(pList);
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(header->m_cUsedLen, 4u);
    EXPECT_EQ(header->m_cTypeLens[6], 2u);
    EXPECT_EQ(Entry(header, 0), Pack(&geoms[0], 6));
    EXPECT_EQ(Entry(header, 1), Pack(&geoms[0], 6));
    EXPECT_EQ(Entry(header, 2), Pack(&geoms[2], 6));
    EXPECT_EQ(Entry(header, 3), Pack(&geoms[3], 6));
}

TEST(ZBaseGeomLists, InvalidPoolSizeAsserts)
{
    ZBaseGeomLists lists(256, nullptr);
    alignas(8) ZBaseGeom geom{};

    EXPECT_THROW(lists.AddBaseGeoms(0, &geom, &geom, 0, 0), std::runtime_error);
    EXPECT_THROW(lists.AddBaseGeoms(0, &geom, &geom, 0, 256), std::runtime_error);
    EXPECT_THROW(lists.AddBaseGeoms(0, &geom, &geom, QLISTGEOMTYPE + 1, 4), std::runtime_error);
}
