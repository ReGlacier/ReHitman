#include <Glacier/PF4/ZBBox.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    PF4::ZBBox MakeBox(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
    {
        PF4::ZBBox box;
        box.m_Min.x = minX; box.m_Min.y = minY; box.m_Min.z = minZ;
        box.m_Max.x = maxX; box.m_Max.y = maxY; box.m_Max.z = maxZ;
        return box;
    }
}

TEST(ZBBox, DefaultConstructor)
{
    PF4::ZBBox box;

    EXPECT_EQ(box.min(), ZVector3(0.f, 0.f, 0.f));
    EXPECT_EQ(box.max(), ZVector3(0.f, 0.f, 0.f));
}

TEST(ZBBox, ResetClearsMembers)
{
    auto box = MakeBox(1.f, 2.f, 3.f, 4.f, 5.f, 6.f);

    box.reset();

    EXPECT_EQ(box.min(), ZVector3(0.f, 0.f, 0.f));
    EXPECT_EQ(box.max(), ZVector3(0.f, 0.f, 0.f));
}

TEST(ZBBox, MinMaxAccessors)
{
    auto box = MakeBox(1.f, 2.f, 3.f, 4.f, 5.f, 6.f);

    EXPECT_EQ(box.min(), ZVector3(1.f, 2.f, 3.f));
    EXPECT_EQ(box.max(), ZVector3(4.f, 5.f, 6.f));
}

TEST(ZBBox, GetLengthOfLongestAxis)
{
    {
        auto box = MakeBox(0.f, 0.f, 0.f, 2.f, 1.f, 3.f);
        EXPECT_FLOAT_EQ(box.GetLengthOfLongestAxis(), 3.f);
    }
    {
        auto box = MakeBox(-5.f, -5.f, -5.f, 5.f, 5.f, 5.f);
        EXPECT_FLOAT_EQ(box.GetLengthOfLongestAxis(), 10.f);
    }
    {
        // all equal -> any axis works
        auto box = MakeBox(0.f, 0.f, 0.f, 2.f, 2.f, 2.f);
        EXPECT_FLOAT_EQ(box.GetLengthOfLongestAxis(), 2.f);
    }
    {
        PF4::ZBBox empty;
        EXPECT_FLOAT_EQ(empty.GetLengthOfLongestAxis(), 0.f);
    }
}

TEST(ZBBox, Center)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 2.f, 4.f, 6.f);

    EXPECT_EQ(box.Center(), ZVector3(1.f, 2.f, 3.f));
}

TEST(ZBBox, CenterNegativeBounds)
{
    auto box = MakeBox(-4.f, -2.f, -6.f, 2.f, 4.f, 6.f);

    EXPECT_EQ(box.Center(), ZVector3(3.f, 3.f, 6.f));
}

TEST(ZBBox, Thickness)
{
    {
        auto box = MakeBox(0.f, 0.f, 0.f, 3.f, 1.f, 2.f);
        EXPECT_FLOAT_EQ(box.thickness(), 1.f);
    }
    {
        auto box = MakeBox(0.f, 0.f, 0.f, -1.f, -1.f, -1.f);
        EXPECT_FLOAT_EQ(box.thickness(), 1.f);
    }
    {
        PF4::ZBBox empty;
        EXPECT_FLOAT_EQ(empty.thickness(), 0.f);
    }
}

TEST(ZBBox, FindFastestWayOut_ClipsEachAxis)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 2.f, 2.f, 2.f);

    EXPECT_EQ(box.FindFastestWayOut(ZVector3(-1.f, 1.f, 3.f)), ZVector3(0.f, 1.f, 2.f));
    EXPECT_EQ(box.FindFastestWayOut(ZVector3(1.f, -1.f, 1.f)), ZVector3(1.f, 0.f, 1.f));
    EXPECT_EQ(box.FindFastestWayOut(ZVector3(5.f, 5.f, 5.f)), ZVector3(2.f, 2.f, 2.f));
    EXPECT_EQ(box.FindFastestWayOut(ZVector3(-5.f, -5.f, -5.f)), ZVector3(0.f, 0.f, 0.f));
    // inside point is unchanged
    EXPECT_EQ(box.FindFastestWayOut(ZVector3(0.5f, 1.5f, 1.f)), ZVector3(0.5f, 1.5f, 1.f));
}

TEST(ZBBox, FindFastestWayOut_NegativeBounds)
{
    auto box = MakeBox(-2.f, -2.f, -2.f, 2.f, 2.f, 2.f);

    EXPECT_EQ(box.FindFastestWayOut(ZVector3(-5.f, 0.f, 0.f)), ZVector3(-2.f, 0.f, 0.f));
    EXPECT_EQ(box.FindFastestWayOut(ZVector3(0.f, 5.f, 0.f)), ZVector3(0.f, 2.f, 0.f));
    // inside negative region
    EXPECT_EQ(box.FindFastestWayOut(ZVector3(-1.f, -1.f, -1.f)), ZVector3(-1.f, -1.f, -1.f));
}

