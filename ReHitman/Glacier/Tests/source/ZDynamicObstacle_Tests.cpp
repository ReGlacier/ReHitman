#include <Glacier/PF4/ZDynamicObstacle.h>
#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>

using namespace Glacier::PF4;

namespace
{
    // Axis-aligned obstacle centred at the origin with half extents 1x1.
    ZDynamicObstacle MakeUnitSquareObstacle()
    {
        ZDynamicObstacle obstacle;
        const float vDir[3] = { 1.0f, 0.0f, 0.0f };
        obstacle.DirXZ(vDir);
        const float vExtents[2] = { 1.0f, 1.0f };
        obstacle.CreateHullFromBox(vExtents);
        return obstacle;
    }

    void ExpectNear(const float* pActual, float fX, float fY, float fTolerance)
    {
        EXPECT_NEAR(pActual[0], fX, fTolerance);
        EXPECT_NEAR(pActual[1], fY, fTolerance);
    }
}

TEST(ZDynamicObstacle, DefaultCtorClearsState)
{
    ZDynamicObstacle obstacle;

    EXPECT_EQ(obstacle.m_HullSize, 0);
    EXPECT_EQ(obstacle.m_Radius, 0.0f);
    EXPECT_EQ(obstacle.m_iLinks, 0);
    EXPECT_EQ(obstacle.m_Location.Component(), -1);
    EXPECT_FALSE(obstacle.Active());
}

TEST(ZDynamicObstacle, CtorFromBoxBuildsHullAndType)
{
    const float vBox[3] = { 3.0f, 0.0f, 4.0f };
    ZDynamicObstacle obstacle(vBox, eSolid);

    EXPECT_EQ(obstacle.m_Type, eSolid);
    EXPECT_EQ(obstacle.m_HullSize, 4);
    EXPECT_FLOAT_EQ(obstacle.m_Radius, std::sqrt(3.0f * 3.0f + 4.0f * 4.0f) * 1.5f);

    // Hull is a rectangle spanning +/-xExt and +/-zExt.
    EXPECT_FLOAT_EQ(obstacle.m_ConvexHull[0].x, 4.0f);
    EXPECT_FLOAT_EQ(obstacle.m_ConvexHull[0].y, 3.0f);
    EXPECT_FLOAT_EQ(obstacle.m_ConvexHull[2].x, -4.0f);
    EXPECT_FLOAT_EQ(obstacle.m_ConvexHull[2].y, -3.0f);
}

TEST(ZDynamicObstacle, CopyAssignmentCopiesWholeObject)
{
    const float vBox[3] = { 1.0f, 0.0f, 2.0f };
    ZDynamicObstacle source(vBox, eWarning);
    source.m_iLinks = 5;

    ZDynamicObstacle copy;
    copy = source;

    EXPECT_EQ(copy.m_Type, eWarning);
    EXPECT_EQ(copy.m_HullSize, source.m_HullSize);
    EXPECT_FLOAT_EQ(copy.m_Radius, source.m_Radius);
    EXPECT_EQ(copy.m_iLinks, 5);
    EXPECT_FLOAT_EQ(copy.m_ConvexHull[1].x, source.m_ConvexHull[1].x);
}

TEST(ZDynamicObstacle, ActiveReflectsLinkCount)
{
    ZDynamicObstacle obstacle;
    EXPECT_FALSE(obstacle.Active());
    obstacle.m_iLinks = 1;
    EXPECT_TRUE(obstacle.Active());
}

TEST(ZDynamicObstacle, IsInsideChecksHull)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();

    const float inside[2] = { 0.0f, 0.0f };
    const float edge[2] = { 0.9f, 0.9f };
    const float outside[2] = { 2.0f, 0.0f };

    EXPECT_TRUE(obstacle.IsInside(inside));
    EXPECT_TRUE(obstacle.IsInside(edge));
    EXPECT_FALSE(obstacle.IsInside(outside));
}

TEST(ZDynamicObstacle, IsInsideRespectsObstaclePosition)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();
    obstacle.m_Location.m_vPos.x = 5.0f;
    obstacle.m_Location.m_vPos.z = 0.0f;

    const float outsideOld[2] = { 2.0f, 0.0f };
    const float insideNew[2] = { 5.0f, 0.0f };
    EXPECT_FALSE(obstacle.IsInside(outsideOld));
    EXPECT_TRUE(obstacle.IsInside(insideNew));
}

TEST(ZDynamicObstacle, ClosestPointReturnsClosestHullPoint)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();

    const float from[2] = { 5.0f, 0.0f };
    float found[2] = { 0.0f, 0.0f };
    const float fDist = obstacle.ClosestPoint(from, found);

    EXPECT_FLOAT_EQ(found[0], 1.0f);
    EXPECT_FLOAT_EQ(found[1], 0.0f);
    EXPECT_FLOAT_EQ(fDist, 4.0f); // linear distance (PC 004D9CC0)
}

TEST(ZDynamicObstacle, IntersectRayThroughSquare)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();

    const float start[2] = { 5.0f, 0.0f };
    const float end[2] = { -5.0f, 0.0f };
    float tangent[2][2] = {};
    int iRight = -1;
    int iLeft = -1;

    const float fT = obstacle.Intersect(start, end, 100.0f,
        std::numeric_limits<float>::max(), tangent, iRight, iLeft);

    // Hull indices: 0 = (1,1) top-right, 1 = (-1,1), 2 = (-1,-1), 3 = (1,-1).
    EXPECT_NEAR(fT, 4.0f, 1e-4f); // entry through the x = 1 side
    EXPECT_EQ(iRight, 0);
    EXPECT_EQ(iLeft, 3);
    ExpectNear(tangent[0], 1.0f, 1.0f, 1e-4f);
    ExpectNear(tangent[1], 1.0f, -1.0f, 1e-4f);
}

