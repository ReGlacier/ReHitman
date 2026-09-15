#include <Glacier/Physics/ZOctreeCompiled.h>
#include <Glacier/ZSTL/CObjectInfo.h>
#include <Glacier/ZSTL/CQuadtree.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <vector>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZNodeCompiled) == 0x6);
    static_assert(sizeof(ZOctree) == 0x14);
    static_assert(sizeof(ZOctreeCompiled) == 0x18);

    constexpr uint16_t MakeNodeInfo(uint16_t nodeId, uint16_t numObjects, bool isLast = false)
    {
        return static_cast<uint16_t>((nodeId & 7) | ((numObjects & 0xFFF) << 3) | (isLast ? 0x8000 : 0));
    }

    struct QueryContext
    {
        std::vector<unsigned int> Values;
    };

    bool CollectCallback(unsigned int value, SOctreeChk* info)
    {
        static_cast<QueryContext*>(info->pUserData)->Values.push_back(value);
        return true;
    }

    SRecurseInfoCompiled MakeQueryInfo(QueryContext& context, ZNodeCompiled* nodes, CObjectInfo* objects)
    {
        SRecurseInfoCompiled info{};
        info.fResT = 2.0f;
        info.pChkFunc = &CollectCallback;
        info.pUserData = &context;
        info.pxBasePtrNodes = nodes;
        info.pxBasePtrObjects = objects;
        return info;
    }

    bool Contains(const std::vector<unsigned int>& values, unsigned int value)
    {
        return std::find(values.begin(), values.end(), value) != values.end();
    }

    unsigned int RemapId(unsigned int id)
    {
        return id * 10 + 1;
    }

    struct DummyOctree : ZOctree
    {
        void RemapObjects(RemapObjectIdFn) override {}
        bool CheckLinesegment(SRecurseInfoCompiled*, float*, float*) override { return false; }
        void CheckCube(SRecurseInfoCompiled*, float*, float*) override {}
        void GetEverything(SRecurseInfoCompiled*) override {}
        void CheckPoint(SRecurseInfoCompiled*, const float*) override {}
    };

    struct BinaryOctreeHeader
    {
        uint32_t m_iObjectsOffset;
        ZVector3 m_vOrigin;
        float m_fScale;
    };

    struct CompiledTreeFixture
    {
        BinaryOctreeHeader Header{};
        ZNodeCompiled Nodes[3]{};
        CObjectInfo Objects[3]{};
    };

    static_assert(offsetof(CompiledTreeFixture, Nodes) == sizeof(BinaryOctreeHeader));
}

