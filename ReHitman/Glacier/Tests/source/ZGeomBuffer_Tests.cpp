#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstring>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZGeomBuffer) == 0x3C);
    static_assert(sizeof(SPackedGeomsHeader) == 0x48);
    static_assert(sizeof(SGeomResourcesHeader) == 0xC);
    static_assert(sizeof(SGeomResources) == 0x8);

}

TEST(ZGeomBuffer, ConstructorInitializesCoreBuffers)
{
    ZGeomBuffer buffer(sizeof(ZBaseGeom) * 4, 256, 256, 128);

    EXPECT_EQ(buffer.m_MaxNumberOfBaseGeoms, 4u);
    EXPECT_EQ(buffer.m_lMaxTotalNrBaseGeoms, 4u);
    ASSERT_NE(buffer.m_BaseGeomMemoryManager, nullptr);
    ASSERT_NE(buffer.m_pExtraGeomElems, nullptr);
    ASSERT_NE(buffer.m_pBaseGeomLists, nullptr);
    ASSERT_NE(buffer.m_pEventBuffer, nullptr);
    ASSERT_NE(buffer.m_pRoomListsBuffer, nullptr);
    ASSERT_NE(buffer.m_pFreeRoomLists, nullptr);
    EXPECT_FALSE(buffer.GeomCreationLocked());
}

TEST(ZGeomBuffer, AllocAndFreeRoomListUseFreeListIndices)
{
    ZGeomBuffer buffer(sizeof(ZBaseGeom) * 2, 128, 128, 128);

    ZBaseGeomRoomList* first = buffer.AllocRoomList();
    ZBaseGeomRoomList* second = buffer.AllocRoomList();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(buffer.GetRoomListNr(first), 1u);
    EXPECT_EQ(buffer.GetRoomListNr(second), 2u);
    EXPECT_EQ(buffer.GetRoomList(1), first);
    EXPECT_EQ(buffer.GetRoomList(2), second);
    EXPECT_EQ(buffer.m_lNrRoomLists, 2u);

    buffer.FreeRoomList(second);
    EXPECT_EQ(buffer.m_lNrRoomLists, 1u);

    ZBaseGeomRoomList* reused = buffer.AllocRoomList();
    EXPECT_EQ(reused, second);
}

TEST(ZGeomBuffer, BaseGeomRefsRoundTripAndFree)
{
    ZGeomBuffer buffer(sizeof(ZBaseGeom) * 2, 128, 128, 128);

    ZBaseGeom* base = buffer.AllocBaseGeom();
    ASSERT_NE(base, nullptr);

    ZREF ref = buffer.GeomPtrToRef(base);
    EXPECT_NE(ref, 0u);
    EXPECT_EQ(buffer.GeomRefToBasePtr(ref), base);
    EXPECT_EQ(buffer.BaseGeomBufferPtr(), buffer.m_BaseGeomMemoryManager->GetBufferPtr());

    buffer.FreeBaseGeom(base);
    EXPECT_EQ(buffer.m_BaseGeomMemoryManager->Count(), 0u);
}

TEST(ZGeomBuffer, AllocExtraGeomUsesExtraGeomBufferAndZeroesMemory)
{
    ZGeomBuffer buffer(sizeof(ZBaseGeom) * 2, 128, 128, 128);

    uint32_t freeBefore = buffer.m_pExtraGeomElems->m_pOffsetAlloc->GetFreeTotal();
    ZGEOM* geom = buffer.AllocExtraGeom(sizeof(ZGEOM));

    ASSERT_NE(geom, nullptr);
    EXPECT_EQ(geom->m_baseGeom, nullptr);
    EXPECT_LT(buffer.m_pExtraGeomElems->m_pOffsetAlloc->GetFreeTotal(), freeBefore);
}

TEST(ZGeomBuffer, InitResourceGeomsCopiesPackedArrays)
{
    struct PackedData
    {
        SPackedGeomsHeader Header{};
        uint32_t HeaderCount{};
        SGeomResourcesHeader Headers[2]{};
        uint32_t ResourceCount{};
        SGeomResources Resources[2]{};
    } packed;

    packed.Header.m_lGeomResourcesHeadersOffset = offsetof(PackedData, HeaderCount);
    packed.Header.m_lGeomResourcesOffset = offsetof(PackedData, ResourceCount);
    packed.HeaderCount = 2;
    packed.Headers[0] = { 1, 10, 2 };
    packed.Headers[1] = { 2, 20, 3 };
    packed.ResourceCount = 2;
    packed.Resources[0] = { 7, 11 };
    packed.Resources[1] = { 8, 12 };

    ZGeomBuffer buffer(sizeof(ZBaseGeom) * 2, 128, 128, 128);

    buffer.InitResourceGeoms(&packed.Header);

    ASSERT_NE(buffer.m_pGeomResourcesHeaders, nullptr);
    ASSERT_NE(buffer.m_pGeomResources, nullptr);
    EXPECT_EQ(buffer.m_lNrGeomResourcesHeaders, 2u);
    EXPECT_EQ(buffer.m_lNrResources, 2u);
    EXPECT_EQ(buffer.m_pGeomResourcesHeaders[1].m_lGeomResourcesStart, 20u);
    EXPECT_EQ(buffer.m_pGeomResources[1].m_lGeomType, 8u);
}

TEST(ZGeomBuffer, LockAndUnlockGeomCreation)
{
    ZGeomBuffer buffer(sizeof(ZBaseGeom) * 2, 128, 128, 128);

    buffer.LockGeomCreation();
    EXPECT_TRUE(buffer.GeomCreationLocked());

    buffer.UnlockGeomCreation();
    EXPECT_FALSE(buffer.GeomCreationLocked());
}
