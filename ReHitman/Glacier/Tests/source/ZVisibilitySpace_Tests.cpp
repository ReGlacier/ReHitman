#include <Glacier/GameBase/ZVisibilitySpace.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    void SetupSpace(ZVisibilitySpace& space)
    {
        space.m_vCellSize = ZVector3(10.0f, 40.0f, 20.0f);
        space.m_vCellSizeInv = ZVector3(0.1f, 0.025f, 0.05f);
        space.m_vLow = ZVector3(-50.0f, -60.0f, -70.0f);
        space.m_vHigh = ZVector3(110.0f, -20.0f, 250.0f);
    }
}

TEST(ZVisibilitySpace, ConvertsBetweenPositionsAndCells)
{
    ZVisibilitySpace space;
    SetupSpace(space);

    const float position[3] = { 5.0f, -40.0f, 100.0f };
    const ZCellCoordinate cell = space.FindCell(position);
    EXPECT_EQ(cell, ZCellCoordinate(5, 0, 8));

    float cellOrigin[3] = {};
    space.GetCellPosition(cellOrigin, cell);
    EXPECT_FLOAT_EQ(cellOrigin[0], 0.0f);
    EXPECT_FLOAT_EQ(cellOrigin[1], -60.0f);
    EXPECT_FLOAT_EQ(cellOrigin[2], 90.0f);
}

TEST(ZVisibilitySpace, ManagesSeeableLifecycle)
{
    ZVisibilitySpace space;
    SetupSpace(space);

    float start[3] = { 0.0f, -55.0f, 50.0f };
    const Locator original = space.AddSeeable(start, 42);
    ASSERT_TRUE(original.IsValid());
    EXPECT_EQ(space.GetCell(original).GetSeeableID(0), 42);

    float moved[3] = { 10.0f, -55.0f, 50.0f };
    const Locator updated = space.UpdateSeeable(42, original, moved);
    ASSERT_TRUE(updated.IsValid());
    EXPECT_EQ(space.GetCell(original).GetNumSeeables(), 0);
    EXPECT_EQ(space.GetCell(updated).GetSeeableID(0), 42);

    float outside[3] = { 500.0f, -55.0f, 50.0f };
    const Locator removed = space.UpdateSeeable(42, updated, outside);
    EXPECT_FALSE(removed.IsValid());
    EXPECT_EQ(space.GetCell(updated).GetNumSeeables(), 0);
}
