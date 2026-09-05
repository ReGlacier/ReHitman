#include <Glacier/Serializer/IOutputSerializerStream.h>
#include <Glacier/Serializer/MaskUtils.h>
#include <Glacier/Serializer/ZSerializerVisitor.h>
#include <Glacier/Serializer/ZTokenCache.h>
#include <Glacier/Runtime/ZEnum.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZBitfield.h>

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Glacier;

namespace
{
    struct RecordingOutputStream final : public IOutputSerializerStream
    {
        using ISerializerStream::GetToken;

        std::vector<std::string> Events;
        std::vector<ZToken> HeaderTokens;
        std::vector<ISerializerStream::EPropertyType> HeaderTypes;
        std::vector<uint32_t> UInt32Values;
        std::vector<uint32_t> ContainerCounts;
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

        void Skip() override { Events.emplace_back("Skip"); }

        void ExchangeContainer(const ZToken token, unsigned int* count) override
        {
            HeaderTokens.push_back(token);
            ContainerCounts.push_back(*count);
            Events.emplace_back("Container");
        }

        void ExchangeRaw(const ZToken token, void*, const unsigned int size) override
        {
            HeaderTokens.push_back(token);
            UInt32Values.push_back(size);
            Events.emplace_back("Raw");
        }

        ZToken GetToken(const char*) override { return ZToken(static_cast<int32_t>(NextToken++)); }
        void BeginArray(const ZToken token, const unsigned int count) override
        {
            HeaderTokens.push_back(token);
            ContainerCounts.push_back(count);
            Events.emplace_back("BeginArray");
        }

        void EndArray() override { Events.emplace_back("EndArray"); }

        void ExchangeHeader(const ZToken token, const EPropertyType type) override
        {
            HeaderTokens.push_back(token);
            HeaderTypes.push_back(type);
            Events.emplace_back("Header");
        }

        void ExchangeFooter(const EPropertyType type) override
        {
            HeaderTypes.push_back(type);
            Events.emplace_back("Footer");
        }

        void ExchangeData(ZSerializable*) override {}
        void ExchangeData(ZBitfieldBase*, const ZEnumInfo*) override {}
        void ExchangeData(void*, const ZEnumInfo*) override {}
        void ExchangeData(zstring&) override {}
        void ExchangeData(const char*&) override {}
        void ExchangeData(double&) override {}
        void ExchangeData(float&) override {}
        void ExchangeData(uint8_t& value) override { UInt32Values.push_back(value); Events.emplace_back("U8"); }
        void ExchangeData(int8_t& value) override { UInt32Values.push_back(static_cast<uint32_t>(value)); Events.emplace_back("I8"); }
        void ExchangeData(uint16_t& value) override { UInt32Values.push_back(value); Events.emplace_back("U16"); }
        void ExchangeData(int16_t& value) override { UInt32Values.push_back(static_cast<uint32_t>(value)); Events.emplace_back("I16"); }
        void ExchangeData(uint32_t& value) override { UInt32Values.push_back(value); Events.emplace_back("U32"); }
        void ExchangeData(int32_t& value) override { UInt32Values.push_back(static_cast<uint32_t>(value)); Events.emplace_back("I32"); }
        void ExchangeData(bool& value) override { UInt32Values.push_back(value ? 1u : 0u); Events.emplace_back("Bool"); }
    };

    struct RefConverter final : public ISerializerStream::IREFConverter
    {
        ZREF Value{ 0x1234u };
        ZREF GetRef() override { return Value; }
        void SetRef(ZREF ref) override { Value = ref; }
    };
}

TEST(MaskUtils, EndiannessMasksMatchElementWidth)
{
    EXPECT_EQ(GetEndiannessMask<uint8_t>(), 0u);
    EXPECT_EQ(GetEndiannessMask<int8_t>(), 0u);
    EXPECT_EQ(GetEndiannessMask<uint16_t>(), 1u);
    EXPECT_EQ(GetEndiannessMask<uint32_t>(), 3u);
    EXPECT_EQ(GetEndiannessMask<uint64_t>(), 7u);
}

TEST(ZEnumInfo, FindsItemsByValueAndTypedDataPointer)
{
    ZEnumEntry first{ nullptr, 1, "one" };
    ZEnumEntry second{ &first, 2, "two" };
    ZEnumEntry third{ &second, 4, "four" };

    ZEnumInfo info;
    info.m_Last = &third;
    info.m_Size = 4;

    EXPECT_STREQ(info.FindItem(1), "one");
    EXPECT_STREQ(info.FindItem(2), "two");
    EXPECT_STREQ(info.FindItem(4), "four");
    EXPECT_EQ(info.FindItem(8), nullptr);

    uint8_t value8 = 1;
    uint16_t value16 = 2;
    uint32_t value32 = 4;

    void* data = &value8;
    info.m_Size = 1;
    EXPECT_STREQ(info.FindItem(&data), "one");

    data = &value16;
    info.m_Size = 2;
    EXPECT_STREQ(info.FindItem(&data), "two");

    data = &value32;
    info.m_Size = 4;
    EXPECT_STREQ(info.FindItem(&data), "four");
}

TEST(ZEnumInfo, InvalidTypedDataSizeAsserts)
{
    ZEnumEntry entry{ nullptr, 1, "one" };
    ZEnumInfo info;
    info.m_Last = &entry;
    info.m_Size = 3;

    uint32_t value = 1;
    void* data = &value;

    EXPECT_THROW(info.FindItem(&data), std::runtime_error);
}