TEST(ZDynamicObstacle, IntersectRespectsObstaclePosition)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();
    obstacle.m_Location.m_vPos.x = 10.0f;
    obstacle.m_Location.m_vPos.z = -4.0f;

    const float start[2] = { 15.0f, -4.0f };
    const float end[2] = { 5.0f, -4.0f };
    float tangent[2][2] = {};
    int iRight = -1;
    int iLeft = -1;

    const float fT = obstacle.Intersect(start, end, 100.0f,
        std::numeric_limits<float>::max(), tangent, iRight, iLeft);

    EXPECT_NEAR(fT, 4.0f, 1e-4f);
    EXPECT_EQ(iRight, 0);
    EXPECT_EQ(iLeft, 3);
    ExpectNear(tangent[0], 11.0f, -3.0f, 1e-4f);
    ExpectNear(tangent[1], 11.0f, -5.0f, 1e-4f);
}

TEST(ZDynamicObstacle, IntersectMissReturnsBest)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();

    const float start[2] = { 5.0f, 5.0f };
    const float end[2] = { 5.0f, 6.0f };
    float tangent[2][2] = {};
    int iRight = -1;
    int iLeft = -1;

    const float fT = obstacle.Intersect(start, end, 100.0f, 3.5f, tangent, iRight, iLeft);

    EXPECT_FLOAT_EQ(fT, 3.5f);
    EXPECT_EQ(iRight, -1);
    EXPECT_EQ(iLeft, -1);
}

TEST(ZDynamicObstacle, IntersectDegenerateRayReturnsMinusOne)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();

    const float start[2] = { 5.0f, 0.0f };
    float tangent[2][2] = {};
    int iRight = -1;
    int iLeft = -1;

    const float fT = obstacle.Intersect(start, start, 100.0f,
        std::numeric_limits<float>::max(), tangent, iRight, iLeft);

    EXPECT_FLOAT_EQ(fT, -1.0f);
    EXPECT_EQ(iRight, -1);
    EXPECT_EQ(iLeft, -1);
}

TEST(ZDynamicObstacle, IntersectDoesNotReplaceBestWithFartherHit)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();

    const float start[2] = { 5.0f, 0.0f };
    const float end[2] = { -5.0f, 0.0f };
    float tangent[2][2] = {};
    int iRight = -1;
    int iLeft = -1;

    // A crossing exists but at distance 4, which is not below best = 3.
    const float fT = obstacle.Intersect(start, end, 100.0f, 3.0f, tangent, iRight, iLeft);

    EXPECT_FLOAT_EQ(fT, 3.0f);
    EXPECT_EQ(iRight, -1);
    EXPECT_EQ(iLeft, -1);
}

TEST(ZDynamicObstacle, FindShortestRouteSymmetricChoosesRightVertex)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();

    const float from[2] = { 5.0f, 0.0f };
    const float target[2] = { -5.0f, 0.0f };
    float route[2] = { 0.0f, 0.0f };

    obstacle.FindShortestRouteToTarget(from, target, 0, 3, route);

    // Both directions cost the same; the right vertex (0 = (1,1)) is picked and
    // the way point is pushed 20 units past the corner away from the centre.
    const float fExpected = 1.0f + 20.0f / std::sqrt(2.0f);
    ExpectNear(route, fExpected, fExpected, 1e-3f);
}

TEST(ZDynamicObstacle, FindShortestRoutePrefersNearTargetSide)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();

    const float from[2] = { 5.0f, 0.0f };
    const float target[2] = { -5.0f, -2.0f };
    float route[2] = { 0.0f, 0.0f };

    // Backward route from the right vertex 0 (top corner) is shorter, so the
    // top corner is returned.
    obstacle.FindShortestRouteToTarget(from, target, 0, 3, route);

    const float fExpected = 1.0f + 20.0f / std::sqrt(2.0f);
    ExpectNear(route, fExpected, fExpected, 1e-3f);
}

TEST(ZDynamicObstacle, FindShortestRoutePrefersFarTargetSide)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();

    const float from[2] = { 5.0f, 0.0f };
    const float target[2] = { -5.0f, 2.0f };
    float route[2] = { 0.0f, 0.0f };

    // Forward route from the left vertex 3 (bottom corner) is shorter now, so
    // the bottom corner is returned.
    obstacle.FindShortestRouteToTarget(from, target, 0, 3, route);

    const float fExpected = 1.0f + 20.0f / std::sqrt(2.0f);
    ExpectNear(route, fExpected, -fExpected, 1e-3f);
}

TEST(ZDynamicObstacle, IntersectFeedsShortestRoute)
{
    ZDynamicObstacle obstacle = MakeUnitSquareObstacle();

    const float start[2] = { 5.0f, 0.0f };
    const float end[2] = { -5.0f, 0.0f };
    float tangent[2][2] = {};
    int iRight = -1;
    int iLeft = -1;

    const float fT = obstacle.Intersect(start, end, 100.0f,
        std::numeric_limits<float>::max(), tangent, iRight, iLeft);
    ASSERT_NEAR(fT, 4.0f, 1e-4f);
    ASSERT_EQ(iRight, 0);
    ASSERT_EQ(iLeft, 3);

    float route[2] = { 0.0f, 0.0f };
    obstacle.FindShortestRouteToTarget(start, end, iRight, iLeft, route);

    const float fExpected = 1.0f + 20.0f / std::sqrt(2.0f);
    ExpectNear(route, fExpected, fExpected, 1e-3f);
}
