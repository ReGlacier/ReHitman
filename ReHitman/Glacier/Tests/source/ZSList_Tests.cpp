#include <Glacier/ZSTL/ZSList.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZSListNodeBase<int, 0>) == 0x4);

    struct TestNode : ZSListNode<TestNode, 0>
    {
        int Value = 0;
    };

    struct OffsetNode : ZSListNode<OffsetNode, 0>
    {
        static int DestructorCount;

        int Value = 0;

        ~OffsetNode()
        {
            ++DestructorCount;
        }
    };

    int OffsetNode::DestructorCount = 0;
}

// ── ZSListNodeBase ──────────────────────────────────────────────────────────

TEST(ZSListNodeBase, ConstructorInitializesNextToNull)
{
    ZSListNodeBase<int, 0> node;

    EXPECT_EQ(node.GetNext(), nullptr);
    EXPECT_EQ(node.m_Next, nullptr);
}

TEST(ZSListNodeBase, GetNextReturnsNextPointer)
{
    ZSListNodeBase<int, 0> first;
    ZSListNodeBase<int, 0> second;

    first.m_Next = reinterpret_cast<int*>(&second);

    EXPECT_EQ(first.GetNext(), &second);
}

// ── ZSListBase ──────────────────────────────────────────────────────────────

TEST(ZSListBase, ConstructorInitializesFirstAndLastToNull)
{
    ZSListBase<int, 0> list;

    EXPECT_EQ(list.m_First, nullptr);
    EXPECT_EQ(list.m_Last, nullptr);
    EXPECT_EQ(list.Count(), 0);
}

TEST(ZSListBase, AddFirstWithEmptyListSetsFirstAndLast)
{
    ZSListBase<TestNode, 0> list;
    TestNode node{ .Value = 42 };

    list.AddFirst(&node);

    EXPECT_EQ(list.Count(), 1);
    EXPECT_EQ(reinterpret_cast<TestNode*>(list.m_First), &node);
    EXPECT_EQ(reinterpret_cast<TestNode*>(list.m_Last), &node);
    EXPECT_EQ(node.m_Next, nullptr);
}

TEST(ZSListBase, AddFirstWithTwoElementsMaintainsOrder)
{
    ZSListBase<TestNode, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.AddFirst(&first);
    list.AddFirst(&second);

    EXPECT_EQ(list.Count(), 2);
    EXPECT_EQ(reinterpret_cast<TestNode*>(list.m_First), &second);
    EXPECT_EQ(reinterpret_cast<TestNode*>(list.m_Last), &first);
    EXPECT_EQ(second.m_Next, &first);
    EXPECT_EQ(first.m_Next, nullptr);
}

TEST(ZSListBase, AddLastWithEmptyListSetsFirstAndLast)
{
    ZSListBase<TestNode, 0> list;
    TestNode node{ .Value = 99 };

    list.AddLast(&node);

    EXPECT_EQ(list.Count(), 1);
    EXPECT_EQ(reinterpret_cast<TestNode*>(list.m_First), &node);
    EXPECT_EQ(reinterpret_cast<TestNode*>(list.m_Last), &node);
    EXPECT_EQ(node.m_Next, nullptr);
}

TEST(ZSListBase, AddLastWithTwoElementsMaintainsOrder)
{
    ZSListBase<TestNode, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.AddLast(&first);
    list.AddLast(&second);

    EXPECT_EQ(list.Count(), 2);
    EXPECT_EQ(reinterpret_cast<TestNode*>(list.m_First), &first);
    EXPECT_EQ(reinterpret_cast<TestNode*>(list.m_Last), &second);
    EXPECT_EQ(first.m_Next, &second);
    EXPECT_EQ(second.m_Next, nullptr);
}

TEST(ZSListBase, GetFirstReturnsFirstElement)
{
    ZSListBase<TestNode, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.AddFirst(&first);
    list.AddFirst(&second);

    auto& nodeRef = list.GetFirst();
    EXPECT_EQ(reinterpret_cast<TestNode*>(nodeRef.m_Next), &first);
}

TEST(ZSListBase, GetLastReturnsLastElement)
{
    ZSListBase<TestNode, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.AddLast(&first);
    list.AddLast(&second);

    auto& nodeRef = list.GetLast();
    EXPECT_EQ(nodeRef.m_Next, nullptr);
}

TEST(ZSListBase, CountReturnsZeroForEmptyList)
{
    ZSListBase<TestNode, 0> list;

    EXPECT_EQ(list.Count(), 0);
}

TEST(ZSListBase, CountReturnsNumberOfElements)
{
    ZSListBase<TestNode, 0> list;
    TestNode nodes[5]{};

    for (auto& node : nodes)
        list.AddLast(&node);

    EXPECT_EQ(list.Count(), 5);
}

