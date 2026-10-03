#include <Glacier/ZSTL/CQuadtree.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <vector>

using namespace Glacier;

namespace
{
    static_assert(sizeof(SRecurseAdd) == 0x1C);
    static_assert(sizeof(CNodeQuad) == 0x10);
    static_assert(sizeof(CQuadtreeObj) == 0x1C);
    static_assert(sizeof(SOctreeChk) == 0xC);
    static_assert(sizeof(SRecurseInfoCompiled) == 0x5C);
    static_assert(sizeof(CQuadtree) == 0x30);

    struct QueryContext
    {
        std::vector<unsigned int> Values;
    };

    bool CollectCallback(unsigned int value, SOctreeChk* info)
    {
        auto* context = static_cast<QueryContext*>(info->pUserData);
        context->Values.push_back(value);
        return true;
    }

    SRecurseInfoCompiled MakeQueryInfo(QueryContext& context)
    {
        SRecurseInfoCompiled info{};
        info.pChkFunc = &CollectCallback;
        info.pUserData = &context;
        return info;
    }

    bool Contains(const std::vector<unsigned int>& values, unsigned int value)
    {
        return std::find(values.begin(), values.end(), value) != values.end();
    }

    void CollectObjectsFromNode(CNodeQuad* node, CMemPool* pool, std::vector<CQuadtreeObj*>& out)
    {
        for (auto* current = node->m_tObjectList.GetHead(); current; current = current->m_pNext)
        {
            out.push_back(static_cast<CQuadtreeObj*>(current));
        }

        for (int i = 0; i < 4; ++i)
        {
            if (node->ChildExists(i))
            {
                CollectObjectsFromNode(node->GetChildPtr(pool, i), pool, out);
            }
        }
    }

    std::vector<CQuadtreeObj*> CollectObjects(CQuadtree& tree)
    {
        std::vector<CQuadtreeObj*> objects;
        CollectObjectsFromNode(tree.m_pRoot, &tree.m_tPool, objects);
        return objects;
    }

    CQuadtreeObj* FindObject(CQuadtree& tree, unsigned int id)
    {
        auto objects = CollectObjects(tree);
        for (auto* object : objects)
        {
            if (object->m_tInfo.iID == id)
            {
                return object;
            }
        }

        return nullptr;
    }
}

TEST(CNodeQuad, InitRootAndChildStoresParentAndDepth)
{
    CMemPool pool;
    pool.Init(sizeof(CNodeQuad), 4, 1);

    auto* root = static_cast<CNodeQuad*>(pool.Alloc());
    ASSERT_NE(root, nullptr);
    root->Init(&pool, nullptr);

    int childBlock = -1;
    auto* child = static_cast<CNodeQuad*>(pool.Alloc(&childBlock));
    ASSERT_NE(child, nullptr);
    child->Init(&pool, root);
    root->SetChild(2, static_cast<int16_t>(childBlock));

    EXPECT_EQ(root->GetDepth(), 0);
    EXPECT_TRUE(root->IsEmpty());
    EXPECT_FALSE(root->ChildExists(0));
    EXPECT_TRUE(root->ChildExists(2));
    EXPECT_FALSE(root->IsLeaf());

    EXPECT_EQ(child->GetDepth(), 1);
    EXPECT_EQ(child->GetParentPtr(&pool), root);
    EXPECT_EQ(root->GetChildPtr(&pool, 2), child);
    EXPECT_TRUE(child->IsLeaf());
}

TEST(CNodeQuad, AttachAndDetachMaintainObjectListAndBackPointer)
{
    CMemPool pool;
    pool.Init(sizeof(CNodeQuad), 1, 1);

    auto* node = static_cast<CNodeQuad*>(pool.Alloc());
    ASSERT_NE(node, nullptr);
    node->Init(&pool, nullptr);

    CQuadtreeObj first(10);
    CQuadtreeObj second(20);

    node->Attach(&first);
    node->Attach(&second);

    EXPECT_EQ(node->m_tObjectList.GetHead(), &second);
    EXPECT_EQ(second.m_pNext, &first);
    EXPECT_EQ(first.m_pPrev, &second);
    EXPECT_EQ(first.m_pNode, node);
    EXPECT_EQ(second.m_pNode, node);
    EXPECT_FALSE(node->IsEmpty());

    node->Detach(&second);

    EXPECT_EQ(node->m_tObjectList.GetHead(), &first);
    EXPECT_EQ(first.m_pPrev, nullptr);
    EXPECT_EQ(second.m_pNext, nullptr);
    EXPECT_EQ(second.m_pPrev, nullptr);
}