TEST(ZBitfieldBase, StoresAndTestsBits)
{
    ZBitfieldBase bitfield;
    bitfield.SetBitfield((1u << 0) | (1u << 31));

    EXPECT_EQ(bitfield.GetBitfield(), 0x80000001u);
    EXPECT_TRUE(bitfield.TestBit(0));
    EXPECT_FALSE(bitfield.TestBit(1));
    EXPECT_TRUE(bitfield.TestBit(31));
}

TEST(ISerializerStream, CheckTypeAndStreamFilterUsePackedFlags)
{
    RecordingOutputStream stream;

    EXPECT_TRUE(stream.IsSaving());
    EXPECT_FALSE(stream.IsLoading());
    EXPECT_TRUE(stream.CheckType(ISerializerStream::TYPE_Tags));
    EXPECT_TRUE(stream.CheckType(ISerializerStream::TYPE_Dictionary));
    EXPECT_FALSE(stream.CheckType(ISerializerStream::TYPE_StringTable));
    EXPECT_TRUE(stream.TestStreamFilter(1u << ISerializerStream::CONTENT_SimpleRepack));
    EXPECT_FALSE(stream.TestStreamFilter(1u << ISerializerStream::CONTENT_SavedGame));
}

TEST(ISerializerStream, TokenCacheCachesPerStreamAndFlushesOnEnd)
{
    ZTokenCache cache("field");
    {
        RecordingOutputStream stream;

        EXPECT_EQ(stream.GetToken(cache), ZToken(0));
        EXPECT_EQ(stream.GetToken(cache), ZToken(0));
        EXPECT_EQ(stream.NextToken, 1u);
        EXPECT_EQ(cache.m_Stream, &stream);
    }

    EXPECT_EQ(cache.m_Stream, nullptr);
}

TEST(ISerializerStream, ExchangePrimitiveWrapsHeaderDataAndFooter)
{
    RecordingOutputStream stream;
    int32_t value = -7;

    stream.Exchange(ZToken(42), value);

    ASSERT_EQ(stream.Events.size(), 3u);
    EXPECT_EQ(stream.Events[0], "Header");
    EXPECT_EQ(stream.Events[1], "I32");
    EXPECT_EQ(stream.Events[2], "Footer");
    EXPECT_EQ(stream.HeaderTokens[0], ZToken(42));
    EXPECT_EQ(stream.HeaderTypes[0], ISerializerStream::PT_Int32);
    EXPECT_EQ(stream.HeaderTypes[1], ISerializerStream::PT_Int32);
    EXPECT_EQ(static_cast<int32_t>(stream.UInt32Values[0]), -7);
}

TEST(IOutputSerializerStream, ExchangeREFWritesConvertedReferenceAsUInt32)
{
    RecordingOutputStream stream;
    RefConverter converter;

    stream.ExchangeREF(ZToken(5), &converter);

    ASSERT_EQ(stream.Events.size(), 3u);
    EXPECT_EQ(stream.HeaderTokens[0], ZToken(5));
    EXPECT_EQ(stream.HeaderTypes[0], ISerializerStream::PT_UInt32);
    EXPECT_EQ(stream.UInt32Values[0], 0x1234u);
}

TEST(IOutputSerializerStream, SaveRawAndSaveContainerResolveTokenNames)
{
    RecordingOutputStream stream;
    uint32_t capacity = 9u;
    char raw[3]{};

    stream.SaveRaw("raw", raw, sizeof(raw));
    stream.SaveContainer("items", capacity);

    ASSERT_EQ(stream.Events.size(), 2u);
    EXPECT_EQ(stream.Events[0], "Raw");
    EXPECT_EQ(stream.Events[1], "Container");
    EXPECT_EQ(stream.HeaderTokens[0], ZToken(0));
    EXPECT_EQ(stream.HeaderTokens[1], ZToken(1));
    EXPECT_EQ(stream.UInt32Values[0], 3u);
    EXPECT_EQ(stream.ContainerCounts[0], 9u);
}

TEST(ZSerializerVisitorSkip, TracksObjectDepthAndStopsAtEnd)
{
    ZSerializerVisitor_SkipObject visitor;
    EXPECT_FALSE(static_cast<bool>(visitor));

    visitor.BeginObject(ZToken::Void);
    EXPECT_TRUE(static_cast<bool>(visitor));

    int32_t value = 1;
    EXPECT_NO_THROW(visitor.Data(ZToken::Void, &value));

    visitor.EndObject();
    EXPECT_FALSE(static_cast<bool>(visitor));
    EXPECT_NO_THROW(visitor.End());
}

TEST(ZSerializerVisitorSkip, RejectsDataOutsideSkippedObject)
{
    ZSerializerVisitor_SkipObject visitor;
    int32_t value = 1;

    EXPECT_THROW(visitor.Data(ZToken::Void, &value), std::runtime_error);
}

TEST(ZSerializerVisitorSkipToNextMark, StopsWhenSkipMarkIsSeen)
{
    ZSerializerVisitor_SkipToNextMark visitor;
    EXPECT_TRUE(static_cast<bool>(visitor));

    visitor.BeginObject(ZToken::Void);
    visitor.EndObject();
    EXPECT_TRUE(static_cast<bool>(visitor));

    visitor.Skip();
    EXPECT_FALSE(static_cast<bool>(visitor));
}