TEST(ZSListBase, UnlinkClearsFirstAndLast)
{
    ZSListBase<TestNode, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.AddFirst(&first);
    list.AddFirst(&second);

    list.Unlink();

    EXPECT_EQ(list.Count(), 0);
    EXPECT_EQ(list.m_First, nullptr);
    EXPECT_EQ(list.m_Last, nullptr);
}

TEST(ZSListBase, UnlinkNullsOutNodeNextPointers)
{
    ZSListBase<TestNode, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };
    TestNode third{ .Value = 3 };

    list.AddLast(&first);
    list.AddLast(&second);
    list.AddLast(&third);

    list.Unlink();

    EXPECT_EQ(first.m_Next, nullptr);
    EXPECT_EQ(second.m_Next, nullptr);
    EXPECT_EQ(third.m_Next, nullptr);
}

TEST(ZSListBase, UnlinkOnEmptyListIsNoop)
{
    ZSListBase<TestNode, 0> list;

    list.Unlink();

    EXPECT_EQ(list.m_First, nullptr);
    EXPECT_EQ(list.m_Last, nullptr);
}

TEST(ZSListBase, DestructorCallsUnlink)
{
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    {
        ZSListBase<TestNode, 0> list;
        list.AddLast(&first);
        list.AddLast(&second);
        ASSERT_EQ(list.Count(), 2);
    }

    EXPECT_EQ(first.m_Next, nullptr);
    EXPECT_EQ(second.m_Next, nullptr);
}

// ── ZSList ──────────────────────────────────────────────────────────────────

TEST(ZSList, IsEmptyReturnsTrueForNewList)
{
    ZSList<TestNode, false, 0> list;

    EXPECT_TRUE(list.IsEmpty());
}

TEST(ZSList, IsEmptyReturnsFalseAfterAddFirst)
{
    ZSList<TestNode, false, 0> list;
    TestNode node{ .Value = 1 };

    list.AddFirst(&node);

    EXPECT_FALSE(list.IsEmpty());
}

TEST(ZSList, IsEmptyReturnsFalseAfterAddLast)
{
    ZSList<TestNode, false, 0> list;
    TestNode node{ .Value = 1 };

    list.AddLast(&node);

    EXPECT_FALSE(list.IsEmpty());
}

TEST(ZSList, BeginReturnsFirstElement)
{
    ZSList<TestNode, false, 0> list;
    TestNode first{ .Value = 10 };
    TestNode second{ .Value = 20 };

    list.AddLast(&first);
    list.AddLast(&second);

    auto it = list.begin();
    EXPECT_EQ(it->Value, 10);
}

TEST(ZSList, EndReturnsNullIterator)
{
    ZSList<TestNode, false, 0> list;
    TestNode node{ .Value = 1 };

    list.AddLast(&node);

    auto it = list.end();
    EXPECT_EQ(it.m_Node, nullptr);
}

TEST(ZSList, IteratorEqualityComparesNodePointers)
{
    ZSList<TestNode, false, 0> list;
    TestNode node{ .Value = 1 };

    list.AddLast(&node);

    auto begin = list.begin();
    auto end = list.end();

    EXPECT_FALSE(begin == end);
    EXPECT_TRUE(begin != end);
}

TEST(ZSList, PrefixIncrementTraversesToNext)
{
    ZSList<TestNode, false, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };
    TestNode third{ .Value = 3 };

    list.AddLast(&first);
    list.AddLast(&second);
    list.AddLast(&third);

    auto it = list.begin();
    EXPECT_EQ(it->Value, 1);

    ++it;
    EXPECT_EQ(it->Value, 2);

    ++it;
    EXPECT_EQ(it->Value, 3);

    ++it;
    EXPECT_EQ(it, list.end());
}

TEST(ZSList, PostfixIncrementReturnsPreviousValue)
{
    ZSList<TestNode, false, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.AddLast(&first);
    list.AddLast(&second);

    auto it = list.begin();
    auto prev = it++;

    EXPECT_EQ(prev->Value, 1);
    EXPECT_EQ(it->Value, 2);
}

TEST(ZSList, DereferenceReturnsObjectReference)
{
    ZSList<TestNode, false, 0> list;
    TestNode node{ .Value = 77 };

    list.AddLast(&node);

    auto it = list.begin();
    EXPECT_EQ((*it).Value, 77);

    (*it).Value = 99;
    EXPECT_EQ(node.Value, 99);
}

