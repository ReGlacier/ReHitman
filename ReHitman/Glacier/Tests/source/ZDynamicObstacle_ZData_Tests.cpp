#include <Glacier/PF4/ZData.h>
#include <Glacier/PF4/ZDynamicObstacle.h>
#include <Glacier/PF4/ZStaticObstacle.h>
#include <Glacier/PF4/ZMetaNode.h>
#include <Glacier/PF4/ZPath.h>
#include <Glacier/ZUniMemory.h>
#include <gtest/gtest.h>

#include <cmath>
#include <cstring>

using namespace Glacier;
using namespace Glacier::PF4;

namespace
{
    // A ZData configured with a single empty component: just enough structure
    // to exercise the dynamic obstacle list bookkeeping (Add/Remove/Find/Push)
    // without depending on real navmesh corner geometry.
    class ZDataObstacleFixture
    {
    public:
        ZDataObstacleFixture()
        {
            comp.m_Corners = 0;
            comp.m_FirstCorner = 0;
            comp.m_FirstHeightTree = -1;

            data.m_iComponentCount = 1;
            data.m_pComponents = &comp;

            data.m_iComponentVisitedSize = 1;
            data.m_pComponentVisited = static_cast<uint32_t*>(ZUniMemory::Allocate(4));
            std::memset(data.m_pComponentVisited, 0, 4);

            data.m_pDynamicObstacles = static_cast<ZLink**>(ZUniMemory::Allocate(4));
            std::memset(data.m_pDynamicObstacles, 0, 4);
        }

        ZData* operator->() { return &data; }
        ZData& operator*() { return data; }

        ZComponent comp{};
        ZData data;
    };

    ZDynamicObstacle MakeSquareAtOrigin()
    {
        ZDynamicObstacle obstacle;
        const float vDir[3] = { 1.0f, 0.0f, 0.0f };
        obstacle.DirXZ(vDir);
        const float vExtents[2] = { 1.0f, 1.0f };
        obstacle.CreateHullFromBox(vExtents);
        return obstacle;
    }

    ZLocation MakeLocation(float x, float z)
    {
        return ZLocation({ x, 0.0f, z }, 0, 0, true);
    }

    // ZData whose vertex array holds a unit square around the origin. The
    // vertices are stored in world coordinates exactly like the dynamic
    // obstacle hull. The square occupies vertex indices [0,4); offset tests
    // shift m_FirstVertex so reserve five slots for the [1,5) ring.
    class ZDataSquareFixture
    {
    public:
        ZDataSquareFixture()
        {
            data.m_iVertexCount = 5;
            data.m_pVertices = vertices;

            // Square ring [0,4): (1,1),(-1,1),(-1,-1),(1,-1).
            vertices[0].m_kPos.x = 1.0f;   vertices[0].m_kPos.y = 1.0f;   vertices[0].m_fHeight = 0.0f;
            vertices[1].m_kPos.x = -1.0f;  vertices[1].m_kPos.y = 1.0f;   vertices[1].m_fHeight = 0.0f;
            vertices[2].m_kPos.x = -1.0f;  vertices[2].m_kPos.y = -1.0f;  vertices[2].m_fHeight = 0.0f;
            vertices[3].m_kPos.x = 1.0f;   vertices[3].m_kPos.y = -1.0f;  vertices[3].m_fHeight = 0.0f;

            // Offset tests shift m_FirstVertex to 1, giving the ring [1,5):
            // (-1,1),(-1,-1),(1,-1),(1,1) -- the same square, one step later.
            vertices[4] = vertices[0];
        }

        ZData* operator->() { return &data; }
        ZData& operator*() { return data; }

        ZVertex vertices[5];
        ZData data;
    };
}