TEST(CNodeQuad, DeleteEmptySingleUnlinksChildAndReturnsPoolSlot)
{
    CMemPool pool;
    pool.Init(sizeof(CNodeQuad), 4, 1);

    auto* root = static_cast<CNodeQuad*>(pool.Alloc());
    ASSERT_NE(root, nullptr);
    root->Init(&pool, nullptr);

    int childBlock = -1;
    auto* child = static_cast<CNodeQuad*>(pool.Alloc(&childBlock));
    ASSERT_NE(child, nullptr);
    child->Init(&pool, root);
    root->SetChild(1, static_cast<int16_t>(childBlock));

    child->DeleteEmptySingle(&pool);

    EXPECT_FALSE(root->ChildExists(1));
    EXPECT_EQ(pool.m_iNumAlloc, 1);
    EXPECT_EQ(pool.Alloc(), child);
}

TEST(CQuadtree, ConstructorInitializesRootPoolDefaults)
{
    CQuadtree tree(16);

    ASSERT_NE(tree.m_pRoot, nullptr);
    EXPECT_EQ(tree.m_iObjects, 0);
    EXPECT_EQ(tree.m_fScale, 0.125f);
    EXPECT_EQ(tree.m_vOrigin, ZVector3(0.0f, 0.0f, 0.0f));
    EXPECT_EQ(tree.m_pRoot->GetDepth(), 0);
    EXPECT_TRUE(tree.m_pRoot->IsLeaf());
    EXPECT_EQ(tree.m_tPool.m_iEntries, 16);
    EXPECT_EQ(tree.m_tPool.m_iNumAlloc, 1);
}

TEST(CQuadtree, GetDepthMapsObjectDimensionToTreeDepth)
{
    CQuadtree tree(4);

    EXPECT_EQ(tree.GetDepth(1), 15);
    EXPECT_EQ(tree.GetDepth(2), 14);
    EXPECT_EQ(tree.GetDepth(3), 14);
    EXPECT_EQ(tree.GetDepth(4), 13);
    EXPECT_EQ(tree.GetDepth(0x4000), 1);
    EXPECT_EQ(tree.GetDepth(0x8000), 0);
    EXPECT_THROW(tree.GetDepth(0xFFFF), std::runtime_error);
}

TEST(CQuadtree, ConvToOCSUsesOriginAndScale)
{
    CQuadtree tree(4);

    tree.SetOrigin(ZVector3(10.0f, 20.0f, 30.0f));
    tree.SetScale(2.0f);

    int out[3]{};
    tree.ConvToOCS(out, ZVector3(11.0f, 18.0f, 35.5f));

    EXPECT_EQ(out[0], 32770);
    EXPECT_EQ(out[1], 32764);
    EXPECT_EQ(out[2], 32779);
    EXPECT_THROW(tree.SetScale(0.0f), std::runtime_error);
}

TEST(CQuadtree, AddMinMaxCreatesObjectAtExpectedLeafAndGetEverythingFindsIt)
{
    CQuadtree tree(64);

    tree.AddMinMax(100, 10, 200, 103, 13, 203, 777);

    EXPECT_EQ(tree.m_iObjects, 1);
    EXPECT_GT(tree.m_tPool.m_iNumAlloc, 1);

    CQuadtreeObj* object = FindObject(tree, 777);
    ASSERT_NE(object, nullptr);
    ASSERT_NE(object->m_pNode, nullptr);
    EXPECT_EQ(object->m_tInfo.iMinX, 100u);
    EXPECT_EQ(object->m_tInfo.iMinY, 10u);
    EXPECT_EQ(object->m_tInfo.iMinZ, 200u);
    EXPECT_EQ(object->m_tInfo.iMaxX, 103u);
    EXPECT_EQ(object->m_tInfo.iMaxY, 13u);
    EXPECT_EQ(object->m_tInfo.iMaxZ, 203u);
    EXPECT_EQ(object->m_pNode->GetDepth(), tree.GetDepth(4));
    EXPECT_EQ(object->m_pNode->m_tObjectList.GetHead(), object);

    QueryContext context;
    auto info = MakeQueryInfo(context);
    tree.GetEverything(&info);

    EXPECT_EQ(context.Values.size(), 1u);
    EXPECT_TRUE(Contains(context.Values, 777));
}

TEST(CQuadtree, AddMinMaxFallsBackToCurrentNodeWhenPoolIsFull)
{
    CQuadtree tree(1);

    tree.AddMinMax(100, 10, 200, 103, 13, 203, 1);

    EXPECT_EQ(tree.m_iObjects, 1);
    EXPECT_EQ(tree.m_tPool.m_iNumAlloc, 1);
    ASSERT_NE(tree.m_pRoot->m_tObjectList.GetHead(), nullptr);
    EXPECT_EQ(tree.m_pRoot->m_tObjectList.GetHead()->m_tInfo.iID, 1u);
}

TEST(CQuadtree, CheckCubeReturnsOnlyOverlappingObjects)
{
    CQuadtree tree(128);

    tree.AddMinMax(100, 0, 100, 110, 10, 110, 1);
    tree.AddMinMax(500, 0, 500, 510, 10, 510, 2);
    tree.AddMinMax(1000, 0, 1000, 1010, 10, 1010, 3);

    QueryContext context;
    auto info = MakeQueryInfo(context);
    tree.CheckCube(&info, 90, 0, 90, 520, 20, 520);

    EXPECT_TRUE(Contains(context.Values, 1));
    EXPECT_TRUE(Contains(context.Values, 2));
    EXPECT_FALSE(Contains(context.Values, 3));
}