TEST(ZSList, ArrowOperatorAccessesMembers)
{
    ZSList<TestNode, false, 0> list;
    TestNode node{ .Value = 55 };

    list.AddLast(&node);

    auto it = list.begin();
    EXPECT_EQ(it->Value, 55);
}

TEST(ZSList, EmptyListBeginEqualsEnd)
{
    ZSList<TestNode, false, 0> list;

    EXPECT_EQ(list.begin(), list.end());
}

TEST(ZSList, NullIteratorIncrementIsNoop)
{
    ZSListIterator<TestNode, 0> it(nullptr);

    ++it;

    EXPECT_EQ(it.m_Node, nullptr);
}

TEST(ZSList, NullIteratorPostfixIncrementDoesNotCrash)
{
    ZSListIterator<TestNode, 0> it(nullptr);

    auto prev = it++;

    EXPECT_EQ(prev.m_Node, nullptr);
    EXPECT_EQ(it.m_Node, nullptr);
}

TEST(ZSList, AddFirstThroughTypedInterface)
{
    ZSList<TestNode, false, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.AddFirst(&first);
    list.AddFirst(&second);

    auto it = list.begin();
    EXPECT_EQ(it->Value, 2);
    ++it;
    EXPECT_EQ(it->Value, 1);
}

TEST(ZSList, AddLastThroughTypedInterface)
{
    ZSList<TestNode, false, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.AddLast(&first);
    list.AddLast(&second);

    auto it = list.begin();
    EXPECT_EQ(it->Value, 1);
    ++it;
    EXPECT_EQ(it->Value, 2);
}

TEST(ZSList, DeleteAllDestroysAllHeapObjects)
{
    OffsetNode::DestructorCount = 0;

    ZSList<OffsetNode, false, 0> list;
    auto* first = new OffsetNode{ .Value = 1 };
    auto* second = new OffsetNode{ .Value = 2 };
    auto* third = new OffsetNode{ .Value = 3 };

    list.AddLast(first);
    list.AddLast(second);
    list.AddLast(third);

    list.DeleteAll();

    EXPECT_EQ(OffsetNode::DestructorCount, 3);
    EXPECT_EQ(list.Count(), 0);
    EXPECT_TRUE(list.IsEmpty());
}

TEST(ZSList, DeleteAllHandlesSingleElement)
{
    OffsetNode::DestructorCount = 0;

    ZSList<OffsetNode, false, 0> list;
    auto* node = new OffsetNode{ .Value = 1 };

    list.AddLast(node);
    list.DeleteAll();

    EXPECT_EQ(OffsetNode::DestructorCount, 1);
    EXPECT_TRUE(list.IsEmpty());
}

TEST(ZSList, DeleteAllOnEmptyListIsNoop)
{
    ZSList<OffsetNode, false, 0> list;

    EXPECT_NO_THROW(list.DeleteAll());
    EXPECT_TRUE(list.IsEmpty());
}

TEST(ZSList, ConstructorDefaultIsNoop)
{
    ZSList<TestNode, false, 0> list;

    EXPECT_TRUE(list.IsEmpty());
    EXPECT_EQ(list.m_First, nullptr);
    EXPECT_EQ(list.m_Last, nullptr);
}

TEST(ZSList, DestructorDefaultIsNoop)
{
    // ZSList destructor is defaulted, so nodes are NOT freed.
    // This test verifies no crash on destruction of empty list.
    {
        ZSList<TestNode, false, 0> list;
    }
    SUCCEED();
}

TEST(ZSList, MixedAddFirstAndAddLast)
{
    ZSList<TestNode, false, 0> list;
    TestNode a{ .Value = 1 };
    TestNode b{ .Value = 2 };
    TestNode c{ .Value = 3 };
    TestNode d{ .Value = 4 };

    list.AddLast(&a);   // a
    list.AddLast(&b);   // a -> b
    list.AddFirst(&c);  // c -> a -> b
    list.AddFirst(&d);  // d -> c -> a -> b

    auto it = list.begin();
    EXPECT_EQ(it->Value, 4); ++it;
    EXPECT_EQ(it->Value, 3); ++it;
    EXPECT_EQ(it->Value, 1); ++it;
    EXPECT_EQ(it->Value, 2); ++it;
    EXPECT_EQ(it, list.end());
}

TEST(ZSList, RangeForLoopIteratesAllElements)
{
    ZSList<TestNode, false, 0> list;
    TestNode nodes[4]{
        { .Value = 10 },
        { .Value = 20 },
        { .Value = 30 },
        { .Value = 40 },
    };

    for (auto& node : nodes)
        list.AddLast(&node);

    int sum = 0;
    for (auto& node : list)
        sum += node.Value;

    EXPECT_EQ(sum, 100);
}