TEST(ZDataObstacle, AddFindRemoveObstacleInSingleComponent)
{
    ZDataObstacleFixture f;

    ZDynamicObstacle obstacle = MakeSquareAtOrigin();
    ZLocation location = MakeLocation(0.0f, 0.0f);

    f->AddObstacle(&obstacle, location);

    EXPECT_EQ(f->m_ObstacleCount, 1);
    EXPECT_TRUE(f->HasObstacles());
    EXPECT_EQ(obstacle.m_iLinks, 1);
    EXPECT_NE(obstacle.m_Location.Component(), -1);

    ZDynamicObstacle* found[4] = {};
    const int iFound = f->FindObstacles(MakeLocation(0.2f, 0.2f), found, 4);
    EXPECT_EQ(iFound, 1);
    EXPECT_EQ(found[0], &obstacle);

    f->RemoveObstacle(&obstacle);
    EXPECT_EQ(f->m_ObstacleCount, 0);
    EXPECT_FALSE(f->HasObstacles());
    EXPECT_EQ(obstacle.m_iLinks, 0);
    EXPECT_EQ(obstacle.m_Location.Component(), -1);
}

TEST(ZDataObstacle, AddObstacleSkipsAlreadyActive)
{
    ZDataObstacleFixture f;

    ZDynamicObstacle obstacle = MakeSquareAtOrigin();
    ZLocation location = MakeLocation(0.0f, 0.0f);

    f->AddObstacle(&obstacle, location);
    EXPECT_EQ(f->m_ObstacleCount, 1);

    // Adding the same (already registered) obstacle again is a no-op.
    f->AddObstacle(&obstacle, location);
    EXPECT_EQ(f->m_ObstacleCount, 1);
    EXPECT_EQ(obstacle.m_iLinks, 1);
}

TEST(ZDataObstacle, PushOutOfObstaclesMovesPointOutsideHull)
{
    ZDataObstacleFixture f;

    ZDynamicObstacle obstacle = MakeSquareAtOrigin();
    obstacle.m_Type = eSolid;
    f->AddObstacle(&obstacle, MakeLocation(0.0f, 0.0f));

    ZMetaNode node;
    node.m_Location = MakeLocation(0.0f, 0.0f);

    // Point strictly inside the square: (0.25, 0.25).
    ZVector3 pos(0.25f, 0.0f, 0.25f);
    f->PushOutOfObstacles(&node, static_cast<int>(eSolid), pos);

    const float fInside[2] = { pos.x, pos.z };
    EXPECT_FALSE(obstacle.IsInside(fInside));
    // Pushed beyond the square's half-extent (1) along the diagonal.
    EXPECT_GT(std::fabs(pos.x), 1.0f);
    EXPECT_GT(std::fabs(pos.z), 1.0f);
}

TEST(ZDataObstacle, ComponentHeightWithNoHeightTreeIsZero)
{
    ZDataObstacleFixture f;

    // m_FirstHeightTree == -1 means there is no floor plane: height query is 0.
    EXPECT_FLOAT_EQ(f->ComponentHeight(0, 12.0f, -7.0f), 0.0f);
}

TEST(ZStaticObstacle, IntersectHitsSquareEdge)
{
    ZDataSquareFixture f;

    ZStaticObstacle obstacle;
    obstacle.m_FirstVertex = 0;
    obstacle.m_Vertices = 4;

    // Ray along +X from outside the square; entry edge is x = 1.
    const ZVector2 start(5.0f, 0.0f);
    const ZVector2 end(-5.0f, 0.0f);
    float found[2][3] = {};
    float fBest = std::numeric_limits<float>::max();

    fBest = obstacle.Intersect(*f, start, end, 100.0f, fBest, found);

    EXPECT_FLOAT_EQ(fBest, 4.0f);
    // Crossing edge (1,-1) -> (1,1), i.e. x = 1, full height zero.
    EXPECT_FLOAT_EQ(found[0][0], 1.0f);
    EXPECT_FLOAT_EQ(found[0][1], 0.0f);
    EXPECT_FLOAT_EQ(found[0][2], -1.0f);
    EXPECT_FLOAT_EQ(found[1][0], 1.0f);
    EXPECT_FLOAT_EQ(found[1][1], 0.0f);
    EXPECT_FLOAT_EQ(found[1][2], 1.0f);
}

TEST(ZStaticObstacle, IntersectMissReturnsBest)
{
    ZDataSquareFixture f;

    ZStaticObstacle obstacle;
    obstacle.m_FirstVertex = 0;
    obstacle.m_Vertices = 4;

    const ZVector2 start(5.0f, 5.0f);
    const ZVector2 end(6.0f, 6.0f);
    float found[2][3] = {};
    float fBest = 2.5f;

    const float fResult = obstacle.Intersect(*f, start, end, 100.0f, fBest, found);

    EXPECT_FLOAT_EQ(fResult, 2.5f);
    // found untouched
    EXPECT_FLOAT_EQ(found[0][0], 0.0f);
}

