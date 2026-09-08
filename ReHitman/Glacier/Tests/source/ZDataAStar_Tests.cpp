#include <Glacier/PF4/ZData.h>
#include <Glacier/PF4/ZGraph.h>
#include <Glacier/PF4/ZOpenNode.h>
#include <Glacier/PF4/ZOpenNodeList.h>
#include <Glacier/PF4/ZPath.h>
#include <Glacier/ZUniMemory.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstring>

using namespace Glacier;
using namespace Glacier::PF4;

namespace
{
    // ZData with a couple of navmesh nodes/vertices so the straightening helpers
    // have valid geometry to read. No game data is involved: the mesh is a simple
    // synthetic corridor.
    class ZDataGeometryFixture
    {
    public:
        ZDataGeometryFixture()
        {
            data.m_iVertexCount = 6;
            data.m_pVertices = vertices;
            data.m_iNodeCount = 0;
            data.m_pNodes = nullptr;
            data.m_iLinkCount = 0;
            data.m_pLinks = nullptr;

            // Three vertical "gates" across a corridor running along +X. Each gate
            // k uses vertices (2k, 2k+1) at (x, z=-6) and (x, z=+6).
            const float fX[3] = { 8.0f, 16.0f, 24.0f };
            for (int k = 0; k < 3; ++k)
            {
                vertices[2 * k].m_kPos.x = fX[k];
                vertices[2 * k].m_kPos.y = -6.0f;
                vertices[2 * k].m_fHeight = 0.0f;
                vertices[2 * k + 1].m_kPos.x = fX[k];
                vertices[2 * k + 1].m_kPos.y = 6.0f;
                vertices[2 * k + 1].m_fHeight = 0.0f;
            }
        }

        ZData* operator->() { return &data; }
        ZData& operator*() { return data; }

        ZVertex vertices[6];
        ZData data;
    };
}

TEST(ZOpenNodeList, DecreaseKeyAndExtractMinSortByKey)
{
    ZNodeData nodes[8];
    ZOpenNodeList list;
    list.SetNodeData(nodes, 8);

    list.DecreaseKey(-1, 3, 0, 0, 0, 4.0f, 1.0f);
    list.DecreaseKey(-1, 1, 0, 0, 0, 2.0f, 0.5f);
    list.DecreaseKey(-1, 5, 0, 0, 0, 6.0f, 2.0f);
    list.DecreaseKey(-1, 0, 0, 0, 0, 1.0f, 0.25f);

    EXPECT_EQ(list.GetKey(3), 4.0f);
    EXPECT_EQ(list.m_iOpenNodeCount, 4);

    EXPECT_EQ(list.ExtractMin(), 0);
    EXPECT_EQ(list.ExtractMin(), 1);
    EXPECT_EQ(list.ExtractMin(), 3);
    EXPECT_EQ(list.ExtractMin(), 5);
    EXPECT_EQ(list.ExtractMin(), -1);
}

TEST(ZOpenNodeList, DecreaseKeyImprovesExistingEntry)
{
    ZNodeData nodes[8];
    ZOpenNodeList list;
    list.SetNodeData(nodes, 8);

    list.DecreaseKey(-1, 2, 0, 0, 0, 9.0f, 1.0f);
    list.DecreaseKey(-1, 2, 0, 0, 0, 3.0f, 0.5f);

    EXPECT_EQ(list.m_iOpenNodeCount, 1);
    EXPECT_EQ(list.GetKey(2), 3.0f);
    EXPECT_EQ(list.GetCost(2), 0.5f);
}

TEST(ZOpenNodeList, ExtractMinKeepsClosedRecordAndResetClears)
{
    ZNodeData nodes[4];
    ZOpenNodeList list;
    list.SetNodeData(nodes, 4);

    list.DecreaseKey(-1, 0, 0, 1, 2, 5.0f, 1.0f);
    list.DecreaseKey(-1, 1, 0, 0, 0, 7.0f, 2.0f);
    list.DecreaseKey(-1, 2, 0, 0, 0, 6.0f, 3.0f);

    EXPECT_EQ(list.ExtractMin(), 0);
    // The popped entry survives in the closed tail so costs stay readable; the
    // engine marks it visited explicitly right after extraction.
    list.SetVisited(0, true);
    EXPECT_FLOAT_EQ(list.GetCost(0), 1.0f);
    EXPECT_EQ(list.GetVisited(0), true);

    list.ResetUsedNodeData();
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(nodes[i].iOpenNode, -1);
    }
    EXPECT_EQ(list.m_iOpenNodeCount, 0);
    EXPECT_EQ(list.m_iClosedNodeCount, 0);
}

TEST(ZDataStraightenGates, StraightCorridorProducesSanePath)
{
    ZDataGeometryFixture f;

    const float fStart[3] = { 0.0f, 0.0f, 0.0f };
    const float fEnd[3] = { 32.0f, 0.0f, 0.0f };

    // Three gate records across the corridor (record[0] unused, like the engine).
    int aGates[9] = { 0, 0, 1, 0, 2, 3, 0, 4, 5 };

    ZDataRef aRefs[512];
    ZPath path(aRefs, 512);
    path.m_PathFinder = &f.data; // needed by ZPath::AddRef to resolve refs
    ZPathLink aLinks[64];
    std::memset(aLinks, 0, sizeof(aLinks));

    const int iLinks = f->StraightenGates(fStart, fEnd, aGates, 3, aLinks, path);

    EXPECT_GE(iLinks, 1);
    EXPECT_GE(path.m_Size, 2);

    // Every emitted way-point must stay inside the corridor.
    float fPrevX = fStart[0];
    float fPrevZ = fStart[2];
    for (int i = 0; i < iLinks; ++i)
    {
        const ZPathLink& link = aLinks[i];
        EXPECT_GE(link.m_Pos.x, -1.0f);
        EXPECT_LE(link.m_Pos.x, 33.0f);
        EXPECT_GE(link.m_Pos.z, -7.0f);
        EXPECT_LE(link.m_Pos.z, 7.0f);
        fPrevX = link.m_Pos.x;
        fPrevZ = link.m_Pos.z;
    }

    // The corridor is traversable end to end: the walk must always move forward.
    EXPECT_EQ(aLinks[iLinks - 1].m_Pos.x, fEnd[0]);
    EXPECT_GT(fPrevX, fStart[0]);
}
