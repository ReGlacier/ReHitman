#include <Glacier/ZSTL/ZList.h>
#include <gtest/gtest.h>

#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZListNodeBase<int>) == 0x8);
    static_assert(sizeof(ZListBase<int>) == 0x8);

    struct TestNode : ZListNode<TestNode, 0>
    {
        int Value = 0;
    };

    struct OffsetBase
    {
        int Prefix = 0;
    };

    struct OffsetNode : OffsetBase, ZListNode<OffsetNode, 0>
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

TEST(ZList, ConstructorInitializesEmptySentinel)
{
    ZList<TestNode, false, 0> list;

    EXPECT_EQ(list.Count(), 0);
    EXPECT_EQ(list.GetFirst(), nullptr);
    EXPECT_EQ(list.GetLast(), nullptr);
    EXPECT_EQ(list.m_Head.m_Next, reinterpret_cast<ZListNodeBase<TestNode>*>(&list));
    EXPECT_EQ(list.m_Head.m_Prev, reinterpret_cast<ZListNodeBase<TestNode>*>(&list));
}

TEST(ZList, AddPrevInsertsBeforeHead)
{
    ZList<TestNode, false, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.m_Head.AddPrev(&first);
    list.m_Head.AddPrev(&second);

    EXPECT_EQ(list.Count(), 2);
    EXPECT_EQ(list.GetFirst(), &first);
    EXPECT_EQ(list.GetLast(), &second);
    EXPECT_TRUE(first.IsLinked());
    EXPECT_TRUE(second.IsLinked());
}

TEST(ZList, AddNextInsertsAfterHead)
{
    ZList<TestNode, false, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.m_Head.AddNext(&first);
    list.m_Head.AddNext(&second);

    EXPECT_EQ(list.Count(), 2);
    EXPECT_EQ(list.GetFirst(), &second);
    EXPECT_EQ(list.GetLast(), &first);
}

TEST(ZList, UnlinkRemovesNodeAndClearsLinks)
{
    ZList<TestNode, false, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };
    TestNode third{ .Value = 3 };

    list.m_Head.AddPrev(&first);
    list.m_Head.AddPrev(&second);
    list.m_Head.AddPrev(&third);

    second.Unlink();

    EXPECT_FALSE(second.IsLinked());
    EXPECT_EQ(second.m_Next, nullptr);
    EXPECT_EQ(second.m_Prev, nullptr);
    EXPECT_EQ(list.Count(), 2);
    EXPECT_EQ(first.m_Next, &third);
    EXPECT_EQ(third.m_Prev, &first);
}

TEST(ZList, UnlinkAllDetachesEveryNode)
{
    ZList<TestNode, false, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.m_Head.AddPrev(&first);
    list.m_Head.AddPrev(&second);

    list.UnlinkAll();

    EXPECT_EQ(list.Count(), 0);
    EXPECT_EQ(list.GetFirst(), nullptr);
    EXPECT_EQ(list.GetLast(), nullptr);
    EXPECT_FALSE(first.IsLinked());
    EXPECT_FALSE(second.IsLinked());
}

TEST(ZList, NodeDestructorUnlinksFromList)
{
    ZList<TestNode, false, 0> list;

    {
        TestNode node{ .Value = 1 };
        list.m_Head.AddPrev(&node);
        ASSERT_EQ(list.Count(), 1);
    }

    EXPECT_EQ(list.Count(), 0);
    EXPECT_EQ(list.GetFirst(), nullptr);
}

TEST(ZList, AddingLinkedNodeAsserts)
{
    ZList<TestNode, false, 0> list;
    TestNode node{ .Value = 1 };

    list.m_Head.AddPrev(&node);

    EXPECT_THROW(list.m_Head.AddPrev(&node), std::runtime_error);
    EXPECT_THROW(list.m_Head.AddNext(&node), std::runtime_error);
}

TEST(ZList, AddFirstInsertsAtFront)
{
    ZList<TestNode, false, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.AddFirst(&first);
    list.AddFirst(&second);

    EXPECT_EQ(list.Count(), 2);
    EXPECT_EQ(&*list.Begin(), &second);
    EXPECT_EQ(list.GetFirst(), &second);
    EXPECT_EQ(list.GetLast(), &first);
}

TEST(ZList, AddLastInsertsAtBack)
{
    ZList<TestNode, false, 0> list;
    TestNode first{ .Value = 1 };
    TestNode second{ .Value = 2 };

    list.AddLast(&first);
    list.AddLast(&second);

    EXPECT_EQ(list.Count(), 2);
    EXPECT_EQ(&*list.Begin(), &first);
    EXPECT_EQ(list.GetFirst(), &first);
    EXPECT_EQ(list.GetLast(), &second);
}

TEST(ZList, IteratorAdjustsEmbeddedNodeOffset)
{
    ZList<OffsetNode, false, 0> list;
    OffsetNode first{ .Value = 1 };
    OffsetNode second{ .Value = 2 };

    list.AddLast(&first);
    list.AddLast(&second);

    auto it = list.Begin();
    EXPECT_EQ(static_cast<OffsetNode*>(it), &first);
    EXPECT_EQ(&*it, &first);
    EXPECT_EQ(it->Value, 1);

    ++it;
    EXPECT_EQ(static_cast<OffsetNode*>(it), &second);
    EXPECT_EQ(it->Value, 2);

    ++it;
    EXPECT_TRUE(it == list.End());
}

TEST(ZList, DeleteAllDeletesEveryObject)
{
    OffsetNode::DestructorCount = 0;

    ZList<OffsetNode, false, 0> list;
    auto* first = new OffsetNode{ .Value = 1 };
    auto* second = new OffsetNode{ .Value = 2 };

    list.AddLast(first);
    list.AddLast(second);

    list.DeleteAll();

    EXPECT_EQ(OffsetNode::DestructorCount, 2);
    EXPECT_EQ(list.Count(), 0);
    EXPECT_EQ(list.GetFirst(), nullptr);
    EXPECT_EQ(list.GetLast(), nullptr);
}
