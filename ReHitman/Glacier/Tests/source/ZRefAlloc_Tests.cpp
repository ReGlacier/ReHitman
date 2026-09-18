#include <Glacier/ZSTL/ZRefAlloc.h>
#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <stdexcept>
#include <vector>

using namespace Glacier;

namespace
{
    static_assert(sizeof(SRefLink) == 0x8);
    static_assert(sizeof(ZRefAlloc) == 0x4648);

    struct MockWriter final : public IOutputSerializerStream
    {
        std::vector<uint32_t>& m_Buffer;

        explicit MockWriter(std::vector<uint32_t>& buffer) : m_Buffer(buffer) {}

        bool IsSaving() const override { return true; }
        void Skip() override {}
        void ExchangeContainer(const ZToken, unsigned int*) override {}
        void ExchangeRaw(const ZToken, void*, const unsigned int) override {}
        void ExchangeREF(const ZToken, IREFConverter*) override {}
        void ExchangeReftab(const ZToken, REFTAB*) override {}
        ZToken GetToken(const char*) override { return ZToken::Void; }
        void BeginArray(const ZToken, const unsigned int) override {}
        void EndArray() override {}
        void ExchangeHeader(const ZToken, const EPropertyType) override {}
        void ExchangeFooter(const EPropertyType) override {}
        void ExchangeData(ZSerializable*) override {}
        void ExchangeData(ZBitfieldBase*, const ZEnumInfo*) override {}
        void ExchangeData(void*, const ZEnumInfo*) override {}
        void ExchangeData(zstring&) override {}
        void ExchangeData(const char*&) override {}
        void ExchangeData(double&) override {}
        void ExchangeData(float&) override {}
        void ExchangeData(uint8_t&) override {}
        void ExchangeData(int8_t&) override {}
        void ExchangeData(uint16_t&) override {}
        void ExchangeData(int16_t&) override {}
        void ExchangeData(uint32_t& data) override { m_Buffer.push_back(data); }
        void ExchangeData(int32_t& data) override { m_Buffer.push_back(static_cast<uint32_t>(data)); }
        void ExchangeData(bool&) override {}
    };

    struct MockReader final : public IInputSerializerStream
    {
        const std::vector<uint32_t>& m_Buffer;
        size_t m_Offset = 0;

        explicit MockReader(const std::vector<uint32_t>& buffer) : m_Buffer(buffer) {}

        bool IsSaving() const override { return false; }
        bool Visit(ISerializerVisitor*) override { return true; }
        void Skip() override {}
        void ExchangeContainer(const ZToken, unsigned int*) override {}
        void ExchangeRaw(const ZToken, void*, const unsigned int) override {}
        void ExchangeREF(const ZToken, IREFConverter*) override {}
        void ExchangeReftab(const ZToken, REFTAB*) override {}
        ZToken GetToken(const char*) override { return ZToken::Void; }
        void BeginArray(const ZToken, const unsigned int) override {}
        void EndArray() override {}
        void ExchangeHeader(const ZToken, const EPropertyType) override {}
        void ExchangeFooter(const EPropertyType) override {}
        void ExchangeData(ZSerializable*) override {}
        void ExchangeData(ZBitfieldBase*, const ZEnumInfo*) override {}
        void ExchangeData(void*, const ZEnumInfo*) override {}
        void ExchangeData(zstring&) override {}
        void ExchangeData(const char*&) override {}
        void ExchangeData(double&) override {}
        void ExchangeData(float&) override {}
        void ExchangeData(uint8_t&) override {}
        void ExchangeData(int8_t&) override {}
        void ExchangeData(uint16_t&) override {}
        void ExchangeData(int16_t&) override {}
        void ExchangeData(uint32_t& data) override
        {
            ASSERT_LT(m_Offset, m_Buffer.size());
            data = m_Buffer[m_Offset++];
        }
        void ExchangeData(int32_t& data) override
        {
            ASSERT_LT(m_Offset, m_Buffer.size());
            data = static_cast<int32_t>(m_Buffer[m_Offset++]);
        }
        void ExchangeData(bool&) override {}
    };
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

TEST(ZRefAlloc, SaveAndLoadRefChainRoundtrip)
{
    ZRefAlloc alloc1;
    uint32_t chain = alloc1.AddToChain(0, 100);
    chain = alloc1.AddToChain(chain, 200);
    chain = alloc1.AddToChain(chain, 300);
    ASSERT_EQ(alloc1.GetChainLength(chain), 3u);

    std::vector<uint32_t> buffer;
    MockWriter writer(buffer);
    alloc1.SaveRefChain(&writer, chain);
    writer.End();

    ASSERT_EQ(buffer.size(), 4u);
    EXPECT_EQ(buffer[0], 3u);
    EXPECT_EQ(buffer[1], 100u);
    EXPECT_EQ(buffer[2], 300u);
    EXPECT_EQ(buffer[3], 200u);

    MockReader reader(buffer);
    ZRefAlloc alloc2;
    uint32_t loadedChain = alloc2.LoadRefChain(&reader);
    reader.End();

    ASSERT_EQ(alloc2.GetChainLength(loadedChain), 3u);

    std::vector<uint32_t> refs1, refs2;
    for (auto* p = alloc1.GetLink(chain); p; p = alloc1.GetLink(p->m_lNext))
        refs1.push_back(p->m_rRef);
    for (auto* p = alloc2.GetLink(loadedChain); p; p = alloc2.GetLink(p->m_lNext))
        refs2.push_back(p->m_rRef);

    std::sort(refs1.begin(), refs1.end());
    std::sort(refs2.begin(), refs2.end());

    EXPECT_EQ(refs1, refs2);
}

TEST(ZRefAlloc, SaveAndLoadEmptyChain)
{
    ZRefAlloc alloc1;
    ASSERT_EQ(alloc1.GetChainLength(0), 0u);

    std::vector<uint32_t> buffer;
    MockWriter writer(buffer);
    alloc1.SaveRefChain(&writer, 0);
    writer.End();

    MockReader reader(buffer);
    ZRefAlloc alloc2;
    uint32_t loadedChain = alloc2.LoadRefChain(&reader);
    reader.End();

    EXPECT_EQ(loadedChain, 0u);
    EXPECT_EQ(alloc2.GetChainLength(loadedChain), 0u);
    EXPECT_TRUE(alloc2.IsEmpty());
}