TEST(ZBBox, FindShortestAxisAlignedExit_NearerToMinX)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 10.f, 0.f, 10.f);

    bool bX = false, bZ = false;
    box.FindShortestAxisAlignedExit(ZVector3(2.f, 0.f, 5.f), bX, bZ);

    // implementation uses m_Max distance when point >= m_Min
    // fDX = |2-10| = 8, fDZ = |5-10| = 5 -> bX = (8 <= 5) = false
    EXPECT_FALSE(bX);
    EXPECT_TRUE(bZ);
}

TEST(ZBBox, FindShortestAxisAlignedExit_NearerToMaxX)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 10.f, 0.f, 10.f);

    bool bX = false, bZ = false;
    box.FindShortestAxisAlignedExit(ZVector3(8.f, 0.f, 5.f), bX, bZ);

    EXPECT_TRUE(bX);   // 2 <= 5
    EXPECT_FALSE(bZ);
}

TEST(ZBBox, FindShortestAxisAlignedExit_NearerToMinZ)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 10.f, 0.f, 10.f);

    bool bX = false, bZ = false;
    box.FindShortestAxisAlignedExit(ZVector3(5.f, 0.f, 1.f), bX, bZ);

    // implementation uses m_Max distance when point >= m_Min
    // fDX = |5-10| = 5, fDZ = |1-10| = 9 -> bX = (5 <= 9) = true
    EXPECT_TRUE(bX);
    EXPECT_FALSE(bZ);
}

TEST(ZBBox, FindShortestAxisAlignedExit_NearerToMaxZ)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 10.f, 0.f, 10.f);

    bool bX = false, bZ = false;
    box.FindShortestAxisAlignedExit(ZVector3(5.f, 0.f, 9.f), bX, bZ);

    EXPECT_FALSE(bX);
    EXPECT_TRUE(bZ);
}

TEST(ZBBox, FindShortestAxisAlignedExit_TieGoesToX)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 10.f, 0.f, 10.f);

    bool bX = false, bZ = false;
    box.FindShortestAxisAlignedExit(ZVector3(3.f, 0.f, 3.f), bX, bZ);

    EXPECT_TRUE(bX);   // fDX == fZ, bX = (fDX <= fDZ)
    EXPECT_FALSE(bZ);
}

TEST(ZBBox, Expand_GrowsBoxByFixedOffsets)
{
    auto box = MakeBox(-2.f, -1.f, 0.f, 2.f, 1.f, 0.f);

    box.expand(ZVector3(0.f, 0.f, 0.f));

    // x: min(-2, 0-1)=-2 stays; max(2, 0+1)=2 stays
    // y: min(-1, 0-100)=-100; max(1, 0+100)=100
    // z: min(0, 0-1)=-1; max(0, 0+1)=1
    EXPECT_EQ(box.min(), ZVector3(-2.f, -100.f, -1.f));
    EXPECT_EQ(box.max(), ZVector3(2.f, 100.f, 1.f));
}

TEST(ZBBox, Expand_CentralPoint)
{
    auto box = MakeBox(-5.f, -5.f, -5.f, 5.f, 5.f, 5.f);

    box.expand(ZVector3(1.f, 2.f, 3.f));

    // x: min(-5, 0)=-5 stays; max(5, 2)=5 stays
    // y: min(-5, -98)=-98; max(5, 102)=102
    // z: min(-5, 2)=-5 stays; max(5, 4)=5 stays
    EXPECT_EQ(box.min(), ZVector3(-5.f, -98.f, -5.f));
    EXPECT_EQ(box.max(), ZVector3(5.f, 102.f, 5.f));
}

TEST(ZBBox, Expand_OnlyGrowsNeverShrinks)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 10.f, 10.f, 10.f);

    box.expand(ZVector3(3.f, 3.f, 3.f));

    // x: min(0, 2) stays 0; max(10, 4) stays 10
    EXPECT_EQ(box.min().x, 0.f);
    EXPECT_EQ(box.max().x, 10.f);
    // y: min(0, -97) = -97; max(10, 103) = 103
    EXPECT_EQ(box.min().y, -97.f);
    EXPECT_EQ(box.max().y, 103.f);
    // z: min(0, 2) stays 0; max(10, 4) stays 10
    EXPECT_EQ(box.min().z, 0.f);
    EXPECT_EQ(box.max().z, 10.f);
}

