#include <Glacier/Animation/ZHumanState.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>

using namespace Glacier;

namespace
{
    constexpr float kTolerance = 0.00001f;
}

TEST(ZHumanState, BoneIdsReturnsIdsForEachGroup)
{
    std::array<uint16_t, 16> ids {};

    EXPECT_EQ(ZHumanState::BoneIds(0, ids.data()), 1);
    EXPECT_EQ(ids[0], 0);

    ids.fill(0);
    EXPECT_EQ(ZHumanState::BoneIds(1, ids.data()), 3);
    EXPECT_EQ((std::array<uint16_t, 3> { ids[0], ids[1], ids[2] }), (std::array<uint16_t, 3> { 1, 2, 3 }));

    ids.fill(0);
    EXPECT_EQ(ZHumanState::BoneIds(2, ids.data()), 2);
    EXPECT_EQ((std::array<uint16_t, 2> { ids[0], ids[1] }), (std::array<uint16_t, 2> { 5, 4 }));

    ids.fill(0);
    EXPECT_EQ(ZHumanState::BoneIds(7, ids.data()), 15);
    EXPECT_EQ(ids[0], 26);
    EXPECT_EQ(ids[14], 40);

    ids.fill(0);
    EXPECT_EQ(ZHumanState::BoneIds(8, ids.data()), 15);
    EXPECT_EQ(ids[0], 41);
    EXPECT_EQ(ids[14], 55);

    EXPECT_EQ(ZHumanState::BoneIds(9, ids.data()), 0);
}

TEST(ZHumanState, ResetRestoresDefaultPose)
{
    ZHumanState state;
    state.m_Floats[1] = 99.0f;
    state.m_Quats[0].i = 99.0f;

    state.Reset();

    const float invSqrt2 = 1.0f / std::sqrt(2.0f);
    EXPECT_NEAR(state.m_Quats[0].i, invSqrt2, kTolerance);
    EXPECT_NEAR(state.m_Quats[0].j, invSqrt2, kTolerance);
    EXPECT_FLOAT_EQ(state.m_Quats[0].k, 0.0f);
    EXPECT_FLOAT_EQ(state.m_Quats[0].w, 0.0f);
    EXPECT_FLOAT_EQ(state.m_Floats[1], 1.0f);
    EXPECT_FLOAT_EQ(state.m_Floats[3], 30.0f);
    EXPECT_FLOAT_EQ(state.m_Floats[16], -0.2f);
    EXPECT_FLOAT_EQ(state.m_Floats[18], 0.3f);
    EXPECT_FLOAT_EQ(state.m_Floats[20], -0.2f);
    EXPECT_FLOAT_EQ(state.m_Floats[22], -0.3f);
}

TEST(ZHumanState, MirrorSwapsLeftRightMaskBits)
{
    ZHumanState state;

    const int mirrored = state.Mirror(ZHumanState::kMirrorLeftLeg | ZHumanState::kMirrorRightArm | ZHumanState::kMirrorLeftHand);

    EXPECT_EQ(mirrored, ZHumanState::kMirrorRightLeg | ZHumanState::kMirrorLeftArm | ZHumanState::kMirrorRightHand);
}

TEST(ZHumanState, MirrorSwapsHandsWithExpectedNegationPattern)
{
    ZHumanState state;
    for (int i = 0; i < 64; ++i)
    {
        state.m_Floats[i] = static_cast<float>(i + 1);
    }

    state.Mirror(ZHumanState::kMirrorHands);

    EXPECT_FLOAT_EQ(state.m_Floats[24], -45.0f);
    EXPECT_FLOAT_EQ(state.m_Floats[25], 46.0f);
    EXPECT_FLOAT_EQ(state.m_Floats[44], -25.0f);
    EXPECT_FLOAT_EQ(state.m_Floats[45], 26.0f);
}
