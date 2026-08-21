#include <gtest/gtest.h>
#include <Tests/EngineFixture.h>
#include <Glacier/Physics/ZCommonAlgorithms.h>
#include <Glacier/Physics/SFastBoxColiTri.h>
#include <Glacier/Physics/SCapsuleColiInfo.h>
#include <cmath>

using namespace Glacier;

// Helper to compare floats with tolerance
static bool FloatNear(float a, float b, float tolerance = 0.0001f)
{
    return std::fabs(a - b) < tolerance;
}

static bool Vec3Near(const float* a, const float* b, float tolerance = 0.0001f)
{
    return FloatNear(a[0], b[0], tolerance) &&
           FloatNear(a[1], b[1], tolerance) &&
           FloatNear(a[2], b[2], tolerance);
}

// Fixture for tests that need g_pSysInterface initialized
class ZCommonAlgorithmsFixture : public Tests::EngineFixture
{
};

TEST(ZCommonAlgorithms, Solve3x3System_Identity)
{
    // Identity matrix
    const float matrix[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float target[3] = { 1.0f, 2.0f, 3.0f };
    float solution[3];

    EXPECT_TRUE(ZCommonAlgorithms::Solve3x3System(matrix, target, solution));
    EXPECT_TRUE(Vec3Near(solution, target));
}

TEST(ZCommonAlgorithms, Solve3x3System_Singular)
{
    // Singular matrix (determinant = 0)
    const float matrix[9] = {
        1.0f, 2.0f, 3.0f,
        2.0f, 4.0f, 6.0f,
        3.0f, 6.0f, 9.0f
    };
    const float target[3] = { 1.0f, 2.0f, 3.0f };
    float solution[3];

    EXPECT_FALSE(ZCommonAlgorithms::Solve3x3System(matrix, target, solution));
}

TEST(ZCommonAlgorithms, Solve3x3System_KnownSolution)
{
    // Matrix with known solution: x = 1, y = 2, z = 3
    // | 2 0 0 | |1|   |2|
    // | 0 3 0 | |2| = |6|
    // | 0 0 4 | |3|   |12|
    const float matrix[9] = {
        2.0f, 0.0f, 0.0f,
        0.0f, 3.0f, 0.0f,
        0.0f, 0.0f, 4.0f
    };
    const float target[3] = { 2.0f, 6.0f, 12.0f };
    float solution[3];

    EXPECT_TRUE(ZCommonAlgorithms::Solve3x3System(matrix, target, solution));
    EXPECT_TRUE(FloatNear(solution[0], 1.0f));
    EXPECT_TRUE(FloatNear(solution[1], 2.0f));
    EXPECT_TRUE(FloatNear(solution[2], 3.0f));
}

TEST(ZCommonAlgorithms, Solve3x3System_ColumnMajorBasis)
{
    // Non-symmetric matrix: columns are the basis vectors (1,0,0), (1,1,0), (0,0,1).
    // target = 2*col0 + 3*col1 + 4*col2, so the solution must be (2, 3, 4).
    const float matrix[9] = {
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float target[3] = { 5.0f, 3.0f, 4.0f };
    float solution[3];

    EXPECT_TRUE(ZCommonAlgorithms::Solve3x3System(matrix, target, solution));
    EXPECT_TRUE(FloatNear(solution[0], 2.0f));
    EXPECT_TRUE(FloatNear(solution[1], 3.0f));
    EXPECT_TRUE(FloatNear(solution[2], 4.0f));
}

TEST(ZCommonAlgorithms, DistPointLineVar2_PointOnLine)
{
    // Point very close to line but not exactly on it (to avoid RandomUnitVector call)
    const float point[3] = { 1.0f, 0.0001f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    float t, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_TRUE(ZCommonAlgorithms::DistPointLineVar2(point, lineStart, lineEnd, &t, &minDist, &dist, dir));
    EXPECT_TRUE(FloatNear(t, 0.5f));
    EXPECT_TRUE(FloatNear(dist, 0.0001f));
}

TEST(ZCommonAlgorithms, DistPointLineVar2_PointAboveLine)
{
    const float point[3] = { 1.0f, 1.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    float t, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_TRUE(ZCommonAlgorithms::DistPointLineVar2(point, lineStart, lineEnd, &t, &minDist, &dist, dir));
    EXPECT_TRUE(FloatNear(t, 0.5f));
    EXPECT_TRUE(FloatNear(dist, 1.0f));
    EXPECT_TRUE(FloatNear(dir[0], 0.0f));
    EXPECT_TRUE(FloatNear(dir[1], 1.0f));
    EXPECT_TRUE(FloatNear(dir[2], 0.0f));
}

TEST(ZCommonAlgorithms, DistPointLineVar2_PointBeforeStart)
{
    const float point[3] = { -1.0f, 0.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    float t, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_TRUE(ZCommonAlgorithms::DistPointLineVar2(point, lineStart, lineEnd, &t, &minDist, &dist, dir));
    EXPECT_TRUE(FloatNear(t, 0.0f));
    EXPECT_TRUE(FloatNear(dist, 1.0f));
}

TEST(ZCommonAlgorithms, DistPointLineVar2_PointAfterEnd)
{
    const float point[3] = { 3.0f, 0.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    float t, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_TRUE(ZCommonAlgorithms::DistPointLineVar2(point, lineStart, lineEnd, &t, &minDist, &dist, dir));
    EXPECT_TRUE(FloatNear(t, 1.0f));
    EXPECT_TRUE(FloatNear(dist, 1.0f));
}

TEST(ZCommonAlgorithms, DistPointLineVar2_TooFar)
{
    const float point[3] = { 1.0f, 100.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    float t, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_FALSE(ZCommonAlgorithms::DistPointLineVar2(point, lineStart, lineEnd, &t, &minDist, &dist, dir));
}

TEST(ZCommonAlgorithms, IntersectTriangleAndLine3_Hit)
{
    // Triangle in XY plane: v0=(0,0,0), v1=(1,0,0), v2=(0,1,0)
    const float vert0[3] = { 0.0f, 0.0f, 0.0f };
    const float vert1[3] = { 1.0f, 0.0f, 0.0f };
    const float vert2[3] = { 0.0f, 1.0f, 0.0f };

    // Edge vectors
    const float edge1[3] = { 1.0f, 0.0f, 0.0f };
    const float edge2[3] = { 0.0f, 1.0f, 0.0f };

    // Normal = cross(edge1, edge2) = (0, 0, 1)
    const float normal[3] = { 0.0f, 0.0f, 1.0f };

    // Build matrix [edge1, edge2, normal] and compute its inverse
    // Matrix:
    // | 1 0 0 |
    // | 0 1 0 |
    // | 0 0 1 |
    // Inverse is identity
    const float inv[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    // Line going through triangle from below (t must be < 1.0)
    const float orig[3] = { 0.25f, 0.25f, -0.5f };
    const float dir[3] = { 0.0f, 0.0f, 1.0f };

    float coliPoint[3];
    float t;

    EXPECT_TRUE(ZCommonAlgorithms::IntersectTriangleAndLine3(coliPoint, orig, dir, vert0, inv, &t, true));
    EXPECT_TRUE(FloatNear(t, 0.5f));
    EXPECT_TRUE(FloatNear(coliPoint[0], 0.25f));
    EXPECT_TRUE(FloatNear(coliPoint[1], 0.25f));
    EXPECT_TRUE(FloatNear(coliPoint[2], 0.0f));
}

TEST(ZCommonAlgorithms, IntersectTriangleAndLine3_Miss)
{
    const float vert0[3] = { 0.0f, 0.0f, 0.0f };
    const float vert1[3] = { 1.0f, 0.0f, 0.0f };
    const float vert2[3] = { 0.0f, 1.0f, 0.0f };

    const float inv[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    // Line going away from triangle
    const float orig[3] = { 2.0f, 2.0f, -1.0f };
    const float dir[3] = { 0.0f, 0.0f, -1.0f };

    float coliPoint[3];
    float t;

    EXPECT_FALSE(ZCommonAlgorithms::IntersectTriangleAndLine3(coliPoint, orig, dir, vert0, inv, &t, true));
}

TEST(ZCommonAlgorithms, IntersectTriangleAndSphere_AboveTriangle)
{
    // Triangle in XY plane
    const float triverts[3][3] = {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f }
    };

    // Sphere above triangle center
    const float spherepos[3] = { 0.25f, 0.25f, 0.5f };
    const float radius = 1.0f;

    float vDir[3];
    float scaledDist[3];
    float penetration;

    EXPECT_TRUE(ZCommonAlgorithms::IntersectTriangleAndSphere(triverts, &spherepos, radius, &vDir, scaledDist, &penetration));
    EXPECT_TRUE(FloatNear(penetration, 0.5f)); // radius - distance
}

TEST(ZCommonAlgorithms, IntersectTriangleAndSphere_BelowTriangle)
{
    const float triverts[3][3] = {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f }
    };

    // Sphere below triangle center
    const float spherepos[3] = { 0.25f, 0.25f, -0.5f };
    const float radius = 1.0f;

    float vDir[3];
    float scaledDist[3];
    float penetration;

    EXPECT_TRUE(ZCommonAlgorithms::IntersectTriangleAndSphere(triverts, &spherepos, radius, &vDir, scaledDist, &penetration));
    EXPECT_TRUE(FloatNear(penetration, 0.5f)); // radius - distance
}

TEST(ZCommonAlgorithms, IntersectTriangleAndSphere_Miss)
{
    const float triverts[3][3] = {
        { 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f }
    };

    // Sphere far from triangle
    const float spherepos[3] = { 10.0f, 10.0f, 10.0f };
    const float radius = 1.0f;

    float vDir[3];
    float scaledDist[3];
    float penetration;

    EXPECT_FALSE(ZCommonAlgorithms::IntersectTriangleAndSphere(triverts, &spherepos, radius, &vDir, scaledDist, &penetration));
}

TEST(ZCommonAlgorithms, AdjustPart2rigid_Basic)
{
    // Use 4 floats to avoid SSE overwrite (AdjustPart2rigid uses _mm_loadu_ps/_mm_storeu_ps)
    alignas(16) float x1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    alignas(16) float x2[4] = { 2.0f, 0.0f, 0.0f, 0.0f };
    const float m1 = 1.0f;
    const float m2 = 1.0f;
    const float dist = 1.0f;

    ZCommonAlgorithms::AdjustPart2rigid(x1, x2, m1, m2, dist);

    // After adjustment, distance should be equal to dist
    const float dx = x2[0] - x1[0];
    const float dy = x2[1] - x1[1];
    const float dz = x2[2] - x1[2];
    const float newDist = std::sqrt(dx * dx + dy * dy + dz * dz);

    EXPECT_TRUE(FloatNear(newDist, dist));
}

TEST_F(ZCommonAlgorithmsFixture, AdjustPart2rigid_ZeroDistance)
{
    // With EngineFixture, g_pSysInterface is initialized, so RandomUnitVector works
    alignas(16) float x1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    alignas(16) float x2[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float m1 = 1.0f;
    const float m2 = 1.0f;
    const float dist = 1.0f;

    // Should not crash and should separate particles
    ZCommonAlgorithms::AdjustPart2rigid(x1, x2, m1, m2, dist);

    const float dx = x2[0] - x1[0];
    const float dy = x2[1] - x1[1];
    const float dz = x2[2] - x1[2];
    const float newDist = std::sqrt(dx * dx + dy * dy + dz * dz);

    EXPECT_TRUE(FloatNear(newDist, dist));
}

TEST(ZCommonAlgorithms, AdjustPart2rigid_DifferentMasses)
{
    alignas(16) float x1[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    alignas(16) float x2[4] = { 2.0f, 0.0f, 0.0f, 0.0f };
    const float m1 = 1.0f;
    const float m2 = 3.0f;
    const float dist = 1.0f;

    ZCommonAlgorithms::AdjustPart2rigid(x1, x2, m1, m2, dist);

    // Heavier particle (m2) should move less
    const float dx = x2[0] - x1[0];
    const float dy = x2[1] - x1[1];
    const float dz = x2[2] - x1[2];
    const float newDist = std::sqrt(dx * dx + dy * dy + dz * dz);

    EXPECT_TRUE(FloatNear(newDist, dist));
}

TEST(ZCommonAlgorithms, CapsuleSphereCollision_Hit)
{
    // Capsule from (0,0,0) to (2,0,0) with radius 0.5
    const ZVector3 cp0(0.0f, 0.0f, 0.0f);
    const ZVector3 cp1(2.0f, 0.0f, 0.0f);
    const float cr = 0.5f;

    // Sphere at (1, 0.3, 0) with radius 0.5
    const ZVector3 sc(1.0f, 0.3f, 0.0f);
    const float sr = 0.5f;

    ZVector3 vDir;
    float fLen;

    EXPECT_TRUE(ZCommonAlgorithms::CapsuleSphereCollision(vDir, fLen, cp0, cp1, cr, sc, sr));

    // Direction should point from capsule to sphere (positive Y)
    EXPECT_TRUE(FloatNear(vDir.x, 0.0f));
    EXPECT_TRUE(FloatNear(vDir.y, 1.0f));
    EXPECT_TRUE(FloatNear(vDir.z, 0.0f));

    // Penetration depth = combined radius - distance
    // Distance from sphere center to capsule axis = 0.3
    // Combined radius = 0.5 + 0.5 = 1.0
    // Penetration = 1.0 - 0.3 = 0.7
    EXPECT_TRUE(FloatNear(fLen, 0.7f));
}

TEST(ZCommonAlgorithms, CapsuleSphereCollision_Miss)
{
    // Capsule from (0,0,0) to (2,0,0) with radius 0.5
    const ZVector3 cp0(0.0f, 0.0f, 0.0f);
    const ZVector3 cp1(2.0f, 0.0f, 0.0f);
    const float cr = 0.5f;

    // Sphere far from capsule
    const ZVector3 sc(1.0f, 5.0f, 0.0f);
    const float sr = 0.5f;

    ZVector3 vDir;
    float fLen;

    EXPECT_FALSE(ZCommonAlgorithms::CapsuleSphereCollision(vDir, fLen, cp0, cp1, cr, sc, sr));
}

// -----------------------------------------------------------------------------
// CheckCutInside
// -----------------------------------------------------------------------------

TEST(ZCommonAlgorithms, CheckCutInside_PointInside)
{
    // Triangle in the XY plane (dominant normal axis = Z), vertices stored contiguously
    const float tri[9] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    const float normal[3] = { 0.0f, 0.0f, 1.0f };
    const float point[3] = { 0.25f, 0.25f, 0.0f };

    EXPECT_TRUE(ZCommonAlgorithms::CheckCutInside(tri, normal, point));
}

TEST(ZCommonAlgorithms, CheckCutInside_PointOutside)
{
    const float tri[9] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    const float normal[3] = { 0.0f, 0.0f, 1.0f };
    const float point[3] = { 0.75f, 0.75f, 0.0f }; // beyond the hypotenuse

    EXPECT_FALSE(ZCommonAlgorithms::CheckCutInside(tri, normal, point));
}

TEST(ZCommonAlgorithms, CheckCutInside_PointOnVertex)
{
    const float tri[9] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    const float normal[3] = { 0.0f, 0.0f, 1.0f };
    const float point[3] = { 0.0f, 0.0f, 0.0f };

    EXPECT_TRUE(ZCommonAlgorithms::CheckCutInside(tri, normal, point));
}

TEST(ZCommonAlgorithms, CheckCutInside_PointOnEdge)
{
    const float tri[9] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    const float normal[3] = { 0.0f, 0.0f, 1.0f };
    const float point[3] = { 0.5f, 0.0f, 0.0f }; // midpoint of edge V0-V1

    EXPECT_TRUE(ZCommonAlgorithms::CheckCutInside(tri, normal, point));
}

TEST(ZCommonAlgorithms, CheckCutInside_OppositeWinding)
{
    // Same triangle, reversed winding (normal flipped) - result must not change
    const float tri[9] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f
    };
    const float normal[3] = { 0.0f, 0.0f, -1.0f };
    const float inside[3] = { 0.25f, 0.25f, 0.0f };
    const float outside[3] = { 0.75f, 0.75f, 0.0f };

    EXPECT_TRUE(ZCommonAlgorithms::CheckCutInside(tri, normal, inside));
    EXPECT_FALSE(ZCommonAlgorithms::CheckCutInside(tri, normal, outside));
}

TEST(ZCommonAlgorithms, CheckCutInside_DominantAxisX)
{
    // Triangle in the YZ plane (dominant normal axis = X)
    const float tri[9] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float normal[3] = { 1.0f, 0.0f, 0.0f };
    const float inside[3] = { 0.0f, 0.25f, 0.25f };
    const float outside[3] = { 0.0f, 0.75f, 0.75f };

    EXPECT_TRUE(ZCommonAlgorithms::CheckCutInside(tri, normal, inside));
    EXPECT_FALSE(ZCommonAlgorithms::CheckCutInside(tri, normal, outside));
}

TEST(ZCommonAlgorithms, CheckCutInside_DominantAxisY)
{
    // Triangle in the XZ plane (dominant normal axis = Y)
    const float tri[9] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f
    };
    const float normal[3] = { 0.0f, 1.0f, 0.0f };
    const float inside[3] = { 0.25f, 0.0f, 0.25f };
    const float outside[3] = { 0.75f, 0.0f, 0.75f };

    EXPECT_TRUE(ZCommonAlgorithms::CheckCutInside(tri, normal, inside));
    EXPECT_FALSE(ZCommonAlgorithms::CheckCutInside(tri, normal, outside));
}

// -----------------------------------------------------------------------------
// LnSphColl (segment vs unit sphere centered at the origin)
// -----------------------------------------------------------------------------

TEST(ZCommonAlgorithms, LnSphColl_StartInsideSphere)
{
    float p1[3] = { 0.5f, 0.0f, 0.0f };
    float p2[3] = { 5.0f, 0.0f, 0.0f };

    EXPECT_TRUE(ZCommonAlgorithms::LnSphColl(p1, p2));
}

TEST(ZCommonAlgorithms, LnSphColl_SegmentThroughSphere)
{
    float p1[3] = { -2.0f, 0.0f, 0.0f };
    float p2[3] = { 2.0f, 0.0f, 0.0f };

    EXPECT_TRUE(ZCommonAlgorithms::LnSphColl(p1, p2));
}

TEST(ZCommonAlgorithms, LnSphColl_ClosestPointWithinSegment)
{
    // Both endpoints outside, closest point of the line (origin) inside the segment
    float p1[3] = { 2.0f, 0.5f, 0.0f };
    float p2[3] = { -2.0f, 0.5f, 0.0f };

    EXPECT_TRUE(ZCommonAlgorithms::LnSphColl(p1, p2));
}

TEST(ZCommonAlgorithms, LnSphColl_EndInsideSphereIsMissFromThisSide)
{
    // Asymmetry of the original algorithm: only Pos1 and the line projection are
    // tested; Pos2 inside the sphere is NOT detected by this call. PolySphColl()
    // covers that case through the LnSphColl() call of the adjacent edge, where
    // this vertex is the start point.
    float p1[3] = { 2.0f, 0.0f, 0.0f };
    float p2[3] = { 0.5f, 0.0f, 0.0f };

    EXPECT_FALSE(ZCommonAlgorithms::LnSphColl(p1, p2));

    // ...but from the other direction the same segment hits (Pos1 inside).
    float q1[3] = { 0.5f, 0.0f, 0.0f };
    float q2[3] = { 2.0f, 0.0f, 0.0f };

    EXPECT_TRUE(ZCommonAlgorithms::LnSphColl(q1, q2));
}

TEST(ZCommonAlgorithms, LnSphColl_MissClosestBeyondEnd)
{
    // Closest point of the infinite line is the origin, but it lies past Pos2
    float p1[3] = { 2.0f, 0.0f, 0.0f };
    float p2[3] = { 3.0f, 0.0f, 0.0f };

    EXPECT_FALSE(ZCommonAlgorithms::LnSphColl(p1, p2));
}

TEST(ZCommonAlgorithms, LnSphColl_MissBehindStart)
{
    // Sphere lies behind the segment start
    float p1[3] = { 2.0f, 0.2f, 0.0f };
    float p2[3] = { 3.0f, 0.2f, 0.0f };

    EXPECT_FALSE(ZCommonAlgorithms::LnSphColl(p1, p2));
}

TEST(ZCommonAlgorithms, LnSphColl_TangentMiss)
{
    // Passing by the sphere at distance > 1
    float p1[3] = { -2.0f, 1.5f, 0.0f };
    float p2[3] = { 2.0f, 1.5f, 0.0f };

    EXPECT_FALSE(ZCommonAlgorithms::LnSphColl(p1, p2));
}

TEST(ZCommonAlgorithms, LnSphColl_GrazingHit)
{
    // Passing by the sphere at distance 0.9 (< 1) - must hit
    float p1[3] = { -2.0f, 0.9f, 0.0f };
    float p2[3] = { 2.0f, 0.9f, 0.0f };

    EXPECT_TRUE(ZCommonAlgorithms::LnSphColl(p1, p2));
}

// -----------------------------------------------------------------------------
// PolySphColl (triangle vs sphere; identity matrix = unit sphere at SphPos)
// -----------------------------------------------------------------------------

TEST(ZCommonAlgorithms, PolySphColl_FaceHit)
{
    // Unit sphere (identity matrix) at the origin, triangle directly above it
    const float sphPos[3] = { 0.0f, 0.0f, 0.0f };
    const float sphMat[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float v1[3] = { -1.0f, -1.0f, 0.5f };
    const float v2[3] = { 1.0f, -1.0f, 0.5f };
    const float v3[3] = { 0.0f, 1.0f, 0.5f };

    EXPECT_TRUE(ZCommonAlgorithms::PolySphColl(sphPos, sphMat, v1, v2, v3));
}

TEST(ZCommonAlgorithms, PolySphColl_EdgeHit)
{
    // Sphere touches the triangle edge (V1-V2 runs along X at distance 0.9)
    const float sphPos[3] = { 0.0f, 0.0f, 0.0f };
    const float sphMat[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float v1[3] = { -1.0f, 0.9f, 0.0f };
    const float v2[3] = { 1.0f, 0.9f, 0.0f };
    const float v3[3] = { 0.0f, 3.0f, 0.0f };

    EXPECT_TRUE(ZCommonAlgorithms::PolySphColl(sphPos, sphMat, v1, v2, v3));
}

TEST(ZCommonAlgorithms, PolySphColl_VertexInsideSphere)
{
    // One triangle vertex lies inside the unit sphere
    const float sphPos[3] = { 0.0f, 0.0f, 0.0f };
    const float sphMat[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float v1[3] = { 0.0f, 0.0f, 0.5f };
    const float v2[3] = { 5.0f, 0.0f, 5.0f };
    const float v3[3] = { 0.0f, 5.0f, 5.0f };

    EXPECT_TRUE(ZCommonAlgorithms::PolySphColl(sphPos, sphMat, v1, v2, v3));
}

TEST(ZCommonAlgorithms, PolySphColl_MissFarAway)
{
    const float sphPos[3] = { 0.0f, 0.0f, 0.0f };
    const float sphMat[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float v1[3] = { 9.0f, 9.0f, 10.0f };
    const float v2[3] = { 11.0f, 9.0f, 10.0f };
    const float v3[3] = { 10.0f, 11.0f, 10.0f };

    EXPECT_FALSE(ZCommonAlgorithms::PolySphColl(sphPos, sphMat, v1, v2, v3));
}

TEST(ZCommonAlgorithms, PolySphColl_MissOutsideEdge)
{
    // Plane is within range (z = 0.5) but the projected center lands outside the triangle
    const float sphPos[3] = { 0.0f, 0.0f, 0.0f };
    const float sphMat[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float v1[3] = { 2.0f, 2.0f, 0.5f };
    const float v2[3] = { 3.0f, 2.0f, 0.5f };
    const float v3[3] = { 2.5f, 3.0f, 0.5f };

    EXPECT_FALSE(ZCommonAlgorithms::PolySphColl(sphPos, sphMat, v1, v2, v3));
}

TEST(ZCommonAlgorithms, PolySphColl_MissBeyondPlane)
{
    // Triangle plane is further than the unit radius
    const float sphPos[3] = { 0.0f, 0.0f, 0.0f };
    const float sphMat[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float v1[3] = { -1.0f, -1.0f, 1.5f };
    const float v2[3] = { 1.0f, -1.0f, 1.5f };
    const float v3[3] = { 0.0f, 1.0f, 1.5f };

    EXPECT_FALSE(ZCommonAlgorithms::PolySphColl(sphPos, sphMat, v1, v2, v3));
}

TEST(ZCommonAlgorithms, PolySphColl_DegenerateTriangle)
{
    // Zero-area triangle must be rejected even with a vertex inside the sphere
    const float sphPos[3] = { 0.0f, 0.0f, 0.0f };
    const float sphMat[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float v1[3] = { 0.0f, 0.0f, 0.0f };
    const float v2[3] = { 0.0f, 0.0f, 0.0f };
    const float v3[3] = { 0.0f, 0.0f, 0.0f };

    EXPECT_FALSE(ZCommonAlgorithms::PolySphColl(sphPos, sphMat, v1, v2, v3));
}

TEST(ZCommonAlgorithms, PolySphColl_ScaledSphere)
{
    // Sphere of radius 2 at the origin: inverse basis rows divided by dimensions (2,2,2).
    // A triangle at distance 1.5 must hit (1.5 < 2), even though it would miss a unit sphere.
    const float sphPos[3] = { 0.0f, 0.0f, 0.0f };
    const float sphMat[9] = {
        0.5f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f,
        0.0f, 0.0f, 0.5f
    };
    const float v1[3] = { -2.0f, -2.0f, 1.5f };
    const float v2[3] = { 2.0f, -2.0f, 1.5f };
    const float v3[3] = { 0.0f, 2.0f, 1.5f };

    EXPECT_TRUE(ZCommonAlgorithms::PolySphColl(sphPos, sphMat, v1, v2, v3));
}

TEST(ZCommonAlgorithms, PolySphColl_OffsetSphere)
{
    // Sphere centered at (0, 0, 5), triangle right under it
    const float sphPos[3] = { 0.0f, 0.0f, 5.0f };
    const float sphMat[9] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };
    const float v1[3] = { -1.0f, -1.0f, 4.5f };
    const float v2[3] = { 1.0f, -1.0f, 4.5f };
    const float v3[3] = { 0.0f, 1.0f, 4.5f };

    EXPECT_TRUE(ZCommonAlgorithms::PolySphColl(sphPos, sphMat, v1, v2, v3));

    // Same triangle, sphere moved away
    const float sphPosFar[3] = { 0.0f, 0.0f, 10.0f };

    EXPECT_FALSE(ZCommonAlgorithms::PolySphColl(sphPosFar, sphMat, v1, v2, v3));
}

// -----------------------------------------------------------------------------
// DistPointLineVar (reference overload; point vs segment, rejects projection outside)
// -----------------------------------------------------------------------------

TEST(ZCommonAlgorithms, DistPointLineVar_ProjectedInside)
{
    const float point[3] = { 1.0f, 1.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    const float expectedDir[3] = { 0.0f, -1.0f, 0.0f };
    float s, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_TRUE(ZCommonAlgorithms::DistPointLineVar(point, lineStart, lineEnd, s, minDist, dist, dir));
    EXPECT_TRUE(FloatNear(s, 0.5f));
    EXPECT_TRUE(FloatNear(dist, 1.0f));
    EXPECT_TRUE(Vec3Near(dir, expectedDir));
}

TEST(ZCommonAlgorithms, DistPointLineVar_BeforeStart)
{
    const float point[3] = { -1.0f, 0.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    float s, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_FALSE(ZCommonAlgorithms::DistPointLineVar(point, lineStart, lineEnd, s, minDist, dist, dir));
}

TEST(ZCommonAlgorithms, DistPointLineVar_AfterEnd)
{
    const float point[3] = { 3.0f, 0.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    float s, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_FALSE(ZCommonAlgorithms::DistPointLineVar(point, lineStart, lineEnd, s, minDist, dist, dir));
}

TEST(ZCommonAlgorithms, DistPointLineVar_TooFar)
{
    const float point[3] = { 1.0f, 100.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    float s, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_FALSE(ZCommonAlgorithms::DistPointLineVar(point, lineStart, lineEnd, s, minDist, dist, dir));
}

// -----------------------------------------------------------------------------
// DistPointLineVar2 (reference overload; clamps projection to the segment)
// -----------------------------------------------------------------------------

TEST(ZCommonAlgorithms, DistPointLineVar2Ref_ProjectedInside)
{
    const float point[3] = { 1.0f, 1.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    const float expectedDir[3] = { 0.0f, 1.0f, 0.0f };
    float t, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_TRUE(ZCommonAlgorithms::DistPointLineVar2(point, lineStart, lineEnd, t, minDist, dist, dir));
    EXPECT_TRUE(FloatNear(t, 0.5f));
    EXPECT_TRUE(FloatNear(dist, 1.0f));
    EXPECT_TRUE(Vec3Near(dir, expectedDir));
}

TEST(ZCommonAlgorithms, DistPointLineVar2Ref_BeforeStart)
{
    const float point[3] = { -1.0f, 0.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    float t, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_TRUE(ZCommonAlgorithms::DistPointLineVar2(point, lineStart, lineEnd, t, minDist, dist, dir));
    EXPECT_TRUE(FloatNear(t, 0.0f));
    EXPECT_TRUE(FloatNear(dist, 1.0f));
}

TEST(ZCommonAlgorithms, DistPointLineVar2Ref_AfterEnd)
{
    const float point[3] = { 3.0f, 0.0f, 0.0f };
    const float lineStart[3] = { 0.0f, 0.0f, 0.0f };
    const float lineEnd[3] = { 2.0f, 0.0f, 0.0f };
    float t, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_TRUE(ZCommonAlgorithms::DistPointLineVar2(point, lineStart, lineEnd, t, minDist, dist, dir));
    EXPECT_TRUE(FloatNear(t, 1.0f));
    EXPECT_TRUE(FloatNear(dist, 1.0f));
}

// -----------------------------------------------------------------------------
// DistLineLineVar (two segments)
// -----------------------------------------------------------------------------

TEST(ZCommonAlgorithms, DistLineLineVar_SkewInterior)
{
    // Line A along X, line B along Z offset by 1 in Y. Closest points are the two midpoints.
    const float a1[3] = { -1.0f, 0.0f, 0.0f };
    const float a2[3] = { 1.0f, 0.0f, 0.0f };
    const float b1[3] = { 0.0f, 1.0f, -1.0f };
    const float b2[3] = { 0.0f, 1.0f, 1.0f };
    const float expectedDir[3] = { 0.0f, -1.0f, 0.0f };
    float s, t, minDist = 10.0f, dist;
    float dir[3];

    EXPECT_TRUE(ZCommonAlgorithms::DistLineLineVar(a1, a2, b1, b2, s, t, minDist, dist, dir));
    EXPECT_TRUE(FloatNear(s, 0.5f));
    EXPECT_TRUE(FloatNear(t, 0.5f));
    EXPECT_TRUE(FloatNear(dist, 1.0f));
    EXPECT_TRUE(Vec3Near(dir, expectedDir));
}

TEST(ZCommonAlgorithms, DistLineLineVar_TooFar)
{
    const float a1[3] = { -1.0f, 0.0f, 0.0f };
    const float a2[3] = { 1.0f, 0.0f, 0.0f };
    const float b1[3] = { 0.0f, 1.0f, -1.0f };
    const float b2[3] = { 0.0f, 1.0f, 1.0f };
    float s, t, minDist = 0.5f, dist;
    float dir[3];

    EXPECT_FALSE(ZCommonAlgorithms::DistLineLineVar(a1, a2, b1, b2, s, t, minDist, dist, dir));
}

// -----------------------------------------------------------------------------
// CollideCapsuleAndTriangle
// -----------------------------------------------------------------------------

namespace
{
    // Builds an SFastBoxColiTri from three vertices, replicating ZFastBoxColi::AddFace.
    SFastBoxColiTri MakeFastBoxTri(const float* v0, const float* v1, const float* v2)
    {
        SFastBoxColiTri tri = {};

        float e01[3];
        float e02[3];
        vsub(e01, v1, v0);
        vsub(e02, v2, v0);

        vcross(tri.m_vTriNorm.Get(), e02, e01);
        vnorm(tri.m_vTriNorm.Get());

        tri.m_avVerts[0] = v0;
        tri.m_avVerts[1] = v1;
        tri.m_avVerts[2] = v2;

        float p1[3];
        float p2[3];
        float p0[3];
        vcross(p1, tri.m_vTriNorm.Get(), e01);
        vcross(p2, e02, tri.m_vTriNorm.Get());
        vadd(p0, p1, p2);
        vscalar(p0, -1.0f);

        vnorm(p0);
        vnorm(p1);
        vnorm(p2);

        tri.m_avEdgePerps[0] = p0;
        tri.m_avEdgePerps[1] = p1;
        tri.m_avEdgePerps[2] = p2;

        return tri;
    }
}

TEST(ZCommonAlgorithms, CollideCapsuleAndTriangle_InteriorHit)
{
    // Triangle in the XY plane.
    const float v0[3] = { 0.0f, 0.0f, 0.0f };
    const float v1[3] = { 2.0f, 0.0f, 0.0f };
    const float v2[3] = { 0.0f, 2.0f, 0.0f };
    const SFastBoxColiTri tri = MakeFastBoxTri(v0, v1, v2);

    // Capsule crossing the triangle interior.
    const float cap0[3] = { 0.4f, 0.4f, 1.0f };
    const float cap1[3] = { 0.4f, 0.4f, -1.0f };
    const float radius = 0.25f;

    SCapsuleColiInfo result;
    const float expectedDir[3] = { 0.0f, 0.0f, -1.0f };

    EXPECT_TRUE(ZCommonAlgorithms::CollideCapsuleAndTriangle(cap0, cap1, radius, &tri, result));
    EXPECT_TRUE(FloatNear(result.t0, 1.0f));
    EXPECT_TRUE(FloatNear(result.fScaledDist, 1.25f));
    EXPECT_TRUE(Vec3Near(result.vDir.Get(), expectedDir));
}

TEST(ZCommonAlgorithms, CollideCapsuleAndTriangle_FaceHit)
{
    const float v0[3] = { 0.0f, 0.0f, 0.0f };
    const float v1[3] = { 2.0f, 0.0f, 0.0f };
    const float v2[3] = { 0.0f, 2.0f, 0.0f };
    const SFastBoxColiTri tri = MakeFastBoxTri(v0, v1, v2);

    // Capsule hovering just above the triangle within the radius.
    const float cap0[3] = { 0.4f, 0.4f, 2.0f };
    const float cap1[3] = { 0.4f, 0.4f, 0.1f };
    const float radius = 0.5f;

    SCapsuleColiInfo result;
    const float expectedDir[3] = { 0.0f, 0.0f, -1.0f };

    EXPECT_TRUE(ZCommonAlgorithms::CollideCapsuleAndTriangle(cap0, cap1, radius, &tri, result));
    EXPECT_TRUE(FloatNear(result.t0, 1.0f));
    EXPECT_TRUE(FloatNear(result.fScaledDist, 0.4f));
    EXPECT_TRUE(Vec3Near(result.vDir.Get(), expectedDir));
}

TEST(ZCommonAlgorithms, CollideCapsuleAndTriangle_Miss)
{
    const float v0[3] = { 0.0f, 0.0f, 0.0f };
    const float v1[3] = { 2.0f, 0.0f, 0.0f };
    const float v2[3] = { 0.0f, 2.0f, 0.0f };
    const SFastBoxColiTri tri = MakeFastBoxTri(v0, v1, v2);

    // Capsule far away from the triangle.
    const float cap0[3] = { 5.0f, 5.0f, 1.0f };
    const float cap1[3] = { 5.0f, 5.0f, -1.0f };
    const float radius = 0.25f;

    SCapsuleColiInfo result;

    EXPECT_FALSE(ZCommonAlgorithms::CollideCapsuleAndTriangle(cap0, cap1, radius, &tri, result));
}
