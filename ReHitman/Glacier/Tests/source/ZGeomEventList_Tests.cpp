#include <Glacier/Geom/ZGeomEventList.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    static_assert(sizeof(SGeomEvent_Direct_Id) == 0x3);
    static_assert(sizeof(ZGeomEventListBuffers::SGeomEventListBufferEntity_Id) == 0x2);
    static_assert(sizeof(ZGeomEventListBuffers::SGeomEventListBufferEntity) == 0x5);
    static_assert(sizeof(ZGeomEventListBuffers::ZGeomEventListBuffer) == 0x4FE);
    static_assert(sizeof(ZGeomEventListBuffers) == 0x504);
    static_assert(sizeof(ZGeomEventList) == 0x4);
}

TEST(SGeomEventDirectId, StoresLowerTwentyFourBitsLittleEndian)
{
    SGeomEvent_Direct_Id id{};

    id.SetVal(0x00ABCDEF);

    EXPECT_EQ(id.m_iDirect_Value[0], 0xEF);
    EXPECT_EQ(id.m_iDirect_Value[1], 0xCD);
    EXPECT_EQ(id.m_iDirect_Value[2], 0xAB);
    EXPECT_EQ(id.GetVal(), 0x00ABCDEFu);
}

TEST(ZGeomEventListBuffers, AllocValueCreatesSingleElementList)
{
    ZGeomEventListBuffers buffers;

    auto first = buffers.Alloc_Value(0x12345);

    EXPECT_NE(first.m_iEntity_Id, 0xFF);
    EXPECT_EQ(buffers.Get_List_Size(first), 1u);
    EXPECT_EQ(buffers.Get_Value(first), 0x12345u);
    EXPECT_TRUE(buffers.Exists(first, 0x12345));
    EXPECT_FALSE(buffers.Exists(first, 0x54321));
}

TEST(ZGeomEventListBuffers, LinkEntityAtEndPreservesOrder)
{
    ZGeomEventListBuffers buffers;

    auto first = buffers.Alloc_Value(10);
    buffers.Alloc_Link_Value(first, 20);
    buffers.Alloc_Link_Value(first, 30);

    EXPECT_EQ(buffers.Get_List_Size(first), 3u);
    EXPECT_EQ(buffers.Get_Value_Nr(first, 0), 10u);
    EXPECT_EQ(buffers.Get_Value_Nr(first, 1), 20u);
    EXPECT_EQ(buffers.Get_Value_Nr(first, 2), 30u);
    EXPECT_EQ(buffers.Get_Value_Nr(first, 3), 0u);
}

TEST(ZGeomEventListBuffers, FreeValueUpdatesHeadAndLinks)
{
    ZGeomEventListBuffers buffers;

    auto first = buffers.Alloc_Value(10);
    buffers.Alloc_Link_Value(first, 20);
    buffers.Alloc_Link_Value(first, 30);

    first = buffers.Free_Value(first, 10);

    EXPECT_EQ(buffers.Get_List_Size(first), 2u);
    EXPECT_EQ(buffers.Get_Value_Nr(first, 0), 20u);
    EXPECT_EQ(buffers.Get_Value_Nr(first, 1), 30u);

    auto oldFirst = first;
    auto returnedFirst = buffers.Free_Value(first, 30);

    EXPECT_EQ(returnedFirst.m_iEntity_Id, 0xFF);
    first = oldFirst;

    EXPECT_EQ(buffers.Get_List_Size(first), 1u);
    EXPECT_EQ(buffers.Get_Value_Nr(first, 0), 20u);
}

TEST(ZGeomEventList, AddRemoveAndCountTrackStateTransitions)
{
    ZGeomEventList::Create_Geom_Event_Buffer_Lists();
    ZGeomEventList list;

    EXPECT_EQ(list.Count(), 0u);
    EXPECT_FALSE(list.ChkEvents());

    list.Add(10);
    EXPECT_EQ(list.Count(), 1u);
    EXPECT_TRUE(list.Exists(10));
    EXPECT_EQ(list.GetValueNr(0), 10u);

    list.Add(20);
    list.Add(30);
    EXPECT_EQ(list.Count(), 3u);
    EXPECT_TRUE(list.Exists(20));
    EXPECT_TRUE(list.Exists(30));
    EXPECT_EQ(list.GetValueNr(0), 10u);
    EXPECT_EQ(list.GetValueNr(1), 20u);
    EXPECT_EQ(list.GetValueNr(2), 30u);

    list.Remove(20);
    EXPECT_EQ(list.Count(), 2u);
    EXPECT_FALSE(list.Exists(20));

    list.Remove(10);
    EXPECT_EQ(list.Count(), 1u);
    EXPECT_EQ(list.GetValueNr(0), 30u);

    list.Remove(30);
    EXPECT_EQ(list.Count(), 0u);
    EXPECT_FALSE(list.ChkEvents());

    ZGeomEventList::Destroy_Geom_Event_Buffer_Lists();
}

TEST(ZGeomEventList, ValueRunIteratesAndRunRemoveDeletesCurrentValue)
{
    ZGeomEventList::Create_Geom_Event_Buffer_Lists();
    ZGeomEventList list;
    list.Add(100);
    list.Add(200);

    ZGeomEventListBuffers::ValueRun run{};
    list.InitValueRun(run);

    ASSERT_FALSE(run.m_bFin);
    EXPECT_EQ(list.GetValueFromValueRun(run), 100u);

    list.NextValueRun(run);
    ASSERT_FALSE(run.m_bFin);
    EXPECT_EQ(list.GetValueFromValueRun(run), 200u);

    list.RunRemoveEvent(run);
    EXPECT_TRUE(list.Exists(100));
    EXPECT_FALSE(list.Exists(200));
    EXPECT_EQ(list.Count(), 1u);

    ZGeomEventList::Destroy_Geom_Event_Buffer_Lists();
}
