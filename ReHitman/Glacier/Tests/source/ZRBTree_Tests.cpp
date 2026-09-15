#include <Glacier/ZSTL/ZRBTree.h>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <vector>

using namespace Glacier;

namespace
{
    SBinTreeNode* InitNode(SBinTreeNode& node, int key)
    {
        node = {};
        node.m_lKey = key;
        return &node;
    }

    int AssertRBSubtree(ZRBTree& tree, SBinTreeNode* node)
    {
        if (node == tree.NIL)
        {
            EXPECT_EQ(node->m_bColor, ZRBTree::BLACK);
            return 1;
        }

        EXPECT_TRUE(node->m_bColor == ZRBTree::BLACK || node->m_bColor == ZRBTree::RED);

        if (node->m_bColor == ZRBTree::RED)
        {
            EXPECT_EQ(node->m_pLeft->m_bColor, ZRBTree::BLACK);
            EXPECT_EQ(node->m_pRight->m_bColor, ZRBTree::BLACK);
        }

        if (node->m_pLeft != tree.NIL)
        {
            EXPECT_LE(node->m_pLeft->m_lKey, node->m_lKey);
            EXPECT_EQ(node->m_pLeft->m_pParent, node);
        }

        if (node->m_pRight != tree.NIL)
        {
            EXPECT_GE(node->m_pRight->m_lKey, node->m_lKey);
            EXPECT_EQ(node->m_pRight->m_pParent, node);
        }

        const int leftBlackHeight = AssertRBSubtree(tree, node->m_pLeft);
        const int rightBlackHeight = AssertRBSubtree(tree, node->m_pRight);
        EXPECT_EQ(leftBlackHeight, rightBlackHeight);

        return leftBlackHeight + (node->m_bColor == ZRBTree::BLACK ? 1 : 0);
    }

    void AssertRBTreeValid(ZRBTree& tree)
    {
        if (tree.GetTopNode())
        {
            EXPECT_EQ(tree.GetTopNode()->m_bColor, ZRBTree::BLACK);
            EXPECT_EQ(tree.GetTopNode()->m_pParent, tree.NIL);
            AssertRBSubtree(tree, tree.GetTopNode());
        }
    }
}

TEST(ZRBTree, InsertMaintainsRedBlackInvariants)
{
    ZRBTree tree;
    std::array<SBinTreeNode, 15> nodes{};
    const std::array<int, 15> keys{8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15};

    for (size_t i = 0; i < keys.size(); ++i)
    {
        tree.Insert(InitNode(nodes[i], keys[i]));
        AssertRBTreeValid(tree);
    }

    EXPECT_LE(tree.Depth(nullptr), 6);
}

TEST(ZRBTree, DeleteLeafNodeMaintainsSearchAndInvariants)
{
    ZRBTree tree;
    std::array<SBinTreeNode, 7> nodes{};
    const std::array<int, 7> keys{40, 20, 60, 10, 30, 50, 70};

    for (size_t i = 0; i < keys.size(); ++i)
    {
        tree.Insert(InitNode(nodes[i], keys[i]));
    }

    SBinTreeNode* deleted = tree.Delete(tree.Search(10, nullptr));

    EXPECT_EQ(deleted, &nodes[3]);
    EXPECT_EQ(tree.Search(10, nullptr), nullptr);
    AssertRBTreeValid(tree);
}

TEST(ZRBTree, DeleteNodeWithTwoChildrenCopiesSuccessorKey)
{
    ZRBTree tree;
    std::array<SBinTreeNode, 7> nodes{};
    const std::array<int, 7> keys{40, 20, 60, 10, 30, 50, 70};

    for (size_t i = 0; i < keys.size(); ++i)
    {
        tree.Insert(InitNode(nodes[i], keys[i]));
    }

    SBinTreeNode* target = tree.Search(40, nullptr);
    SBinTreeNode* deleted = tree.Delete(target);

    EXPECT_EQ(deleted->m_lKey, 50);
    EXPECT_EQ(target->m_lKey, 50);
    EXPECT_EQ(tree.Search(40, nullptr), nullptr);
    EXPECT_NE(tree.Search(50, nullptr), nullptr);
    AssertRBTreeValid(tree);
}

TEST(ZRBTree, DeleteAllNodesEventuallyEmptiesTree)
{
    ZRBTree tree;
    std::array<SBinTreeNode, 9> nodes{};
    const std::array<int, 9> keys{5, 3, 7, 2, 4, 6, 8, 1, 9};

    for (size_t i = 0; i < keys.size(); ++i)
    {
        tree.Insert(InitNode(nodes[i], keys[i]));
    }

    for (int key : keys)
    {
        SBinTreeNode* node = tree.Search(key, nullptr);
        ASSERT_NE(node, nullptr);
        EXPECT_NE(tree.Delete(node), nullptr);
        AssertRBTreeValid(tree);
    }

    EXPECT_EQ(tree.GetTopNode(), nullptr);
}
