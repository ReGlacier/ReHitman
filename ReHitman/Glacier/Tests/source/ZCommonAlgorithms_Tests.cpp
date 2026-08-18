#include <gtest/gtest.h>
#include <Tests/EngineFixture.h>
#include <Glacier/Fysix/ZCommonAlgorithms.h>
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
