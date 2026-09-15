#include <Glacier/ZSTL/ZValTree.h>
#include <gtest/gtest.h>

#include <array>
#include <vector>

using namespace Glacier;

namespace
{
    void CollectKeys(ZValTree& tree, SBinTreeNode* node, std::vector<int>& keys)
    {
        if (!node || node == tree.NIL)
        {
            return;
        }

        CollectKeys(tree, node->m_pLeft, keys);
        keys.push_back(node->m_lKey);
        CollectKeys(tree, node->m_pRight, keys);
    }
}

TEST(ZValTree, InsertKeyStoresAndFindsValues)
{
    ZValTree tree(4);

    tree.InsertKey(10, 100);
    tree.InsertKey(5, 50);
    tree.InsertKey(20, 200);

    EXPECT_EQ(tree.GetKeyVal(10), 100);
    EXPECT_EQ(tree.GetKeyVal(5), 50);
    EXPECT_EQ(tree.GetKeyVal(20), 200);
    EXPECT_EQ(tree.GetKeyVal(99), 0);
    EXPECT_EQ(tree.m_pNodesList->Count(), 3);
    tree.CheckLinks(nullptr);
}

TEST(ZValTree, DeleteKeyRemovesOnlyMatchingNodeAndReleasesStorage)
{
    ZValTree tree(2);

    tree.InsertKey(10, 100);
    tree.InsertKey(5, 50);
    tree.InsertKey(20, 200);

    tree.DeleteKey(5);

    EXPECT_EQ(tree.GetKeyVal(5), 0);
    EXPECT_EQ(tree.GetKeyVal(10), 100);
    EXPECT_EQ(tree.GetKeyVal(20), 200);
    EXPECT_EQ(tree.m_pNodesList->Count(), 2);
    tree.CheckLinks(nullptr);
}

TEST(ZValTree, DeleteNodeWithTwoChildrenCopiesSuccessorValue)
{
    ZValTree tree(8);

    tree.InsertKey(40, 400);
    tree.InsertKey(20, 200);
    tree.InsertKey(60, 600);
    tree.InsertKey(10, 100);
    tree.InsertKey(30, 300);
    tree.InsertKey(50, 500);
    tree.InsertKey(70, 700);

    tree.DeleteKey(40);

    EXPECT_EQ(tree.GetKeyVal(40), 0);
    EXPECT_EQ(tree.GetKeyVal(50), 500);
    EXPECT_EQ(tree.m_pNodesList->Count(), 6);
    tree.CheckLinks(nullptr);
}

TEST(ZValTree, DuplicateKeysAreSupportedAndDeletedOneAtATime)
{
    ZValTree tree(4);

    tree.InsertKey(10, 100);
    tree.InsertKey(10, 101);
    tree.InsertKey(10, 102);

    EXPECT_NE(tree.Search(10, nullptr), nullptr);
    EXPECT_EQ(tree.m_pNodesList->Count(), 3);

    tree.DeleteKey(10);
    EXPECT_EQ(tree.m_pNodesList->Count(), 2);
    EXPECT_NE(tree.Search(10, nullptr), nullptr);

    tree.DeleteKey(10);
    tree.DeleteKey(10);
    EXPECT_EQ(tree.Search(10, nullptr), nullptr);
    EXPECT_EQ(tree.m_pNodesList->Count(), 0);
    EXPECT_EQ(tree.GetTopNode(), nullptr);
}

TEST(ZValTree, DeleteMissingKeyIsNoOp)
{
    ZValTree tree(2);

    tree.InsertKey(1, 10);
    tree.DeleteKey(99);

    EXPECT_EQ(tree.GetKeyVal(1), 10);
    EXPECT_EQ(tree.m_pNodesList->Count(), 1);
}
