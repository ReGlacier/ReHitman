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

TEST(Model, GetAimFramesSelectsEachVerticalBand)
{
    struct TestCase
    {
        float vertical;
        float circle1Offset;
        float circle2Offset;
        float expectedBlend;
    };

    constexpr TestCase cases[] = {
        { 0.5f, 130.0f, 195.0f, (0.5f - 0.2617994f) * 1.9098593f },
        { 1.0f, 0.0f, 130.0f, (1.0f - 0.78539819f) * 1.2732395f },
        { 2.0f, 0.0f, 65.0f, 1.0f - (2.0f - 1.5707964f) * 1.2732395f },
        { 2.5f, 65.0f, 260.0f, 1.0f - (2.5f - 2.3561945f) * 1.9098593f },
    };

    constexpr float horizontal = 2.0f;
    constexpr float baseCircle = horizontal * 0.15915494f * 64.0f;

    for (const TestCase& testCase : cases)
    {
        float circle1 = 0.0f;
        float circle2 = 0.0f;
        float blend = 0.0f;

        Model::GetAimFrames(circle1, circle2, blend, horizontal, testCase.vertical);

        EXPECT_NEAR(circle1, baseCircle + testCase.circle1Offset, 1e-5f);
        EXPECT_NEAR(circle2, baseCircle + testCase.circle2Offset, 1e-5f);
        EXPECT_NEAR(blend, testCase.expectedBlend, 1e-5f);
    }
}

TEST(Model, GetAimFramesClampsHorizontalAngle)
{
    float circle1 = 0.0f;
    float circle2 = 0.0f;
    float blend = 0.0f;

    Model::GetAimFrames(circle1, circle2, blend, 10.0f, 1.5707964f);

    EXPECT_NEAR(circle1, 64.0f, 1e-4f);
    EXPECT_NEAR(circle2, 194.0f, 1e-4f);
    EXPECT_FLOAT_EQ(blend, 1.0f);
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

TEST(Model, DepackOrderSkipsSecondHalfOfDualAnimation)
{
    Model model;
    model.m_ActiveAnims[0].mode = 1 | 8 | 0x20000;
    model.m_ActiveAnims[1].mode = 1 | 0x10000;
    model.m_ActiveAnims[2].mode = 1 | 0x30000;
    model.m_ActiveAnims[3].mode = 0;

    uint8_t order[4] = {};
    EXPECT_EQ(model.DepackOrder(order), 2);
    EXPECT_EQ(order[0], 0);
    EXPECT_EQ(order[1], 2);
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
