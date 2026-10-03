#include <Glacier/ZSTL/ZRBTree.h>
#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <vector>

using namespace Glacier;

namespace
{
    struct TestRBTree : ZRBTree
    {
        using ZRBTree::Insert;
        using ZRBTree::Delete;
    };

    SBinTreeNode* MakeNode(SBinTreeNode& node, int key)
    {
        node = {};
        node.m_lKey = key;
        return &node;
    }

    void CollectInOrder(ZBinTree& tree, SBinTreeNode* node, std::vector<int>& keys)
    {
        if (!node || node == tree.NIL)
        {
            return;
        }

        CollectInOrder(tree, node->m_pLeft, keys);
        keys.push_back(node->m_lKey);
        CollectInOrder(tree, node->m_pRight, keys);
    }
}

TEST(ZBinTree, SearchAndOrderedHelpersFollowBinaryTreeRules)
{
    TestRBTree tree;
    std::array<SBinTreeNode, 7> nodes{};
    const std::array<int, 7> keys{40, 20, 60, 10, 30, 50, 70};

    for (size_t i = 0; i < keys.size(); ++i)
    {
        tree.Insert(MakeNode(nodes[i], keys[i]));
    }

    EXPECT_EQ(tree.GetTopNode()->m_lKey, 40);
    EXPECT_EQ(tree.Search(30, nullptr), &nodes[4]);
    EXPECT_EQ(tree.Search(99, nullptr), nullptr);
    EXPECT_EQ(tree.Minimum(tree.GetTopNode())->m_lKey, 10);
    EXPECT_EQ(tree.Maximum(tree.GetTopNode())->m_lKey, 70);
    EXPECT_EQ(tree.Successor(tree.Search(40, nullptr))->m_lKey, 50);
    EXPECT_EQ(tree.SearchLargerOrSame(25, nullptr)->m_lKey, 30);
    EXPECT_EQ(tree.SearchLargerOrSame(70, nullptr)->m_lKey, 70);
    EXPECT_EQ(tree.SearchLargerOrSame(71, nullptr), nullptr);

    std::vector<int> orderedKeys;
    CollectInOrder(tree, tree.GetTopNode(), orderedKeys);
    EXPECT_EQ(orderedKeys, (std::vector<int>{10, 20, 30, 40, 50, 60, 70}));

    tree.CheckLinks(nullptr);
}

TEST(ZBinTree, DuplicatedKeysAreStoredOnRightSideAndSearchFindsFirstMatch)
{
    TestRBTree tree;
    std::array<SBinTreeNode, 4> nodes{};

    tree.Insert(MakeNode(nodes[0], 10));
    tree.Insert(MakeNode(nodes[1], 10));
    tree.Insert(MakeNode(nodes[2], 9));
    tree.Insert(MakeNode(nodes[3], 11));

    std::vector<int> orderedKeys;
    CollectInOrder(tree, tree.GetTopNode(), orderedKeys);

    EXPECT_EQ(orderedKeys, (std::vector<int>{9, 10, 10, 11}));
    EXPECT_NE(tree.Search(10, nullptr), nullptr);
    tree.CheckLinks(nullptr);
}

TEST(ZBinTree, NextReturnsChildBeforeSibling)
{
    TestRBTree tree;
    std::array<SBinTreeNode, 5> nodes{};
    const std::array<int, 5> keys{30, 10, 40, 5, 20};

    for (size_t i = 0; i < keys.size(); ++i)
    {
        tree.Insert(MakeNode(nodes[i], keys[i]));
    }

    ASSERT_NE(tree.GetTopNode(), nullptr);
    ASSERT_NE(tree.GetTopNode()->m_pLeft, tree.NIL);
    EXPECT_EQ(tree.Next(tree.GetTopNode()), tree.GetTopNode()->m_pLeft);
    tree.CheckLinks(nullptr);
}
