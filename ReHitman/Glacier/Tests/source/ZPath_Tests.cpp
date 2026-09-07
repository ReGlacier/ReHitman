#include <Glacier/PF4/ZPath.h>
#include <gtest/gtest.h>

#include <vector>

using namespace Glacier;
using namespace Glacier::PF4;

namespace
{
    struct PathFixture
    {
        std::vector<ZDataRef> Buffer{ 200 };

        ZPath Path{ Buffer.data(), 200 };
    };
}

TEST(ZPath, AddVertexAppendsCustomVertexEntry)
{
    PathFixture fixture;
    const float vPos[3] = { 1.0f, 2.0f, 3.0f };

    const ZDataRef ref = fixture.Path.AddVertex(vPos);

    EXPECT_EQ(fixture.Path.m_Size, 1);
    EXPECT_EQ(fixture.Path.m_CustomVertices, 1);
    EXPECT_EQ(fixture.Path.m_Cost, 0.0f);

    EXPECT_EQ(ref.m_Type, 3u);
    EXPECT_EQ(ref.m_Id, 0u);
    EXPECT_FLOAT_EQ(ref.m_Pos.x, 1.0f);
    EXPECT_FLOAT_EQ(ref.m_Pos.y, 3.0f);

    const ZDataRef& stored = fixture.Path.m_pathIdx[0];
    EXPECT_EQ(stored.m_Type, 3u);
    EXPECT_EQ(stored.m_Id, 0u);

    const ZVector3& vertex = fixture.Path.m_Vertices[0];
    EXPECT_FLOAT_EQ(vertex.x, 1.0f);
    EXPECT_FLOAT_EQ(vertex.y, 2.0f);
    EXPECT_FLOAT_EQ(vertex.z, 3.0f);
}

TEST(ZPath, AddVertexAccumulatesCost)
{
    PathFixture fixture;
    const float vFrom[3] = { 0.0f, 0.0f, 0.0f };
    const float vTo[3] = { 3.0f, 0.0f, 4.0f };

    fixture.Path.AddVertex(vFrom);
    EXPECT_EQ(fixture.Path.m_Cost, 0.0f);

    const ZDataRef ref = fixture.Path.AddVertex(vTo);
    EXPECT_EQ(fixture.Path.m_Size, 2);
    EXPECT_EQ(fixture.Path.m_CustomVertices, 2);
    EXPECT_EQ(ref.m_Type, 3u);
    EXPECT_EQ(ref.m_Id, 1u);
    EXPECT_FLOAT_EQ(fixture.Path.m_Cost, 5.0f);
}

TEST(ZPath, AddRefAppendsEncodedEntryAndExtendsCost)
{
    PathFixture fixture;
    const float vFrom[3] = { 0.0f, 0.0f, 0.0f };
    const float vTo[3] = { 3.0f, 0.0f, 4.0f };

    const ZDataRef refFrom = fixture.Path.AddVertex(vFrom);
    fixture.Path.AddVertex(vTo);
    EXPECT_FLOAT_EQ(fixture.Path.m_Cost, 5.0f);

    fixture.Path.AddRef(refFrom);
    EXPECT_EQ(fixture.Path.m_Size, 3);
    EXPECT_FLOAT_EQ(fixture.Path.m_Cost, 10.0f);

    const ZDataRef& stored = fixture.Path.m_pathIdx[2];
    EXPECT_EQ(stored.m_Type, 3u);
    EXPECT_EQ(stored.m_Id, 0u);
    EXPECT_FLOAT_EQ(stored.m_Pos.x, 0.0f);
    EXPECT_FLOAT_EQ(stored.m_Pos.y, 0.0f);
}

TEST(ZPath, AddRefIgnoresFullPath)
{
    PathFixture fixture;
    const float vPos[3] = { 0.0f, 0.0f, 0.0f };
    const ZDataRef ref = fixture.Path.AddVertex(vPos);
    fixture.Path.m_Size = 0; // reset bookkeeping, keep buffer

    for (int i = 0; i < 200; ++i)
    {
        fixture.Path.AddRef(ref);
    }

    EXPECT_EQ(fixture.Path.m_Size, 199);
    fixture.Path.AddRef(ref);
    EXPECT_EQ(fixture.Path.m_Size, 199);
}

TEST(ZPath, CutAtDistanceInterpolatesInsideSegment)
{
    PathFixture fixture;
    const float vFrom[3] = { 0.0f, 0.0f, 0.0f };
    const float vTo[3] = { 3.0f, 0.0f, 4.0f };
    fixture.Path.AddVertex(vFrom);
    fixture.Path.AddVertex(vTo);
    EXPECT_FLOAT_EQ(fixture.Path.m_Cost, 5.0f);

    EXPECT_TRUE(fixture.Path.CutAtDistance(2.5f));
    EXPECT_EQ(fixture.Path.m_Size, 2);
    EXPECT_FLOAT_EQ(fixture.Path.m_Cost, 2.5f);

    ZVector3 end;
    fixture.Path.GetEndPosition(end);
    EXPECT_FLOAT_EQ(end.x, 1.5f);
    EXPECT_FLOAT_EQ(end.z, 2.0f);
}

TEST(ZPath, CutAtDistanceDoesNotExtendBeyondCost)
{
    PathFixture fixture;
    const float vFrom[3] = { 0.0f, 0.0f, 0.0f };
    const float vTo[3] = { 3.0f, 0.0f, 4.0f };
    fixture.Path.AddVertex(vFrom);
    fixture.Path.AddVertex(vTo);

    EXPECT_FALSE(fixture.Path.CutAtDistance(12.0f));
    EXPECT_EQ(fixture.Path.m_Size, 2);
}

TEST(ZPath, CutAtDistanceClampsMicroDistancesToOneEntry)
{
    PathFixture fixture;
    const float vFrom[3] = { 0.0f, 0.0f, 0.0f };
    const float vTo[3] = { 3.0f, 0.0f, 4.0f };
    fixture.Path.AddVertex(vFrom);
    fixture.Path.AddVertex(vTo);

    EXPECT_FALSE(fixture.Path.CutAtDistance(0.5f));
    EXPECT_EQ(fixture.Path.m_Size, 1);
}