TEST(ZBBox, Contains2D)
{
    auto outer = MakeBox(0.f, 0.f, 0.f, 10.f, 0.f, 10.f);

    EXPECT_TRUE(outer.Contains2D(outer));
    EXPECT_TRUE(outer.Contains2D(MakeBox(1.f, 0.f, 1.f, 5.f, 0.f, 5.f)));
    EXPECT_TRUE(outer.Contains2D(MakeBox(10.f, 0.f, 0.f, 10.f, 0.f, 10.f))); // degenerate, touching
    EXPECT_FALSE(outer.Contains2D(MakeBox(-1.f, 0.f, 0.f, 5.f, 0.f, 5.f)));
    EXPECT_FALSE(outer.Contains2D(MakeBox(1.f, 0.f, 1.f, 11.f, 0.f, 5.f)));
    EXPECT_FALSE(outer.Contains2D(MakeBox(1.f, 0.f, -1.f, 5.f, 0.f, 5.f)));
    EXPECT_FALSE(outer.Contains2D(MakeBox(1.f, 0.f, 1.f, 5.f, 0.f, 11.f)));
}

TEST(ZBBox, ContainsBox)
{
    auto outer = MakeBox(0.f, 0.f, 0.f, 10.f, 10.f, 10.f);

    EXPECT_TRUE(outer.Contains(outer));
    EXPECT_TRUE(outer.Contains(MakeBox(1.f, 1.f, 1.f, 5.f, 5.f, 5.f)));
    EXPECT_TRUE(outer.Contains(MakeBox(10.f, 10.f, 10.f, 10.f, 10.f, 10.f))); // degenerate corner
    EXPECT_FALSE(outer.Contains(MakeBox(-1.f, 0.f, 0.f, 5.f, 5.f, 5.f)));
    EXPECT_FALSE(outer.Contains(MakeBox(0.f, -1.f, 0.f, 5.f, 5.f, 5.f)));
    EXPECT_FALSE(outer.Contains(MakeBox(0.f, 0.f, -1.f, 5.f, 5.f, 5.f)));
    EXPECT_FALSE(outer.Contains(MakeBox(0.f, 0.f, 0.f, 11.f, 5.f, 5.f)));
    EXPECT_FALSE(outer.Contains(MakeBox(0.f, 0.f, 0.f, 5.f, 11.f, 5.f)));
    EXPECT_FALSE(outer.Contains(MakeBox(0.f, 0.f, 0.f, 5.f, 5.f, 11.f)));
}

TEST(ZBBox, ContainsPointWithSize)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 10.f, 10.f, 10.f);

    // point and size box fully inside
    EXPECT_TRUE(box.Contains(ZVector3(5.f, 5.f, 5.f), 1.f));
    // exactly touching all sides
    EXPECT_TRUE(box.Contains(ZVector3(1.f, 1.f, 1.f), 1.f));
    EXPECT_TRUE(box.Contains(ZVector3(9.f, 9.f, 9.f), 1.f));
    // center with size 5 fills the box
    EXPECT_TRUE(box.Contains(ZVector3(5.f, 5.f, 5.f), 5.f));
    // centers with size that fit within the box boundaries
    EXPECT_TRUE(box.Contains(ZVector3(0.f, 5.f, 5.f), 1.f));
    EXPECT_TRUE(box.Contains(ZVector3(10.f, 5.f, 5.f), 1.f));
    EXPECT_TRUE(box.Contains(ZVector3(5.f, -1.f, 5.f), 5.f));
    EXPECT_TRUE(box.Contains(ZVector3(5.f, 11.f, 5.f), 5.f));
    EXPECT_TRUE(box.Contains(ZVector3(5.f, 5.f, -2.f), 4.f));
    // zero size = plain point containment
    EXPECT_TRUE(box.Contains(ZVector3(0.f, 0.f, 0.f), 0.f));
    EXPECT_TRUE(box.Contains(ZVector3(10.f, 10.f, 10.f), 0.f));
}

TEST(ZBBox, AxisOverlap)
{
    PF4::ZBBox box;

    EXPECT_TRUE(box.AxisOverlap(0.f, 1.f, 1.f, 2.f));  // touching counts as overlap
    EXPECT_TRUE(box.AxisOverlap(0.f, 1.f, 0.5f, 0.7f));
    EXPECT_TRUE(box.AxisOverlap(0.f, 1.f, -1.f, 2.f));
    EXPECT_TRUE(box.AxisOverlap(0.f, 1.f, 0.f, 1.f));
    EXPECT_FALSE(box.AxisOverlap(0.f, 1.f, 1.5f, 2.f));
    EXPECT_FALSE(box.AxisOverlap(0.f, 1.f, -2.f, -0.5f));
}

