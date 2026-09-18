#include <Glacier/ZSTL/TIMETYPE.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    static_assert(sizeof(TIMETYPE) == 0x4);
}

TEST(TIMETYPE, DefaultConstructorInitializesZero)
{
    TIMETYPE time;

    EXPECT_EQ(time.secs, 0);
    EXPECT_EQ(static_cast<int>(time), 0);
    EXPECT_FLOAT_EQ(static_cast<float>(time), 0.0f);
    EXPECT_FALSE(static_cast<bool>(time));
}

TEST(TIMETYPE, IntConstructorStoresWholeSecondsAsFixedPointTicks)
{
    TIMETYPE time(3);

    EXPECT_EQ(time.secs, 3 << 10);
    EXPECT_EQ(static_cast<int>(time), 3);
    EXPECT_FLOAT_EQ(static_cast<float>(time), 3.0f);
    EXPECT_TRUE(static_cast<bool>(time));
}

TEST(TIMETYPE, FloatConstructorStoresFractionalSecondsAsFixedPointTicks)
{
    TIMETYPE time(1.5f);

    EXPECT_EQ(time.secs, 1536);
    EXPECT_EQ(static_cast<int>(time), 1);
    EXPECT_FLOAT_EQ(static_cast<float>(time), 1.5f);
}

TEST(TIMETYPE, ConvertTruncatesFloatToFixedPointTicks)
{
    TIMETYPE time;

    EXPECT_EQ(time.convert(0.001f), 1);
    EXPECT_EQ(time.convert(1.9999f), static_cast<int>(1.9999f * TIMETYPE::kTicksPerSecond));
    EXPECT_EQ(time.convert(-1.5f), -1536);
}

TEST(TIMETYPE, CopyConstructorAndAssignmentCopyRawTicks)
{
    TIMETYPE source(2.25f);
    TIMETYPE copied(source);
    TIMETYPE assigned;

    assigned = source;

    EXPECT_EQ(copied.secs, source.secs);
    EXPECT_EQ(assigned.secs, source.secs);
}

TEST(TIMETYPE, SelfAssignmentKeepsValue)
{
    TIMETYPE time(4);
    TIMETYPE* same = &time;

    time = *same;

    EXPECT_EQ(time.secs, 4 << 10);
}
