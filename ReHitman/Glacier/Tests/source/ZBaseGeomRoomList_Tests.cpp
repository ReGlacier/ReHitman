#include <Glacier/Geom/ZBaseGeomRoomList.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZBaseGeomRoomList) == 0x1C);

    ZROOM* Room(uintptr_t value)
    {
        return reinterpret_cast<ZROOM*>(value);
    }
}

TEST(ZBaseGeomRoomList, InitAndClearResetCount)
{
    ZBaseGeomRoomList list{};
    list.m_cNrRooms = 3;

    list.Init();
    EXPECT_EQ(list.Count(), 0u);

    ASSERT_TRUE(list.Add(Room(1)));
    ASSERT_TRUE(list.Add(Room(2)));
    EXPECT_EQ(list.Count(), 2u);

    list.Clear();
    EXPECT_EQ(list.Count(), 0u);
}

TEST(ZBaseGeomRoomList, AddStoresUpToSixRooms)
{
    ZBaseGeomRoomList list{};
    list.Init();

    for (uintptr_t i = 0; i < 6; ++i)
    {
        EXPECT_TRUE(list.Add(Room(i + 1)));
    }

    EXPECT_FALSE(list.Add(Room(7)));
    EXPECT_EQ(list.Count(), 6u);

    for (uint32_t i = 0; i < 6; ++i)
    {
        EXPECT_EQ(list.GetRoomNr(i), Room(i + 1));
    }
}

TEST(ZBaseGeomRoomList, ExistsFindsOnlyStoredRooms)
{
    ZBaseGeomRoomList list{};
    list.Init();

    ZROOM* first = Room(0x1000);
    ZROOM* second = Room(0x2000);
    ZROOM* missing = Room(0x3000);

    list.Add(first);
    list.Add(second);

    EXPECT_TRUE(list.Exists(first));
    EXPECT_TRUE(list.Exists(second));
    EXPECT_FALSE(list.Exists(missing));
}

TEST(ZBaseGeomRoomList, RemoveSwapsLastRoomIntoRemovedSlot)
{
    ZBaseGeomRoomList list{};
    list.Init();

    ZROOM* first = Room(1);
    ZROOM* second = Room(2);
    ZROOM* third = Room(3);

    list.Add(first);
    list.Add(second);
    list.Add(third);

    list.Remove(second);

    EXPECT_EQ(list.Count(), 2u);
    EXPECT_EQ(list.GetRoomNr(0), first);
    EXPECT_EQ(list.GetRoomNr(1), third);
    EXPECT_FALSE(list.Exists(second));
}

TEST(ZBaseGeomRoomList, RemoveMissingRoomDoesNothing)
{
    ZBaseGeomRoomList list{};
    list.Init();

    ZROOM* first = Room(1);
    ZROOM* second = Room(2);

    list.Add(first);

    list.Remove(second);

    EXPECT_EQ(list.Count(), 1u);
    EXPECT_EQ(list.GetRoomNr(0), first);
}

TEST(ZBaseGeomRoomList, GetRoomListReturnsRoomsArray)
{
    ZBaseGeomRoomList list{};
    list.Init();

    ZROOM* first = Room(1);
    list.Add(first);

    ZROOM** rooms = list.GetRoomList();

    ASSERT_EQ(rooms, list.m_pRooms);
    EXPECT_EQ(rooms[0], first);
}

TEST(ZBaseGeomRoomList, GetRoomNrAssertsWhenIndexIsOutOfRange)
{
    ZBaseGeomRoomList list{};
    list.Init();

    EXPECT_THROW(list.GetRoomNr(0), std::runtime_error);
}
