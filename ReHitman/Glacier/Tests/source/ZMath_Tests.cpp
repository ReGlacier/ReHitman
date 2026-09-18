#include <gtest/gtest.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cmath>

using namespace Glacier;

// Identity rotation in Glacier storage: rows are Z[0..2], Y[3..5], X[6..8].
static const float kIdentity[9] = {
    0.0f, 0.0f, 1.0f, // Z axis (0,0,1)
    0.0f, 1.0f, 0.0f, // Y axis (0,1,0)
    1.0f, 0.0f, 0.0f  // X axis (1,0,0)
};

static const float kOrigin[3] = { 0.0f, 0.0f, 0.0f };
static const float kUnitExtents[3] = { 1.0f, 1.0f, 1.0f };

// 90 degrees around Z: X -> (0,1,0), Y -> (-1,0,0), Z -> (0,0,1).
static const float kRotZ90[9] = {
    0.0f, 0.0f, 1.0f,  // Z axis (0,0,1)
    -1.0f, 0.0f, 0.0f, // Y axis (-1,0,0)
    0.0f, 1.0f, 0.0f   // X axis (0,1,0)
};

TEST(ZMath, rectBoxColi_IdentityOverlap)
{
    EXPECT_TRUE(rectBoxColi(kIdentity, kOrigin, kUnitExtents, kIdentity, kOrigin, kUnitExtents));
}

TEST(ZMath, rectBoxColi_SeparatedX)
{
    const float pos[3] = { 3.0f, 0.0f, 0.0f };

    EXPECT_FALSE(rectBoxColi(kIdentity, kOrigin, kUnitExtents, kIdentity, pos, kUnitExtents));
}

TEST(ZMath, rectBoxColi_TouchingX)
{
    // 2.0 == 1.0 + 1.0, faces exactly touch.
    const float pos[3] = { 2.0f, 0.0f, 0.0f };

    EXPECT_TRUE(rectBoxColi(kIdentity, kOrigin, kUnitExtents, kIdentity, pos, kUnitExtents));
}

TEST(ZMath, rectBoxColi_SeparatedY)
{
    const float pos[3] = { 0.0f, 3.0f, 0.0f };

    EXPECT_FALSE(rectBoxColi(kIdentity, kOrigin, kUnitExtents, kIdentity, pos, kUnitExtents));
}

TEST(ZMath, rectBoxColi_SeparatedZ)
{
    const float pos[3] = { 0.0f, 0.0f, 3.0f };

    EXPECT_FALSE(rectBoxColi(kIdentity, kOrigin, kUnitExtents, kIdentity, pos, kUnitExtents));
}

TEST(ZMath, rectBoxColi_OverlapAlongDiagonal)
{
    // Overlapping corners: |t| = (0.5, 0.5, 0.5), sum of half extents = (2, 2, 2).
    const float pos[3] = { 0.5f, 0.5f, 0.5f };

    EXPECT_TRUE(rectBoxColi(kIdentity, kOrigin, kUnitExtents, kIdentity, pos, kUnitExtents));
}

TEST(ZMath, rectBoxColi_Rotated90Z_Overlap)
{
    // Same center, any rotation must overlap.
    EXPECT_TRUE(rectBoxColi(kIdentity, kOrigin, kUnitExtents, kRotZ90, kOrigin, kUnitExtents));
}

TEST(ZMath, rectBoxColi_Rotated90Z_SeparatedX)
{
    // Box2 rotated 90 around Z: its world X extent is its local Y extent (1.0).
    // Sum of extents along X is 2.0, so distance 2.5 is a clear miss.
    const float pos[3] = { 2.5f, 0.0f, 0.0f };

    EXPECT_FALSE(rectBoxColi(kIdentity, kOrigin, kUnitExtents, kRotZ90, pos, kUnitExtents));
}

TEST(ZMath, rectBoxColi_Rotated90Z_OverlapX)
{
    // Distance 1.5 < 2.0 must hit even though box2 is rotated.
    const float pos[3] = { 1.5f, 0.0f, 0.0f };

    EXPECT_TRUE(rectBoxColi(kIdentity, kOrigin, kUnitExtents, kRotZ90, pos, kUnitExtents));
}

TEST(ZMath, rectBoxColi_DifferentExtents)
{
    const float big[3] = { 2.0f, 2.0f, 2.0f };
    const float pos[3] = { 2.5f, 0.0f, 0.0f };

    // 2.5 < 2.0 + 1.0 => hit.
    EXPECT_TRUE(rectBoxColi(kIdentity, kOrigin, big, kIdentity, pos, kUnitExtents));

    const float farPos[3] = { 3.5f, 0.0f, 0.0f };

    // 3.5 > 2.0 + 1.0 => miss.
    EXPECT_FALSE(rectBoxColi(kIdentity, kOrigin, big, kIdentity, farPos, kUnitExtents));
}

TEST(ZMath, rectBoxColi_Symmetric)
{
    const float pos[3] = { 1.5f, 0.25f, -0.5f };

    const bool forward = rectBoxColi(kIdentity, kOrigin, kUnitExtents, kRotZ90, pos, kUnitExtents);
    const bool reverse = rectBoxColi(kRotZ90, pos, kUnitExtents, kIdentity, kOrigin, kUnitExtents);

    EXPECT_EQ(forward, reverse);
}
