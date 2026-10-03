#include <Glacier/Geom/ZSHAPE.h>
#include <Glacier/SSplineMover.h>
#include <Glacier/ZUniMemory.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    // Builds a straight line spline shape:
    //   segment 0: (0,0,0) -> (1,0,0)
    //   segment 1: (1,0,0) -> (2,0,0)
    void MakeStraightShape(BasicShape& shape, ParametricCurve* pCurves)
    {
        float aVertices[7][3] = {
            { 0.0f, 0.0f, 0.0f },
            { 1.0f / 3.0f, 0.0f, 0.0f },
            { 2.0f / 3.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f, 0.0f },
            { 4.0f / 3.0f, 0.0f, 0.0f },
            { 5.0f / 3.0f, 0.0f, 0.0f },
            { 2.0f, 0.0f, 0.0f },
        };

        shape.DefineBezierSpline(pCurves, aVertices, 2, 0.1f);
    }

    // Builds a spline with uneven segments: length 1 and length 3.
    void MakeUnevenShape(BasicShape& shape, ParametricCurve* pCurves)
    {
        float aVertices[7][3] = {
            { 0.0f, 0.0f, 0.0f },
            { 1.0f / 3.0f, 0.0f, 0.0f },
            { 2.0f / 3.0f, 0.0f, 0.0f },
            { 1.0f, 0.0f, 0.0f },
            { 2.0f, 0.0f, 0.0f },
            { 3.0f, 0.0f, 0.0f },
            { 4.0f, 0.0f, 0.0f },
        };

        shape.DefineBezierSpline(pCurves, aVertices, 2, 0.1f);
    }

    TEST(ParametricCurveTest, Constructor)
    {
        ParametricCurve curve;
        EXPECT_EQ(curve.m_fLen, -1.0f);
    }

    TEST(ParametricCurveTest, DefineBezierStraightLine)
    {
        ParametricCurve curve;

        const float p0[3] = { 0.0f, 0.0f, 0.0f };
        const float p1[3] = { 1.0f / 3.0f, 0.0f, 0.0f };
        const float p2[3] = { 2.0f / 3.0f, 0.0f, 0.0f };
        const float p3[3] = { 1.0f, 0.0f, 0.0f };

        curve.DefineBezier(&p0, &p1, &p2, &p3, 0.1f);

        EXPECT_NEAR(curve.GetLen(), 1.0f, 0.001f);

        float pPos[3];
        curve.GetSplinePos(&pPos, 0.25f);
        EXPECT_NEAR(pPos[0], 0.25f, 0.001f);
        EXPECT_NEAR(pPos[1], 0.0f, 0.001f);
        EXPECT_NEAR(pPos[2], 0.0f, 0.001f);

        float pVel[3];
        curve.GetSplineVelocity(&pVel, 0.5f);
        EXPECT_NEAR(pVel[0], 1.0f, 0.001f);
        EXPECT_NEAR(pVel[1], 0.0f, 0.001f);
        EXPECT_NEAR(pVel[2], 0.0f, 0.001f);
    }

    TEST(ParametricCurveTest, GetSplinePosAtEndpoints)
    {
        ParametricCurve curve;

        const float p0[3] = { 0.0f, 0.0f, 0.0f };
        const float p1[3] = { 1.0f, 2.0f, 0.0f };
        const float p2[3] = { 2.0f, 3.0f, 0.0f };
        const float p3[3] = { 3.0f, 4.0f, 0.0f };

        curve.DefineBezier(&p0, &p1, &p2, &p3, 0.1f);

        float pPos[3];
        curve.GetSplinePos(&pPos, 0.0f);
        EXPECT_NEAR(pPos[0], 0.0f, 0.001f);
        EXPECT_NEAR(pPos[1], 0.0f, 0.001f);

        curve.GetSplinePos(&pPos, 1.0f);
        EXPECT_NEAR(pPos[0], 3.0f, 0.001f);
        EXPECT_NEAR(pPos[1], 4.0f, 0.001f);
    }

    TEST(BasicShapeTest, Constructor)
    {
        BasicShape shape;
        EXPECT_EQ(shape.m_fLen, -1.0f);
        EXPECT_EQ(shape.m_fMinSegLen, 9.9999997e37f);
        EXPECT_EQ(shape.m_pSegments, nullptr);
        EXPECT_EQ(shape.m_lNrSegment, 0);
    }

    TEST(BasicShapeTest, DefineBezierSpline)
    {
        ParametricCurve aCurves[2];
        BasicShape shape;

        MakeStraightShape(shape, aCurves);

        EXPECT_EQ(shape.m_lNrSegment, 2);
        EXPECT_EQ(shape.m_pSegments, aCurves);
        EXPECT_NEAR(shape.m_fLen, 2.0f, 0.001f);
        EXPECT_NEAR(shape.m_fMinSegLen, 1.0f, 0.001f);
        EXPECT_NEAR(shape.GetLen(), 2.0f, 0.001f);
        EXPECT_NEAR(shape.CalcShapeLen(), 2.0f, 0.001f);
    }

    TEST(BasicShapeTest, GetLenComputesOnDemand)
    {
        BasicShape shape;
        EXPECT_EQ(shape.m_fLen, -1.0f);

        // A shape with no segments yields zero length once computed.
        EXPECT_NEAR(shape.GetLen(), 0.0f, 0.001f);
        EXPECT_EQ(shape.m_fLen, 0.0f);
    }

    TEST(BasicShapeTest, GetSplinePosAndVelocity)
    {
        ParametricCurve aCurves[2];
        BasicShape shape;
        MakeStraightShape(shape, aCurves);

        float pPos[3];
        shape.GetSplinePos(&pPos, 0.25f);
        EXPECT_NEAR(pPos[0], 0.25f, 0.001f);
        EXPECT_NEAR(pPos[1], 0.0f, 0.001f);

        shape.GetSplinePos(&pPos, 1.25f);
        EXPECT_NEAR(pPos[0], 2.25f, 0.001f);

        float pVel[3];
        shape.GetSplineVelocity(&pVel, 0.5f);
        EXPECT_NEAR(pVel[0], 1.0f, 0.001f);
        EXPECT_NEAR(pVel[1], 0.0f, 0.001f);

        float pPos2[3];
        shape.GetSplinePosVelocity(&pPos2, &pVel, 1.5f);
        EXPECT_NEAR(pPos2[0], 2.5f, 0.001f);
        EXPECT_NEAR(pVel[0], 1.0f, 0.001f);
    }

    TEST(BasicShapeTest, IsNormalizedWithEqualSegments)
    {
        ParametricCurve aCurves[2];
        BasicShape shape;
        MakeStraightShape(shape, aCurves);

        EXPECT_TRUE(shape.IsNormalized());
    }

    TEST(BasicShapeTest, IsNormalizedWithUnevenSegments)
    {
        ParametricCurve aCurves[2];
        BasicShape shape;
        MakeUnevenShape(shape, aCurves);

        EXPECT_FALSE(shape.IsNormalized());
    }

    TEST(BasicShapeTest, SaveFreesSegments)
    {
        BasicShape shape;
        shape.m_pSegments = static_cast<ParametricCurve*>(ZUniMemory::Allocate(sizeof(ParametricCurve) * 2));
        shape.m_lNrSegment = 2;

        EXPECT_TRUE(shape.Save(nullptr));
        EXPECT_EQ(shape.m_pSegments, nullptr);
    }
}
