#include <Glacier/Animation/ActiveAnimation.h>
#include <Glacier/Animation/Model.h>
#include <gtest/gtest.h>

#include <cmath>

using namespace Glacier::Animation;

TEST(Model, GetAimFramesUsesVerticalAngleForPoseBlend)
{
    float circle1 = 0.0f;
    float circle2 = 0.0f;
    float blend = 0.0f;

    Model::GetAimFrames(circle1, circle2, blend, 1.0f, 0.0f);

    EXPECT_NEAR(circle1, 1.0f * 0.15915494f * 64.0f + 130.0f, 1e-5f);
    EXPECT_NEAR(circle2, 1.0f * 0.15915494f * 64.0f + 195.0f, 1e-5f);
    EXPECT_FLOAT_EQ(blend, 0.0f);
}

TEST(Model, GetAimFramesClampsAnglesAndBlend)
{
    float circle1 = 0.0f;
    float circle2 = 0.0f;
    float blend = 0.0f;

    Model::GetAimFrames(circle1, circle2, blend, -1.0f, 4.0f);

    EXPECT_FLOAT_EQ(circle1, 65.0f);
    EXPECT_FLOAT_EQ(circle2, 260.0f);
    EXPECT_FLOAT_EQ(blend, 0.0f);
}

TEST(Model, DepackOrderSortsActiveAnimationsByPriority)
{
    Model model;
    model.m_ActiveAnims[0].mode = 1 | 0x10000;
    model.m_ActiveAnims[1].mode = 1 | 0x30000;
    model.m_ActiveAnims[2].mode = 0;
    model.m_ActiveAnims[3].mode = 1 | 0x20000;

    uint8_t order[4] = {};
    EXPECT_EQ(model.DepackOrder(order), 3);
    EXPECT_EQ(order[0], 0);
    EXPECT_EQ(order[1], 3);
    EXPECT_EQ(order[2], 1);
}

TEST(ActiveAnimation, ConstructorAndCreateResetRuntimeState)
{
    ActiveAnimation animation;
    EXPECT_EQ(animation.mode, 0x4000);
    EXPECT_FLOAT_EQ(animation.m_fRemCallBackFrame, -1.0f);
    EXPECT_EQ(animation.m_prtCallBacks, nullptr);
    EXPECT_EQ(animation.m_pLnkObj, nullptr);
    EXPECT_EQ(animation.sequenceId, 0);

    animation.SetCurrentFrame(12.5f);
    EXPECT_FLOAT_EQ(animation.frame, 12.5f);
    EXPECT_FLOAT_EQ(animation.m_fRemCallBackFrame, -1.0f);

    animation.Create(nullptr);
    EXPECT_FLOAT_EQ(animation.m_fRemCallBackFrame, -1.0f);
    EXPECT_FLOAT_EQ(animation.frame, 12.5f);
    EXPECT_EQ(animation.sequenceId, 0);
}
