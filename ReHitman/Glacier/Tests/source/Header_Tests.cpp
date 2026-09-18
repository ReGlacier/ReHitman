#include <Glacier/Animation/Header.h>
#include <Glacier/Animation/Manager.h>
#include <gtest/gtest.h>

using namespace Glacier::Animation;

TEST(AnimationHeader, MissingCompressedGroundReturnsIdentity)
{
    Manager manager;
    Header header{};
    header.m_Frames = 1;
    header.m_Mask = 0;
    header.m_GroundOffset = -1;

    float quat[4] = { 1.0f, 2.0f, 3.0f, 4.0f };
    float position[3] = { 5.0f, 6.0f, 7.0f };
    header.GetGround(&manager, 0.0f, quat, position, false);

    EXPECT_FLOAT_EQ(quat[0], 0.0f);
    EXPECT_FLOAT_EQ(quat[1], 0.0f);
    EXPECT_FLOAT_EQ(quat[2], 0.0f);
    EXPECT_FLOAT_EQ(quat[3], 1.0f);
    EXPECT_FLOAT_EQ(position[0], 0.0f);
    EXPECT_FLOAT_EQ(position[1], 0.0f);
    EXPECT_FLOAT_EQ(position[2], 0.0f);
}
