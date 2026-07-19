#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace Glacier;

namespace
{
    static_assert(sizeof(ZEventBuffer) == 0x1C);
    static_assert(sizeof(ZEventBase) == 0x2C);

    struct TestEvent : public ZEventBase
    {
        const char* EventName() override { return "TestEvent"; }
    };

    struct RecordingOutputStream final : public IOutputSerializerStream
    {
        std::vector<std::string> Events;
        std::vector<uint32_t> ContainerCounts;
        std::vector<uint32_t> UInt32Values;
        std::vector<std::string> StringValues;
        uint32_t NextToken{ 0 };

        RecordingOutputStream()
            : IOutputSerializerStream(TYPE_TagsAndDictionary, CONTENT_SimpleRepack)
        {
        }

        ~RecordingOutputStream() override
        {
            if (!m_Finished)
                End();
        }

        void Skip() override {}
        void ExchangeContainer(const ZToken, unsigned int* count) override
        {
            ContainerCounts.push_back(*count);
            Events.emplace_back("Container");
        }
        void ExchangeRaw(const ZToken, void*, const unsigned int) override {}
        ZToken GetToken(const char*) override { return ZToken(static_cast<int32_t>(NextToken++)); }
        void BeginArray(const ZToken, const unsigned int count) override
        {
            ContainerCounts.push_back(count);
            Events.emplace_back("BeginArray");
        }
        void EndArray() override { Events.emplace_back("EndArray"); }
        void ExchangeHeader(const ZToken, const EPropertyType) override { Events.emplace_back("Header"); }
        void ExchangeFooter(const EPropertyType) override { Events.emplace_back("Footer"); }
        void ExchangeData(ZSerializable*) override {}
        void ExchangeData(ZBitfieldBase*, const ZEnumInfo*) override {}
        void ExchangeData(void*, const ZEnumInfo*) override {}
        void ExchangeData(zstring&) override {}
        void ExchangeData(const char*& value) override
        {
            StringValues.emplace_back(value ? value : "");
            Events.emplace_back("String");
        }
        void ExchangeData(double&) override {}
        void ExchangeData(float&) override {}
        void ExchangeData(uint8_t&) override {}
        void ExchangeData(int8_t&) override {}
        void ExchangeData(uint16_t&) override {}
        void ExchangeData(int16_t&) override {}
        void ExchangeData(uint32_t& value) override
        {
            UInt32Values.push_back(value);
            Events.emplace_back("U32");
        }
        void ExchangeData(int32_t&) override {}
        void ExchangeData(bool&) override {}
    };
}

TEST(ZEventBuffer, ConstructorInitializesAlignedEventRamAndRefs)
{
    ZEventBuffer buffer(30);

    EXPECT_EQ(buffer.m_lEventRamSize, 32u);
    ASSERT_NE(buffer.m_pEventRam, nullptr);
    ASSERT_NE(buffer.m_pEventAlloc, nullptr);
    ASSERT_NE(buffer.m_EventRefs, nullptr);
    EXPECT_EQ(buffer.m_lAllocatedEventsRam, 0u);
    EXPECT_EQ(buffer.m_lNrAllocatedEvents, 0u);
}

TEST(ZEventBuffer, AllocAndFreeEventRamTrackAlignedSizes)
{
    ZEventBuffer buffer(128);

    ZEventBase* event = buffer.AllocEventRam(0x2C);

    ASSERT_NE(event, nullptr);
    EXPECT_EQ(event->m_lEventAllocSize, 0x2Cu >> 2);
    EXPECT_EQ(buffer.m_lAllocatedEventsRam, 0x2Cu);
    EXPECT_EQ(buffer.m_lNrAllocatedEvents, 1u);

    buffer.FreeEventRam(event);

    EXPECT_EQ(buffer.m_lAllocatedEventsRam, 0u);
    EXPECT_EQ(buffer.m_lNrAllocatedEvents, 0u);
}

TEST(ZEventBuffer, AllocRefStoresAndResolvesEventPointer)
{
    ZEventBuffer buffer(128);
    alignas(TestEvent) unsigned char eventStorage[sizeof(TestEvent)]{};
    TestEvent* event = reinterpret_cast<TestEvent*>(eventStorage);

    ZREF ref = buffer.AllocRef(event);

    EXPECT_EQ(buffer.ConvEventRefToPtr(ref), event);

    buffer.FreeRef(ref);
    EXPECT_EQ(buffer.m_EventRefs->Count(), 0u);
}

TEST(ZEventBuffer, AllocRefDirectStoresRequestedReference)
{
    ZEventBuffer buffer(128);
    alignas(TestEvent) unsigned char eventStorage[sizeof(TestEvent)]{};
    TestEvent* event = reinterpret_cast<TestEvent*>(eventStorage);

    int returnedRef = buffer.AllocRefDirect(event, 5);

    EXPECT_EQ(returnedRef, 6);
    EXPECT_EQ(buffer.ConvEventRefToPtr(5), event);
}

TEST(ZEventBuffer, SaveObjectWritesEventRefsAndTypes)
{
    ZEventBuffer buffer(128);
    TestEvent event;
    alignas(ZGEOM) unsigned char geomStorage[sizeof(ZGEOM)]{};
    ZGEOM* geom = reinterpret_cast<ZGEOM*>(geomStorage);
    geom->m_baseGeom = nullptr;
    event.m_pBaseGeom = geom;
    ZREF ref = event.GetRef();

    RecordingOutputStream stream;

    buffer.SaveObject(stream);

    ASSERT_FALSE(stream.ContainerCounts.empty());
    EXPECT_EQ(stream.ContainerCounts[0], 1u);
    ASSERT_FALSE(stream.UInt32Values.empty());
    EXPECT_EQ(stream.UInt32Values[0], static_cast<uint32_t>(ref));
    ASSERT_FALSE(stream.StringValues.empty());
    EXPECT_EQ(stream.StringValues[0], "TestEvent");
}
