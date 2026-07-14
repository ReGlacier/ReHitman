#include <Glacier/ZSTL/ZList.h>
#include <gtest/gtest.h>

#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZListNodeBase) == 0x8);
    static_assert(sizeof(ZListBase) == 0x8);

    struct TestNode : ZListNode<TestNode, 0>
    {
        int Value = 0;
    };
}

TEST(ZList, ConstructorInitializesEmptySentinel)
{
    ZList<TestNode, false, 0> list;

    EXPECT_EQ(list.Count(), 0);
    EXPECT_EQ(list.GetFirst(), nullptr);
    EXPECT_EQ(list.GetLast(), nullptr);
    EXPECT_EQ(list.m_Head.m_Next, reinterpret_cast<ZListNodeBase*>(&list));
    EXPECT_EQ(list.m_Head.m_Prev, reinterpret_cast<ZListNodeBase*>(&list));
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