TEST(ZMath, Vector3ConstructorsAndMutatingOperatorsMatchEngineStyle)
{
    const float raw[3] = {1.0f, 2.0f, 3.0f};

    ZVector3 fromScalar(5.0f);
    ZVector3 fromPointer(raw);
    ZVector3 fromComponents(4.0f, 6.0f, 8.0f);

    EXPECT_EQ(fromScalar, ZVector3(5.0f, 5.0f, 5.0f));
    EXPECT_EQ(fromPointer, ZVector3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(fromComponents - fromPointer, ZVector3(3.0f, 4.0f, 5.0f));

    fromComponents * 0.5f;
    EXPECT_EQ(fromComponents, ZVector3(2.0f, 3.0f, 4.0f));

    fromComponents / 2.0f;
    EXPECT_EQ(fromComponents, ZVector3(1.0f, 1.5f, 2.0f));

    fromComponents + 10.0f;
    EXPECT_EQ(fromComponents, ZVector3(11.0f, 11.5f, 12.0f));
}

TEST(ZOctree, SettersGettersAndOCSConversionUseOriginAndScale)
{
    DummyOctree octree;

    octree.SetOrigin(ZVector3(10.0f, 20.0f, 30.0f));
    octree.SetScale(2.0f);

    ZVector3 origin;
    octree.GetOrigin(origin);

    EXPECT_EQ(origin, ZVector3(10.0f, 20.0f, 30.0f));
    EXPECT_EQ(octree.GetScale(), 2.0f);

    int ocs[3]{};
    octree.ConvToOCS_NoAssert(ocs, ZVector3(11.0f, 18.0f, 35.5f));
    EXPECT_EQ(ocs[0], 32770);
    EXPECT_EQ(ocs[1], 32764);
    EXPECT_EQ(ocs[2], 32779);

    const float raw[3] = {9.5f, 20.25f, 30.0f};
    octree.ConvToOCS_NoAssert(ocs, raw);
    EXPECT_EQ(ocs[0], 32767);
    EXPECT_EQ(ocs[1], 32768);
    EXPECT_EQ(ocs[2], 32768);
}

TEST(ZOctree, InvalidScaleAsserts)
{
    DummyOctree octree;

    EXPECT_THROW(octree.SetScale(0.0f), std::runtime_error);
}

TEST(ZNodeCompiled, PackedInfoAccessorsAndBaseRelativePointers)
{
    ZNodeCompiled nodes[3]{};
    CObjectInfo objects[4]{};

    nodes[0].m_iInfo = MakeNodeInfo(5, 17, true);
    nodes[0].m_iFirstChild = 2;
    nodes[0].m_iFirstObject = 3;

    EXPECT_TRUE(nodes[0].IsLast());
    EXPECT_EQ(nodes[0].GetNodeID(), 5u);
    EXPECT_EQ(nodes[0].GetNumObjects(), 17u);
    EXPECT_EQ(nodes[0].GetFirstChildPtr(nodes), &nodes[2]);
    EXPECT_EQ(nodes[0].GetFirstObjectPtr(objects), &objects[3]);

    nodes[0].m_iFirstChild = 0;
    EXPECT_EQ(nodes[0].GetFirstChildPtr(nodes), nullptr);
}

TEST(ZNodeCompiled, GetObjectsWalksCurrentNodeAndAllContiguousChildren)
{
    ZNodeCompiled nodes[3]{};
    CObjectInfo objects[3]{};

    nodes[0] = {MakeNodeInfo(0, 1), 1, 0};
    nodes[1] = {MakeNodeInfo(0, 1), 0, 1};
    nodes[2] = {MakeNodeInfo(1, 1, true), 0, 2};

    objects[0].iID = 100;
    objects[1].iID = 200;
    objects[2].iID = 300;

    QueryContext context;
    auto info = MakeQueryInfo(context, nodes, objects);

    nodes[0].GetObjects(&info);

    EXPECT_EQ(context.Values, (std::vector<unsigned int>{100, 200, 300}));
}

TEST(ZNodeCompiled, RemapObjectsUpdatesCurrentNodeAndChildren)
{
    ZNodeCompiled nodes[3]{};
    CObjectInfo objects[3]{};

    nodes[0] = {MakeNodeInfo(0, 1), 1, 0};
    nodes[1] = {MakeNodeInfo(0, 1), 0, 1};
    nodes[2] = {MakeNodeInfo(1, 1, true), 0, 2};

    objects[0].iID = 1;
    objects[1].iID = 2;
    objects[2].iID = 3;

    SRecurseInfoCompiled info{};
    info.pxBasePtrNodes = nodes;
    info.pxBasePtrObjects = objects;
    info.pUserData = reinterpret_cast<void*>(&RemapId);

    nodes[0].RemapObjects(&info);

    EXPECT_EQ(objects[0].iID, 11u);
    EXPECT_EQ(objects[1].iID, 21u);
    EXPECT_EQ(objects[2].iID, 31u);
}

TEST(ZNodeCompiled, CheckPointReportsOnlyObjectsContainingThePoint)
{
    ZNodeCompiled nodes[1]{};
    CObjectInfo objects[2]{};

    nodes[0] = {MakeNodeInfo(0, 2, true), 0, 0};
    objects[0] = {10, 100, 100, 100, 200, 200, 200};
    objects[1] = {20, 201, 100, 100, 300, 200, 200};

    QueryContext context;
    auto info = MakeQueryInfo(context, nodes, objects);
    info.vPos[0] = 150;
    info.vPos[1] = 150;
    info.vPos[2] = 150;

    nodes[0].CheckPoint(&info, 0x8000, 0x8000, 0x8000);

    EXPECT_EQ(context.Values, (std::vector<unsigned int>{10}));
    EXPECT_EQ(info.iDepth, 0);
}

TEST(ZNodeCompiled, CheckCubeReportsOverlapsAndUsesGetObjectsForFullyCoveredChild)
{
    ZNodeCompiled nodes[2]{};
    CObjectInfo objects[3]{};

    nodes[0] = {MakeNodeInfo(0, 1), 1, 0};
    nodes[1] = {MakeNodeInfo(0, 2, true), 0, 1};
    objects[0] = {10, 40000, 40000, 40000, 40100, 40100, 40100};
    objects[1] = {20, 100, 100, 100, 110, 110, 110};
    objects[2] = {30, 200, 200, 200, 210, 210, 210};

    QueryContext context;
    auto info = MakeQueryInfo(context, nodes, objects);
    info.iMinX = -1000;
    info.iMinY = -1000;
    info.iMinZ = -1000;
    info.iMaxX = 0x8000;
    info.iMaxY = 0x8000;
    info.iMaxZ = 0x8000;

    EXPECT_TRUE(nodes[0].CheckCube(&info, 0, 0x8000, 0x8000, 0x8000));
    EXPECT_FALSE(Contains(context.Values, 10));
    EXPECT_TRUE(Contains(context.Values, 20));
    EXPECT_TRUE(Contains(context.Values, 30));
}

TEST(ZNodeCompiled, CheckLinesegmentVisitsIntersectedChildrenFrontToBack)
{
    ZNodeCompiled nodes[3]{};
    CObjectInfo objects[2]{};

    nodes[0] = {MakeNodeInfo(0, 0), 1, 0};
    nodes[1] = {MakeNodeInfo(0, 1), 0, 0};
    nodes[2] = {MakeNodeInfo(1, 1, true), 0, 1};

    objects[0] = {10, 1000, 32760, 32760, 1100, 32780, 32780};
    objects[1] = {20, 64000, 32760, 32760, 64100, 32780, 32780};

    float a[3] = {-2000.0f, 32768.0f, 32768.0f};
    float b[3] = {66000.0f, 32768.0f, 32768.0f};

    QueryContext context;
    auto info = MakeQueryInfo(context, nodes, objects);
    info.pA = a;
    info.pB = b;

    EXPECT_TRUE(nodes[0].CheckLinesegment2(&info, 0x10000, 0x8000, 0x8000, 0x8000));
    EXPECT_EQ(context.Values, (std::vector<unsigned int>{10, 20}));
}

TEST(ZOctreeCompiled, InitExposesCompiledTreePointersAndCopiesHeaderTransform)
{
    CompiledTreeFixture tree{};
    tree.Header.m_iObjectsOffset = offsetof(CompiledTreeFixture, Objects);
    tree.Header.m_vOrigin = ZVector3(1.0f, 2.0f, 3.0f);
    tree.Header.m_fScale = 4.0f;
    tree.Nodes[0] = {MakeNodeInfo(0, 1, true), 0, 0};
    tree.Objects[0] = {123, 0, 0, 0, 10, 10, 10};

    ZOctreeCompiled octree;
    octree.Init(&tree);

    ZVector3 origin;
    octree.GetOrigin(origin);

    EXPECT_EQ(octree.GetTreeDataPtr(), &tree);
    EXPECT_EQ(octree.GetBasePtrNodes(), tree.Nodes);
    EXPECT_EQ(octree.GetBasePtrObjects(), tree.Objects);
    EXPECT_EQ(origin, ZVector3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(octree.GetScale(), 4.0f);

    QueryContext context;
    SRecurseInfoCompiled info{};
    info.pChkFunc = &CollectCallback;
    info.pUserData = &context;
    octree.GetEverything(&info);

    EXPECT_EQ(context.Values, (std::vector<unsigned int>{123}));
}

TEST(ZOctreeCompiled, CheckCubeReportsOnlyOverlappingObjectsThroughPublicAPI)
{
    CompiledTreeFixture tree{};
    tree.Header.m_iObjectsOffset = offsetof(CompiledTreeFixture, Objects);
    tree.Header.m_fScale = 1.0f;
    tree.Nodes[0] = {MakeNodeInfo(0, 3, true), 0, 0};
    tree.Objects[0] = {10, 32760, 32760, 32760, 32770, 32770, 32770};
    tree.Objects[1] = {20, 33000, 32760, 33000, 33010, 32770, 33010};
    tree.Objects[2] = {30, 100, 100, 100, 110, 110, 110};

    ZOctreeCompiled octree;
    octree.Init(&tree);

    float minPos[3] = {-20.0f, -20.0f, -20.0f};
    float maxPos[3] = {20.0f, 20.0f, 20.0f};
    QueryContext context;
    SRecurseInfoCompiled info{};
    info.pChkFunc = &CollectCallback;
    info.pUserData = &context;

    octree.CheckCube(&info, minPos, maxPos);

    EXPECT_EQ(context.Values, (std::vector<unsigned int>{10}));
    EXPECT_EQ(info.iMinX, 32748);
    EXPECT_EQ(info.iMaxX, 32788);
}

TEST(ZOctreeCompiled, CheckCubeOCSReportsOnlyOverlappingObjectsThroughPublicAPI)
{
    CompiledTreeFixture tree{};
    tree.Header.m_iObjectsOffset = offsetof(CompiledTreeFixture, Objects);
    tree.Header.m_fScale = 1.0f;
    tree.Nodes[0] = {MakeNodeInfo(0, 3, true), 0, 0};
    tree.Objects[0] = {10, 32760, 32760, 32760, 32770, 32770, 32770};
    tree.Objects[1] = {20, 33000, 32760, 33000, 33010, 32770, 33010};
    tree.Objects[2] = {30, 100, 100, 100, 110, 110, 110};

    ZOctreeCompiled octree;
    octree.Init(&tree);

    QueryContext context;
    auto info = MakeQueryInfo(context, tree.Nodes, tree.Objects);

    EXPECT_TRUE(octree.CheckCube(&info, 32990, 32750, 32990, 33020, 32780, 33020));
    EXPECT_EQ(context.Values, (std::vector<unsigned int>{20}));
}

TEST(ZOctreeCompiled, CheckPointReportsOnlyObjectsContainingWorldPoint)
{
    CompiledTreeFixture tree{};
    tree.Header.m_iObjectsOffset = offsetof(CompiledTreeFixture, Objects);
    tree.Header.m_fScale = 1.0f;
    tree.Nodes[0] = {MakeNodeInfo(0, 3, true), 0, 0};
    tree.Objects[0] = {10, 32760, 32760, 32760, 32770, 32770, 32770};
    tree.Objects[1] = {20, 33000, 32760, 33000, 33010, 32770, 33010};
    tree.Objects[2] = {30, 100, 100, 100, 110, 110, 110};

    ZOctreeCompiled octree;
    octree.Init(&tree);

    float point[3] = {0.0f, 0.0f, 0.0f};
    QueryContext context;
    SRecurseInfoCompiled info{};
    info.pChkFunc = &CollectCallback;
    info.pUserData = &context;

    octree.CheckPoint(&info, point);

    EXPECT_EQ(context.Values, (std::vector<unsigned int>{10}));
    EXPECT_EQ(info.vPos[0], 32768);
    EXPECT_EQ(info.vPos[1], 32768);
    EXPECT_EQ(info.vPos[2], 32768);
}

TEST(ZOctreeCompiled, CheckLinesegmentReportsIntersectedObjectsThroughPublicAPI)
{
    CompiledTreeFixture tree{};
    tree.Header.m_iObjectsOffset = offsetof(CompiledTreeFixture, Objects);
    tree.Header.m_fScale = 1.0f;
    tree.Nodes[0] = {MakeNodeInfo(0, 3, true), 0, 0};
    tree.Objects[0] = {10, 32760, 32760, 32760, 32770, 32770, 32770};
    tree.Objects[1] = {20, 33000, 32760, 33000, 33010, 32770, 33010};
    tree.Objects[2] = {30, 100, 100, 100, 110, 110, 110};

    ZOctreeCompiled octree;
    octree.Init(&tree);

    float a[3] = {-20.0f, 0.0f, 0.0f};
    float b[3] = {20.0f, 0.0f, 0.0f};
    QueryContext context;
    SRecurseInfoCompiled info{};
    info.fResT = 2.0f;
    info.pChkFunc = &CollectCallback;
    info.pUserData = &context;

    EXPECT_TRUE(octree.CheckLinesegment(&info, a, b));
    EXPECT_EQ(context.Values, (std::vector<unsigned int>{10}));
}

TEST(ZOctreeCompiled, RemapObjectsUpdatesAllCompiledObjectIdsThroughPublicAPI)
{
    CompiledTreeFixture tree{};
    tree.Header.m_iObjectsOffset = offsetof(CompiledTreeFixture, Objects);
    tree.Header.m_fScale = 1.0f;
    tree.Nodes[0] = {MakeNodeInfo(0, 3, true), 0, 0};
    tree.Objects[0].iID = 1;
    tree.Objects[1].iID = 2;
    tree.Objects[2].iID = 3;

    ZOctreeCompiled octree;
    octree.Init(&tree);

    octree.RemapObjects(&RemapId);

    EXPECT_EQ(tree.Objects[0].iID, 11u);
    EXPECT_EQ(tree.Objects[1].iID, 21u);
    EXPECT_EQ(tree.Objects[2].iID, 31u);
}
