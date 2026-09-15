#include <Glacier/IK/ZLNKOBJ.h>

#include <gtest/gtest.h>

namespace Glacier
{
    TEST(ZPoseModel, SingleKeyReturnsNormalizedStrength)
    {
        SPoseKey keys[] = {
            { 0, 128 },
        };
        ZPoseModel model;

        model.SetKeys(keys);
        model.SetSize(1);

        EXPECT_FLOAT_EQ(model.GetWeight(0.0f), 0.5f);
    }

    TEST(ZPoseModel, InterpolatesLinearlyBetweenTwoKeys)
    {
        SPoseKey keys[] = {
            { 0, 0 },
            { 50, 128 },
            { 100, 255 },
        };
        ZPoseModel model;

        model.SetKeys(keys);
        model.SetSize(2);

        EXPECT_FLOAT_EQ(model.GetWeight(1.0f), 0.25f);
    }

    TEST(ZPoseModel, InterpolatesQuadraticBezierFromFirstThreeKeys)
    {
        SPoseKey keys[] = {
            { 0, 0 },
            { 25, 64 },
            { 50, 128 },
            { 75, 255 },
        };
        ZPoseModel model;

        model.SetKeys(keys);
        model.SetSize(3);

        EXPECT_FLOAT_EQ(model.GetWeight(1.0f), 0.25f);
    }

    TEST(ZPoseModel, InterpolatesCubicCatmullRomFromFourNeighbouringKeys)
    {
        SPoseKey keys[] = {
            { 0, 0 },
            { 25, 64 },
            { 50, 128 },
            { 75, 192 },
            { 100, 255 },
            { 125, 255 },
        };
        ZPoseModel model;

        model.SetKeys(keys);
        model.SetSize(5);

        EXPECT_FLOAT_EQ(model.GetWeight(1.5f), 0.375f);
    }

    TEST(ZPoseModel, DeactivatesWhenItReachesLastKey)
    {
        SPoseKey keys[] = {
            { 0, 0 },
            { 25, 64 },
            { 50, 128 },
        };
        ZPoseModel model;

        model.SetKeys(keys);
        model.SetSize(2);

        EXPECT_FLOAT_EQ(model.GetWeight(3.0f), 0.5f);
        EXPECT_FALSE(model.Active());
    }
}
