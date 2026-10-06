#include <Glacier/ZSTL/CObjectInfo.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    static_assert(sizeof(CObjectInfo) == 0x10);
}

TEST(CObjectInfo, IsOverlapAABBUsesHalfOpenBounds)
{
    CObjectInfo object{};
    object.iID = 1;
    object.iMinX = 10;
    object.iMinY = 20;
    object.iMinZ = 30;
    object.iMaxX = 20;
    object.iMaxY = 30;
    object.iMaxZ = 40;

    EXPECT_TRUE(object.IsOverlapAABB(15, 25, 35, 25, 35, 45));
    EXPECT_TRUE(object.IsOverlapAABB(0, 0, 0, 11, 21, 31));

    EXPECT_FALSE(object.IsOverlapAABB(20, 20, 30, 25, 30, 40));
    EXPECT_FALSE(object.IsOverlapAABB(0, 0, 0, 10, 30, 40));
    EXPECT_FALSE(object.IsOverlapAABB(10, 30, 30, 20, 40, 40));
    EXPECT_FALSE(object.IsOverlapAABB(10, 20, 40, 20, 30, 50));
}