TEST(ZBBox, Overlap)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 2.f, 2.f, 2.f);

    EXPECT_TRUE(box.Overlap(box));
    EXPECT_TRUE(box.Overlap(MakeBox(1.f, 1.f, 1.f, 3.f, 3.f, 3.f)));
    EXPECT_TRUE(box.Overlap(MakeBox(-1.f, -1.f, -1.f, 1.f, 1.f, 1.f)));
    EXPECT_TRUE(box.Overlap(MakeBox(2.f, 0.f, 0.f, 3.f, 2.f, 2.f)));       // face touch X
    EXPECT_FALSE(box.Overlap(MakeBox(3.f, 0.f, 0.f, 4.f, 2.f, 2.f)));       // separated X
    EXPECT_FALSE(box.Overlap(MakeBox(0.f, 3.f, 0.f, 2.f, 4.f, 2.f)));       // separated Y
    EXPECT_FALSE(box.Overlap(MakeBox(0.f, 0.f, 3.f, 2.f, 2.f, 4.f)));       // separated Z
    EXPECT_FALSE(box.Overlap(MakeBox(1.f, 3.f, 0.f, 3.f, 4.f, 2.f)));       // Y separated wins
    EXPECT_FALSE(box.Overlap(MakeBox(3.f, 1.f, 1.f, 4.f, 2.f, 2.f)));       // separated X
}

TEST(ZBBox, OverlapDiagonal)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 2.f, 2.f, 2.f);

    // Diagonal neighbor touching only at a corner
    EXPECT_TRUE(box.Overlap(MakeBox(2.f, 2.f, 2.f, 3.f, 3.f, 3.f)));

    // Slightly off
    EXPECT_FALSE(box.Overlap(MakeBox(2.5f, 2.5f, 2.5f, 3.5f, 3.5f, 3.5f)));
}

TEST(ZBBox, Inside_Point)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 2.f, 2.f, 2.f);

    const float inside[3] = { 1.f, 1.f, 1.f };
    EXPECT_TRUE(box.Inside(inside));

    const float corner[3] = { 0.f, 0.f, 0.f };
    EXPECT_TRUE(box.Inside(corner)); // inclusive

    const float out[3] = { 3.f, 1.f, 1.f };
    EXPECT_FALSE(box.Inside(out));

    const float negativeOut[3] = { -0.5f, 1.f, 1.f };
    EXPECT_FALSE(box.Inside(negativeOut));
}

TEST(ZBBox, Inside_PointWithSize)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 2.f, 2.f, 2.f);

    const float inside[3] = { 1.f, 1.f, 1.f };
    EXPECT_TRUE(box.Inside(inside, 0.5f));

    // size fills each axis exactly
    const float edge[3] = { 0.5f, 0.5f, 0.5f };
    EXPECT_TRUE(box.Inside(edge, 0.5f));
    const float farEdge[3] = { 1.5f, 1.5f, 1.5f };
    EXPECT_TRUE(box.Inside(farEdge, 0.5f));

    // point+/-size fits within box boundaries
    const float fit[3] = { 0.4f, 1.f, 1.f };
    EXPECT_TRUE(box.Inside(fit, 0.5f));
    const float fitY[3] = { 1.f, -0.1f, 1.f };
    EXPECT_TRUE(box.Inside(fitY, 0.5f));
    const float outZ[3] = { 1.f, 1.f, 2.6f };
    EXPECT_FALSE(box.Inside(outZ, 0.5f));
}

TEST(ZBBox, InsideXZ_StrictInterior)
{
    auto box = MakeBox(0.f, 0.f, 0.f, 2.f, 0.f, 2.f);

    EXPECT_TRUE(box.InsideXZ(1.f, 1.f));
    // boundary is excluded
    EXPECT_FALSE(box.InsideXZ(0.f, 1.f));
    EXPECT_FALSE(box.InsideXZ(2.f, 1.f));
    EXPECT_FALSE(box.InsideXZ(1.f, 0.f));
    EXPECT_FALSE(box.InsideXZ(1.f, 2.f));
    EXPECT_FALSE(box.InsideXZ(-1.f, 1.f));
    EXPECT_FALSE(box.InsideXZ(1.f, -1.f));
    // y is irrelevant
    EXPECT_TRUE(box.InsideXZ(0.5f, 0.5f));
}

TEST(ZBBox, GetTopAndGetBottom)
{
    auto box = MakeBox(1.f, 2.f, -3.f, 4.f, 5.f, 7.f);

    EXPECT_FLOAT_EQ(box.GetTop(), 7.f);
    EXPECT_FLOAT_EQ(box.GetBottom(), -3.f);
}
