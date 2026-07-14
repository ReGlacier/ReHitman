#include <Glacier/ZSTL/ZRefAlloc.h>
#include <gtest/gtest.h>

#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(SRefLink) == 0x8);
    static_assert(sizeof(ZRefAlloc) == 0x4648);
}

TEST(ZRefAlloc, ConstructorInitializesFreeList)
{
    ZRefAlloc alloc;

    EXPECT_TRUE(alloc.IsEmpty());
    EXPECT_EQ(alloc.m_lNumLinks, 0);
    EXPECT_EQ(alloc.m_lNextFree, 1);
    EXPECT_EQ(alloc.m_Links[1].m_lNext, 2u | ZRefAlloc::FREE_LINK_FLAG);
    EXPECT_EQ(alloc.m_Links[ZRefAlloc::LAST_REF_LINK].m_lNext, 0u);
}

TEST(ZRefAlloc, AllocRefTakesNextFreeLink)
{
    ZRefAlloc alloc;

    uint32_t first = alloc.AllocRef();
    uint32_t second = alloc.AllocRef();

    EXPECT_EQ(first, 1u);
    EXPECT_EQ(second, 2u);
    EXPECT_EQ(alloc.m_lNumLinks, 2);
    EXPECT_EQ(alloc.m_lNextFree, 3);
    EXPECT_EQ(alloc.m_Links[first].m_lNext, 0xFFFFFFFEu);
}

TEST(ZRefAlloc, AllocRefUsesPcLinkRangeThroughLastLink)
{
    ZRefAlloc alloc;
    uint32_t last = 0;

    for (uint32_t i = ZRefAlloc::FIRST_REF_LINK; i <= ZRefAlloc::LAST_REF_LINK; ++i)
        last = alloc.AllocRef();

    EXPECT_EQ(last, ZRefAlloc::LAST_REF_LINK);
    EXPECT_EQ(alloc.m_lNumLinks, static_cast<int>(ZRefAlloc::LAST_REF_LINK));
    EXPECT_EQ(alloc.m_lNextFree, 0);
    EXPECT_THROW(alloc.AllocRef(), std::runtime_error);
}

TEST(ZRefAlloc, FreeRefReturnsLinkToFreeList)
{
    ZRefAlloc alloc;
    uint32_t first = alloc.AllocRef();
    uint32_t second = alloc.AllocRef();

    alloc.FreeRef(second);

    EXPECT_EQ(alloc.m_lNumLinks, 1);
    EXPECT_EQ(alloc.m_lNextFree, static_cast<int>(second));
    EXPECT_EQ(alloc.m_Links[second].m_lNext, first + 2u | ZRefAlloc::FREE_LINK_FLAG);
}

TEST(ZRefAlloc, FreeRefRejectsInvalidSentinels)
{
    ZRefAlloc alloc;

    EXPECT_THROW(alloc.FreeRef(0), std::runtime_error);
    EXPECT_THROW(alloc.FreeRef(ZRefAlloc::REFCHAIN_END), std::runtime_error);
}

TEST(ZRefAlloc, AddToChainCreatesAndPrependsAfterHead)
{
    ZRefAlloc alloc;

    uint32_t chain = alloc.AddToChain(0, 100);
    chain = alloc.AddToChain(chain, 200);
    chain = alloc.AddToChain(chain, 300);

    ASSERT_EQ(chain, 1u);
    EXPECT_EQ(alloc.GetChainLength(chain), 3u);
    ASSERT_NE(alloc.GetLink(chain), nullptr);
    EXPECT_EQ(alloc.GetLink(chain)->m_rRef, 100u);

    uint32_t second = alloc.GetLink(chain)->m_lNext;
    ASSERT_NE(alloc.GetLink(second), nullptr);
    EXPECT_EQ(alloc.GetLink(second)->m_rRef, 300u);

    uint32_t third = alloc.GetLink(second)->m_lNext;
    ASSERT_NE(alloc.GetLink(third), nullptr);
    EXPECT_EQ(alloc.GetLink(third)->m_rRef, 200u);
    EXPECT_EQ(alloc.GetLink(third)->m_lNext, ZRefAlloc::REFCHAIN_END);
}

TEST(ZRefAlloc, AddToChainWithEndSentinelCreatesNewChain)
{
    ZRefAlloc alloc;

    uint32_t chain = alloc.AddToChain(ZRefAlloc::REFCHAIN_END, 55);

    EXPECT_EQ(chain, 1u);
    ASSERT_NE(alloc.GetLink(chain), nullptr);
    EXPECT_EQ(alloc.GetLink(chain)->m_rRef, 55u);
    EXPECT_EQ(alloc.GetLink(chain)->m_lNext, ZRefAlloc::REFCHAIN_END);
}

TEST(ZRefAlloc, FreeChainReturnsEveryLinkToFreeList)
{
    ZRefAlloc alloc;

    uint32_t chain = alloc.AddToChain(0, 100);
    chain = alloc.AddToChain(chain, 200);
    chain = alloc.AddToChain(chain, 300);
    ASSERT_EQ(alloc.m_lNumLinks, 3);

    alloc.FreeChain(chain);

    EXPECT_TRUE(alloc.IsEmpty());
    EXPECT_EQ(alloc.m_lNextFree, 2);
}

TEST(ZRefAlloc, GetLinkReturnsNullForSentinels)
{
    ZRefAlloc alloc;

    EXPECT_EQ(alloc.GetLink(0), nullptr);
    EXPECT_EQ(alloc.GetLink(ZRefAlloc::REFCHAIN_END), nullptr);
}

TEST(ZRefAlloc, SerializerMethodsAreNotReversedYet)
{
    ZRefAlloc alloc;

    EXPECT_THROW(alloc.SaveRefChain(nullptr, 0), std::runtime_error);
    EXPECT_THROW(alloc.LoadRefChain(nullptr), std::runtime_error);
}