TEST(ZStaticObstacle, IntersectRespectsObstacleVertexOffset)
{
    ZDataSquareFixture f;

    // Shift the obstacle to start one vertex later (starts at (-1,1)).
    ZStaticObstacle obstacle;
    obstacle.m_FirstVertex = 1;
    obstacle.m_Vertices = 4;

    const ZVector2 start(5.0f, 0.0f);
    const ZVector2 end(-5.0f, 0.0f);
    float found[2][3] = {};
    float fBest = std::numeric_limits<float>::max();

    const float fResult = obstacle.Intersect(*f, start, end, 100.0f, fBest, found);

    EXPECT_FLOAT_EQ(fResult, 4.0f);
    // The vertex ring now starts one step later but still closes over the same
    // square, so the crossing edge is the same (1,-1) -> (1,1).
    EXPECT_FLOAT_EQ(found[0][0], 1.0f);
    EXPECT_FLOAT_EQ(found[0][2], -1.0f);
    EXPECT_FLOAT_EQ(found[1][0], 1.0f);
    EXPECT_FLOAT_EQ(found[1][2], 1.0f);
}


TEST(ZDataPath, AllocateBufferPathCopiesPathBuffer)
{
    ZData data;

    ZDataRef refs[4];
    for (int i = 0; i < 4; ++i)
    {
        refs[i].m_Id = static_cast<unsigned>(i + 100);
        refs[i].m_Type = static_cast<unsigned>(i % 3);
        refs[i].m_Pos.x = static_cast<float>(i);
        refs[i].m_Pos.y = static_cast<float>(i * 2);
    }

    ZPath src;
    src.m_pathIdx = refs;
    src.m_iMaxSize = 10;
    src.m_Size = 4;
    src.m_Cost = 17.5f;

    ZPath dst;
    const bool bOk = data.AllocateBufferPath(src, &dst);

    EXPECT_TRUE(bOk);
    EXPECT_EQ(dst.m_Size, 4);
    EXPECT_FLOAT_EQ(dst.m_Cost, 17.5f);
    EXPECT_NE(dst.m_pathIdx, refs);
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(dst.m_pathIdx[i].m_Id, refs[i].m_Id);
        EXPECT_EQ(dst.m_pathIdx[i].m_Type, refs[i].m_Type);
        EXPECT_FLOAT_EQ(dst.m_pathIdx[i].m_Pos.x, refs[i].m_Pos.x);
    }
}

TEST(ZDataPath, FreePathReturnsAllocatedBufferAndClearsPath)
{
    ZData data;

    ZDataRef refs[4];
    for (int i = 0; i < 4; ++i)
    {
        refs[i].m_Id = static_cast<unsigned>(i + 10);
        refs[i].m_Type = static_cast<unsigned>(i % 3);
        refs[i].m_Pos.x = static_cast<float>(i);
        refs[i].m_Pos.y = static_cast<float>(i);
    }

    ZPath src;
    src.m_pathIdx = refs;
    src.m_iMaxSize = 10;
    src.m_Size = 3;
    src.m_Cost = 4.0f;

    ZPath dst;
    ASSERT_TRUE(data.AllocateBufferPath(src, &dst));
    EXPECT_NE(dst.m_pathIdx, nullptr);
    EXPECT_EQ(dst.m_Size, 3);

    data.FreePath(&dst);
    EXPECT_EQ(dst.m_pathIdx, nullptr);
    EXPECT_EQ(dst.m_Size, 0);
    EXPECT_FLOAT_EQ(dst.m_Cost, 0.0f);
    EXPECT_EQ(dst.m_CustomVertices, 0);
}

TEST(ZDataPath, TeleportNodeFarWithoutNavMeshKeepsNodeRegistered)
{
    ZDataObstacleFixture f;

    ZMetaNode node;
    node.m_Location = MakeLocation(0.0f, 0.0f);

    // No graph data is configured, so a teleport far away cannot be mapped and
    // must leave the registered node untouched.
    ZVector3 farPos(500.0f, 0.0f, 500.0f);
    EXPECT_FALSE(f->TeleportNode(&node, farPos));
    EXPECT_EQ(node.m_Location.Component(), 0);
}