TEST(CQuadtree, AddMinMaxVectorConvertsWorldBounds)
{
    CQuadtree tree(64);
    tree.SetOrigin(ZVector3(10.0f, 0.0f, 20.0f));
    tree.SetScale(2.0f);

    tree.AddMinMax(ZVector3(10.0f, 0.0f, 20.0f), ZVector3(12.0f, 4.0f, 23.0f), 99);

    CQuadtreeObj* object = FindObject(tree, 99);
    ASSERT_NE(object, nullptr);
    EXPECT_EQ(object->m_tInfo.iMinX, 32768u);
    EXPECT_EQ(object->m_tInfo.iMinY, 32768u);
    EXPECT_EQ(object->m_tInfo.iMinZ, 32768u);
    EXPECT_EQ(object->m_tInfo.iMaxX, 32772u);
    EXPECT_EQ(object->m_tInfo.iMaxY, 32776u);
    EXPECT_EQ(object->m_tInfo.iMaxZ, 32774u);
}

TEST(CQuadtree, DeleteRemovesObjectAndPrunesEmptyNodes)
{
    CQuadtree tree(64);
    tree.AddMinMax(100, 10, 200, 103, 13, 203, 7);

    CQuadtreeObj* object = FindObject(tree, 7);
    ASSERT_NE(object, nullptr);
    EXPECT_GT(tree.m_tPool.m_iNumAlloc, 1);

    tree.Delete(object);

    EXPECT_EQ(tree.m_iObjects, 0);
    EXPECT_EQ(tree.m_tPool.m_iNumAlloc, 1);
    EXPECT_TRUE(tree.m_pRoot->IsLeaf());

    QueryContext context;
    auto info = MakeQueryInfo(context);
    tree.GetEverything(&info);
    EXPECT_TRUE(context.Values.empty());
}

TEST(CQuadtree, MoveInsideSameNodeUpdatesBoundsWithoutReinsert)
{
    CQuadtree tree(64);
    tree.AddMinMax(100, 0, 100, 103, 3, 103, 42);

    CQuadtreeObj* object = FindObject(tree, 42);
    ASSERT_NE(object, nullptr);
    CNodeQuad* oldNode = object->m_pNode;
    int oldAllocCount = tree.m_tPool.m_iNumAlloc;

    int minPos[3] = {104, 1, 104};
    int maxPos[3] = {107, 4, 107};
    tree.Move(object, minPos, maxPos);

    EXPECT_EQ(object->m_pNode, oldNode);
    EXPECT_EQ(tree.m_tPool.m_iNumAlloc, oldAllocCount);
    EXPECT_EQ(object->m_tInfo.iMinX, 104u);
    EXPECT_EQ(object->m_tInfo.iMaxZ, 107u);
}

TEST(CQuadtree, MoveToDifferentNodeReattachesObjectAndLeavesOldNodeEmpty)
{
    CQuadtree tree(128);
    tree.AddMinMax(100, 0, 100, 103, 3, 103, 42);

    CQuadtreeObj* object = FindObject(tree, 42);
    ASSERT_NE(object, nullptr);
    CNodeQuad* oldNode = object->m_pNode;

    int minPos[3] = {4000, 1, 4000};
    int maxPos[3] = {4003, 4, 4003};
    tree.Move(object, minPos, maxPos);

    EXPECT_NE(object->m_pNode, oldNode);
    EXPECT_EQ(object->m_pNode->m_tObjectList.GetHead(), object);
    EXPECT_TRUE(oldNode->IsEmpty());

    QueryContext context;
    auto info = MakeQueryInfo(context);
    tree.CheckCube(&info, 3990, 0, 3990, 4010, 10, 4010);
    EXPECT_TRUE(Contains(context.Values, 42));
}

TEST(CQuadtree, CheckLinesegmentReportsIntersectedObjectInfoPointers)
{
    CQuadtree tree(128);
    tree.SetScale(1.0f);

    tree.AddMinMax(32760, 32760, 32760, 32770, 32770, 32770, 1);
    tree.AddMinMax(33000, 32760, 33000, 33010, 32770, 33010, 2);

    CQuadtreeObj* first = FindObject(tree, 1);
    ASSERT_NE(first, nullptr);

    QueryContext context;
    auto info = MakeQueryInfo(context);
    bool hit = tree.CheckLinesegment(&info, ZVector3(-20.0f, 0.0f, -20.0f), ZVector3(20.0f, 0.0f, 20.0f));

    EXPECT_TRUE(hit);
    EXPECT_TRUE(Contains(context.Values, reinterpret_cast<unsigned int>(&first->m_tInfo)));
}
