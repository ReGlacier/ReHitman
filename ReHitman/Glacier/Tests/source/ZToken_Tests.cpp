#include <Glacier/Serializer/ZToken.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZToken) == 0x4);
}

TEST(ZToken, DefaultConstructorCreatesVoidToken)
{
    ZToken token;

    EXPECT_EQ(token.m_Token, -1);
    EXPECT_EQ(token, ZToken::Void);
}

TEST(ZToken, StaticSentinelsMatchOriginalValues)
{
    EXPECT_EQ(static_cast<int32_t>(ZToken::Void), -1);
    EXPECT_EQ(static_cast<int32_t>(ZToken::Unknown), -2);
    EXPECT_EQ(static_cast<int32_t>(ZToken::Joker), -3);
}

TEST(ZToken, ComparisonsUseRawTokenValue)
{
    ZToken one(1);
    ZToken two(2);

    EXPECT_LT(one, two);
    EXPECT_LE(one, two);
    EXPECT_GT(two, one);
    EXPECT_TRUE(one == ZToken(1));
}

TEST(ZToken, AssignmentIncrementAndIntConversion)
{
    ZToken token(4);
    ZToken other;

    other = token;
    ++other;

    EXPECT_EQ(static_cast<int32_t>(other), 5);
}