TEST(ZDataPath, TeleportNodeCloseMovesInPlace)
{
    ZDataObstacleFixture f;

    ZMetaNode node;
    node.m_Location = MakeLocation(0.0f, 0.0f);

    // Within the 100/100 tolerance the node is nudged in place via
    // MoveNodeConstrained and stays registered.
    ZVector3 closePos(5.0f, 0.0f, 5.0f);
    EXPECT_TRUE(f->TeleportNode(&node, closePos));
    EXPECT_EQ(node.m_Location.Component(), 0);
}

TEST(ZDataPath, FindPathWithUnmappedLocationsFailsFast)
{
    ZData data;

    // Unmapped locations (graph == -1) fail before touching any navmesh data.
    ZLocation source;
    ZLocation destination;

    ZDataRef aBuffer[4];
    ZPath path(aBuffer, 4);
    ZPathLink aLinks[200];

    int iLinkCount = 123;
    const bool bFound = data.FindPath(source, destination, path, aLinks, iLinkCount, false, 0);

    EXPECT_FALSE(bFound);
    EXPECT_EQ(iLinkCount, 0);
    EXPECT_EQ(path.m_Size, 0);
}

TEST(ZDataPath, FindPathLengthReturnsZeroAndRestoresPathPtr)
{
    ZData data;

    ZPath dummyPath;

    // Unmapped (graph == -1) locations make the path search fail up front
    // without requiring any loaded navmesh data.
    ZLocation source;
    ZLocation destination;

    ZPathRequest request{};
    request.m_Source = &source;
    request.m_Dest = &destination;
    request.m_Path = &dummyPath;

    const float fCost = data.FindPathLength(&request);

    EXPECT_FLOAT_EQ(fCost, 0.0f);
    EXPECT_EQ(request.m_Path, &dummyPath);
}

TEST(ZDataPath, MapValidLocationFailsWithoutGraphData)
{
    ZDataObstacleFixture f;

    ZVector3 pos(10.0f, 5.0f, -5.0f);
    ZLocation location;
    location.m_Component = 7;
    location.m_Graph = 9;

    const bool bOk = f->MapValidLocation(pos, location);

    EXPECT_FALSE(bOk);
    EXPECT_EQ(location.Component(), 7);
    EXPECT_EQ(location.Graph(), 9);
}

TEST(ZDataPath, FindCornersInGraphEmitsSquareCorners)
{
    ZDataSquareFixture f;

    // Vertex ring: a 200x200 square around the origin, so every edge exceeds
    // the 60-unit threshold; consecutive edges are perpendicular and the cross
    // product Y component is negative, so all four vertices must be reported.
    ZVertex ring[4];
    ring[0].m_kPos.x = 100.0f;   ring[0].m_kPos.y = 100.0f;
    ring[1].m_kPos.x = -100.0f;  ring[1].m_kPos.y = 100.0f;
    ring[2].m_kPos.x = -100.0f;  ring[2].m_kPos.y = -100.0f;
    ring[3].m_kPos.x = 100.0f;   ring[3].m_kPos.y = -100.0f;
    for (int i = 0; i < 4; ++i)
    {
        ring[i].m_fHeight = 0.0f;
    }

    f->m_pVertices = ring;
    f->m_iVertexCount = 4;

    ZGraph graph{};
    graph.m_iVertices = 4;
    graph.m_iExits = 0;
    graph.m_iFirstVertex = 0;

    f->m_pGraphs = &graph;
    f->m_iGraphCount = 1;

    int iMax = 8;
    float aOut[72] = {};
    f->FindCornersInGraph(0, iMax, aOut);

    EXPECT_EQ(iMax, 4);
    // First corner: vertex (100, 100) world position stored as (x, 0, z).
    EXPECT_FLOAT_EQ(aOut[0], 100.0f);
    EXPECT_FLOAT_EQ(aOut[1], 0.0f);
    EXPECT_FLOAT_EQ(aOut[2], 100.0f);
}
