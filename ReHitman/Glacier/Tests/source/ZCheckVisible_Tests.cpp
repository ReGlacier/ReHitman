#include <Glacier/GameBase/ZCheckVisible.h>
#include <Glacier/GameBase/ZSeeable.h>
#include <Glacier/GameBase/SSeer.h>
#include <Glacier/GameBase/SSeerObject.h>
#include <Glacier/GameBase/Locator.h>
#include <Glacier/GameBase/ZCellBitmap.h>
#include <Glacier/GameBase/ZCellCoordinate.h>
#include <Glacier/RTP/Base.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace Glacier;

namespace
{
    std::vector<std::string> CollectPropertyNames(const RTP::ZPropertyInfo& info)
    {
        std::vector<std::string> names;
        for (const RTP::cNode* node = info.First; node; node = node->m_Next)
        {
            names.emplace_back(node->m_Name ? node->m_Name : "");
        }
        return names;
    }
}

TEST(ZCheckVisible, PropertyTableMatchesBinary)
{
    const RTP::ZPropertyInfo& info = ZCheckVisible::Info;
    EXPECT_EQ(info.Super, &ZEventBase::Info);
    EXPECT_STREQ(info.Name, "CheckVisible");
    EXPECT_EQ(CollectPropertyNames(info), (std::vector<std::string>{
        "m_fVisionDistMultiplier",
        "m_iNumSeers",
        "m_iNumSeeables",
        "m_iHighestSeeableDecalID",
        "m_fDeltaTimeFraction",
    }));
}

TEST(ZSeeable, TracksExistenceAndDecalState)
{
    ZSeeable seeable;
    EXPECT_FALSE(seeable.Exists());
    EXPECT_FALSE(seeable.IsDecal());

    seeable.pSeeable = reinterpret_cast<ZGEOM*>(0x1234);
    seeable.type = Vision::ITEM;
    EXPECT_TRUE(seeable.Exists());
    EXPECT_FALSE(seeable.IsDecal());

    seeable.type = Vision::GROUNDDECAL;
    EXPECT_TRUE(seeable.IsDecal());
}

TEST(SSeerObject, VisibilityAndDeltaTimeSharePackedStorage)
{
    SSeerObject object;
    object.SetVisible(true);
    object.SetDeltaTime(42);

    EXPECT_TRUE(object.GetVisible());
    EXPECT_EQ(object.GetDeltaTime(), 42);

    object.SetVisible(false);
    EXPECT_FALSE(object.GetVisible());
    EXPECT_EQ(object.GetDeltaTime(), 42);
}

TEST(SSeer, CalculateTimeElapsedAccumulatesFractionAndClamps)
{
    SSeer seer;
    seer.ttLastTimeChecked = TIMETYPE(0);

    TIMETYPE fraction(0.016f);
    EXPECT_EQ(seer.CalculateTimeElapsed(fraction), 0);
    EXPECT_GT(seer.fLastTimeFraction, 0.0f);

    TIMETYPE later(0.5f);
    EXPECT_EQ(seer.CalculateTimeElapsed(later), 30);

    TIMETYPE farFuture(3.0f);
    EXPECT_EQ(seer.CalculateTimeElapsed(farFuture), 127);
}

TEST(ZCellCoordinate, ValidatesShiftsAndInclusiveRanges)
{
    EXPECT_TRUE(ZCellCoordinate(0, 0, 0).IsValid());
    EXPECT_TRUE(ZCellCoordinate(15, 0, 15).IsValid());
    EXPECT_FALSE(ZCellCoordinate(-1, 0, 0).IsValid());
    EXPECT_FALSE(ZCellCoordinate(0, -1, 0).IsValid());
    EXPECT_FALSE(ZCellCoordinate(0, 0, -1).IsValid());
    EXPECT_FALSE(ZCellCoordinate(16, 0, 0).IsValid());

    EXPECT_EQ(ZCellCoordinate(0, 0, 15).Shift(-1, 1, 1), ZCellCoordinate(0, 0, 15));
    EXPECT_TRUE(ZCellCoordinate(3, 0, 4).IsWithin(ZCellCoordinate(3, 0, 4), ZCellCoordinate(3, 0, 4)));
}

TEST(Locator, AssignmentCopiesCoordinateAndIndex)
{
    Locator destination(ZCellCoordinate(1, 0, 2), 3);
    const Locator source(ZCellCoordinate(12, 0, 14), 62);
    destination = source;

    EXPECT_EQ(destination.m_X, 12);
    EXPECT_EQ(destination.m_Y, 0);
    EXPECT_EQ(destination.m_Z, 14);
    EXPECT_EQ(destination.GetCellIndex(), 62);
    EXPECT_TRUE(destination.IsValid());
    EXPECT_FALSE(Locator::Invalid.IsValid());
}

TEST(ZCellBitmap, MapsBoundaryBitsAndCopiesState)
{
    ZCellBitmap bitmap;
    ZCellCoordinate low(0, 0, 0);
    ZCellCoordinate wordBoundary(15, 0, 1);
    ZCellCoordinate high(15, 0, 15);

    bitmap.Set(low);
    bitmap.Set(wordBoundary);
    bitmap.Set(high);
    EXPECT_EQ(bitmap.m_SeerCells[0], 0x80000001u);
    EXPECT_EQ(bitmap.m_SeerCells[7], 0x80000000u);

    ZCellBitmap copy(bitmap);
    copy.Clear(wordBoundary);
    EXPECT_TRUE(copy.IsSet(low));
    EXPECT_FALSE(copy.IsSet(wordBoundary));
    EXPECT_TRUE(copy.IsSet(high));
    EXPECT_TRUE(bitmap.IsSet(wordBoundary));
}

TEST(ZSeeable, DecrementsValidLocatorIndex)
{
    ZSeeable seeable;
    seeable.locator = Locator(ZCellCoordinate(5, 0, 6), 2);
    seeable.DecrementLocatorIndex();
    EXPECT_EQ(seeable.locator.GetCellIndex(), 1);
}

TEST(SSeerObject, SupportsPackedBoundaryValues)
{
    SSeerObject object;
    object.lValue = 0x2A;
    object.SetVisible(true);
    EXPECT_EQ(object.lValue, 0xAA);
    object.SetDeltaTime(127);
    EXPECT_EQ(object.lValue, 0xFF);
    object.SetVisible(false);
    EXPECT_EQ(object.lValue, 0x7F);

    const SSeerObject& constObject = object;
    EXPECT_EQ(constObject.GetDeltaTime(), 127);
}
