#include <Glacier/Serializer/ZPackedInput.h>
#include <Glacier/Serializer/ZPackedOutput.h>
#include <Glacier/Serializer/ZSerializerVisitor.h>
#include <Glacier/Serializer/ZFastDictionary.h>
#include <Glacier/Runtime/ZEnum.h>
#include <Glacier/ZSTL/zstring.h>
#include <Glacier/ZSTL/ZBitfield.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

using namespace Glacier;

namespace
{
    struct MemoryOutputStream final : public IOutputStream
    {
        std::vector<char> Bytes;

        uint32_t Write(const void* pAddr, const uint32_t lSize) override
        {
            const auto* bytes = static_cast<const char*>(pAddr);
            Bytes.insert(Bytes.end(), bytes, bytes + lSize);
            return lSize;
        }
    };

    struct MemoryInputStream final : public ZInputStreamBase
    {
        const std::vector<char>& Bytes;
        size_t Offset{ 0 };

        explicit MemoryInputStream(const std::vector<char>& bytes)
            : Bytes(bytes)
        {
            m_ChangeEndianness = false;
        }

        uint32_t ReadRaw(char* address, const uint32_t size) override
        {
            const uint32_t remaining = static_cast<uint32_t>(Bytes.size() - Offset);
            const uint32_t readSize = std::min(size, remaining);

            if (readSize != 0)
                std::memcpy(address, Bytes.data() + Offset, readSize);

            Offset += readSize;
            return readSize;
        }

        uint32_t ReadChangeEndianness(char* address, const uint32_t size, const uint32_t mask) override
        {
            uint32_t readSize = 0;
            while (readSize < size && Offset < Bytes.size())
            {
                address[readSize ^ mask] = Bytes[Offset++];
                ++readSize;
            }

            return readSize;
        }
    };

    struct RecordingVisitor final : public ISerializerVisitor
    {
        ZToken Token{ ZToken::Void };
        int32_t Int32Value{ 0 };
        uint32_t RawSize{ 0 };
        uint32_t BeginArrayCount{ 0 };
        std::string StringValue;
        std::string RawData;
        bool SawInt32{ false };
        bool SawRawData{ false };
        bool SawString{ false };
        bool SawBeginArray{ false };
        bool SawEndArray{ false };
        bool SawEnd{ false };

        void Data(const ZToken token, int32_t* data) override
        {
            Token = token;
            Int32Value = *data;
            SawInt32 = true;
        }

        void Data(const ZToken token, const char* data) override
        {
            Token = token;
            StringValue = data;
            SawString = true;
        }

        void Data(const ZToken token, void* data, const uint32_t size) override
        {
            Token = token;
            RawSize = size;
            RawData.assign(static_cast<const char*>(data), static_cast<const char*>(data) + size);
            SawRawData = true;
        }

        void BeginArray(const ZToken token, const uint32_t capacity) override
        {
            Token = token;
            BeginArrayCount = capacity;
            SawBeginArray = true;
        }

        void EndArray() override
        {
            SawEndArray = true;
        }

        void End() override
        {
            SawEnd = true;
        }
    };

    struct EndInputOnScopeExit
    {
        ZPackedInput& Input;

        ~EndInputOnScopeExit()
        {
            if (!Input.m_Finished)
                Input.End();
        }
    };

    template <typename T>
    T ReadLE(const std::vector<char>& bytes, size_t offset)
    {
        T value{};
        std::memcpy(&value, bytes.data() + offset, sizeof(T));
        return value;
    }

    template <typename T>
    void WriteLE(std::vector<char>& bytes, T value)
    {
        const auto* raw = reinterpret_cast<const char*>(&value);
        bytes.insert(bytes.end(), raw, raw + sizeof(T));
    }

    constexpr ISerializerStream::EType ToType(uint32_t type)
    {
        return static_cast<ISerializerStream::EType>(type);
    }

    size_t PayloadOffset()
    {
        return sizeof("IOPacked v0.1") + sizeof(uint8_t) + sizeof(uint32_t) * 3;
    }

    ZEnumInfo MakeBitfieldEnum(ZEnumEntry& flag0, ZEnumEntry& flag2)
    {
        flag0.m_Prev = nullptr;
        flag0.m_Value = 1;
        flag0.m_Name = "flag0";
        flag2.m_Prev = &flag0;
        flag2.m_Value = 4;
        flag2.m_Name = "flag2";

        ZEnumInfo info;
        info.m_Last = &flag2;
        info.m_Size = 4;
        return info;
    }
}

TEST(ZPackedOutput, ConstructorWritesPackedHeader)
{
    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_Tags, ISerializerStream::CONTENT_SimpleRepack, false);
        output.End();
    }

    static constexpr char kSignature[] = "IOPacked v0.1";
    ASSERT_GE(memory.Bytes.size(), sizeof(kSignature) + 9u);
    EXPECT_TRUE(std::equal(std::begin(kSignature), std::end(kSignature), memory.Bytes.begin()));

    size_t offset = sizeof(kSignature);
    EXPECT_EQ(static_cast<uint8_t>(memory.Bytes[offset]), 0u);
    offset += sizeof(uint8_t);

    EXPECT_EQ(ReadLE<uint32_t>(memory.Bytes, offset), static_cast<uint32_t>(ISerializerStream::TYPE_Tags));
    offset += sizeof(uint32_t);

    EXPECT_EQ(ReadLE<uint32_t>(memory.Bytes, offset), static_cast<uint32_t>(ISerializerStream::CONTENT_SimpleRepack));
}

TEST(ZFastDictionary, FirstInsertedWordGetsZeroToken)
{
    ZFastDictionary dictionary;

    EXPECT_EQ(dictionary.GetToken("hello"), ZToken(0));
    EXPECT_EQ(dictionary.GetToken("hello"), ZToken(0));
    EXPECT_EQ(dictionary.GetToken("world"), ZToken(1));
}

TEST(ZPackedOutput, WritesOneByteTagsReadableByExchangePath)
{
    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_Tags, ISerializerStream::CONTENT_SimpleRepack, false);
        int32_t value = -123456;
        output.Exchange(ZToken::Void, value);
        output.End();
    }

    MemoryInputStream inputStream(memory.Bytes);
    ZPackedInput input(&inputStream);
    EndInputOnScopeExit endInput{ input };

    int32_t value = 0;
    input.Exchange(ZToken::Void, value);

    EXPECT_EQ(value, -123456);
}

TEST(ZPackedOutput, RoundTripsDataOnlyPrimitiveWithoutTags)
{
    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_DataOnly, ISerializerStream::CONTENT_SimpleRepack, false);
        int32_t value = 0x12345678;
        output.Exchange(ZToken::Void, value);
        output.End();
    }

    MemoryInputStream inputStream(memory.Bytes);
    ZPackedInput input(&inputStream);
    EndInputOnScopeExit endInput{ input };

    int32_t value = 0;
    input.Exchange(ZToken::Void, value);

    EXPECT_EQ(value, 0x12345678);
}

TEST(ZPackedOutput, RoundTripsBigEndianTaggedPrimitive)
{
    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_Tags, ISerializerStream::CONTENT_SimpleRepack, true);
        int32_t value = 0x12345678;
        output.Exchange(ZToken::Void, value);
        output.End();
    }

    static constexpr char kSignature[] = "IOPacked v0.1";
    ASSERT_GT(memory.Bytes.size(), sizeof(kSignature));
    EXPECT_EQ(static_cast<uint8_t>(memory.Bytes[sizeof(kSignature)]), 1u);

    MemoryInputStream inputStream(memory.Bytes);
    ZPackedInput input(&inputStream);
    EndInputOnScopeExit endInput{ input };

    int32_t value = 0;
    input.Exchange(ZToken::Void, value);

    EXPECT_TRUE(input.m_BigEndian);
    EXPECT_EQ(value, 0x12345678);
}

TEST(ZPackedOutput, RoundTripsDataOnlyPrimitiveMatrix)
{
    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_DataOnly, ISerializerStream::CONTENT_SimpleRepack, false);
        double d = 12.5;
        float f = -3.25f;
        uint8_t u8 = 0xAB;
        int8_t i8 = -12;
        uint16_t u16 = 0xBEEF;
        int16_t i16 = -1234;
        uint32_t u32 = 0xDEADBEEF;
        int32_t i32 = -987654;
        bool b = true;

        output.ExchangeData(d);
        output.ExchangeData(f);
        output.ExchangeData(u8);
        output.ExchangeData(i8);
        output.ExchangeData(u16);
        output.ExchangeData(i16);
        output.ExchangeData(u32);
        output.ExchangeData(i32);
        output.ExchangeData(b);
        output.End();
    }

    MemoryInputStream inputStream(memory.Bytes);
    ZPackedInput input(&inputStream);
    EndInputOnScopeExit endInput{ input };

    double d = 0.0;
    float f = 0.0f;
    uint8_t u8 = 0;
    int8_t i8 = 0;
    uint16_t u16 = 0;
    int16_t i16 = 0;
    uint32_t u32 = 0;
    int32_t i32 = 0;
    bool b = false;

    input.ExchangeData(d);
    input.ExchangeData(f);
    input.ExchangeData(u8);
    input.ExchangeData(i8);
    input.ExchangeData(u16);
    input.ExchangeData(i16);
    input.ExchangeData(u32);
    input.ExchangeData(i32);
    input.ExchangeData(b);

    EXPECT_EQ(d, 12.5);
    EXPECT_EQ(f, -3.25f);
    EXPECT_EQ(u8, 0xAB);
    EXPECT_EQ(i8, -12);
    EXPECT_EQ(u16, 0xBEEF);
    EXPECT_EQ(i16, -1234);
    EXPECT_EQ(u32, 0xDEADBEEF);
    EXPECT_EQ(i32, -987654);
    EXPECT_TRUE(b);
}

TEST(ZPackedOutput, ExternalTemporaryStreamKeepsPayloadSeparate)
{
    MemoryOutputStream mainStream;
    MemoryOutputStream tempStream;
    {
        ZPackedOutput output(mainStream, &tempStream, ISerializerStream::TYPE_DataOnly, ISerializerStream::CONTENT_SimpleRepack, false);
        int32_t value = 0x12345678;
        output.ExchangeData(value);
        output.End();
    }

    ASSERT_EQ(tempStream.Bytes.size(), sizeof(uint32_t));
    EXPECT_EQ(ReadLE<uint32_t>(tempStream.Bytes, 0), 0x12345678u);
    ASSERT_EQ(mainStream.Bytes.size(), PayloadOffset());
    EXPECT_EQ(ReadLE<uint32_t>(mainStream.Bytes, PayloadOffset() - sizeof(uint32_t)), 0u);
}

TEST(ZPackedOutput, WritesBitfieldAsCountedStringNames)
{
    MemoryOutputStream memory;
    {
        ZEnumEntry flag0;
        ZEnumEntry flag2;
        ZEnumInfo info = MakeBitfieldEnum(flag0, flag2);
        ZBitfieldBase bitfield;
        bitfield.SetBitfield((1u << 0) | (1u << 2));

        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_Tags, ISerializerStream::CONTENT_SimpleRepack, false);
        output.ExchangeBitfield(ZToken::Void, bitfield, info);
        output.End();
    }

    size_t offset = PayloadOffset();
    ASSERT_GT(memory.Bytes.size(), offset + 1 + sizeof(uint32_t));
    EXPECT_EQ(static_cast<uint8_t>(memory.Bytes[offset]), static_cast<uint8_t>(ZPackedStream::TAG_Bitfield));
    offset += sizeof(uint8_t);

    EXPECT_EQ(ReadLE<uint32_t>(memory.Bytes, offset), 2u);
    offset += sizeof(uint32_t);

    EXPECT_EQ(ReadLE<uint32_t>(memory.Bytes, offset), 5u);
    offset += sizeof(uint32_t);
    EXPECT_STREQ(memory.Bytes.data() + offset, "flag0");
    offset += 6;

    EXPECT_EQ(ReadLE<uint32_t>(memory.Bytes, offset), 5u);
    offset += sizeof(uint32_t);
    EXPECT_STREQ(memory.Bytes.data() + offset, "flag2");
}

TEST(ZPackedOutput, RoundTripsNamedPrimitiveThroughDictionaryAndVisitor)
{
    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_TagsAndDictionary, ISerializerStream::CONTENT_SimpleRepack, false);
        int32_t value = 9001;
        output.Exchange("score", value);
        output.End();
    }

    MemoryInputStream inputStream(memory.Bytes);
    ZPackedInput input(&inputStream);
    EndInputOnScopeExit endInput{ input };
    RecordingVisitor visitor;

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawInt32);
    EXPECT_EQ(visitor.Token, input.GetToken("score"));
    EXPECT_EQ(visitor.Int32Value, 9001);

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawEnd);

}

TEST(ZPackedOutput, RoundTripsRawDataAndArrayControlTags)
{
    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_Tags, ISerializerStream::CONTENT_SimpleRepack, false);
        char raw[] = { 'd', 'a', 't', 'a' };
        output.ExchangeRaw(ZToken::Void, raw, sizeof(raw));
        output.BeginArray(ZToken::Void, 2);
        output.EndArray();
        output.End();
    }

    MemoryInputStream inputStream(memory.Bytes);
    ZPackedInput input(&inputStream);
    EndInputOnScopeExit endInput{ input };
    RecordingVisitor visitor;

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawRawData);
    EXPECT_EQ(visitor.RawSize, 4u);
    EXPECT_EQ(visitor.RawData, "data");

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawBeginArray);
    EXPECT_EQ(visitor.BeginArrayCount, 2u);

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawEndArray);

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawEnd);

}

TEST(ZPackedOutput, RoundTripsZStringThroughStringTable)
{
    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ToType(ISerializerStream::TYPE_Tags | ISerializerStream::TYPE_StringTable), ISerializerStream::CONTENT_SimpleRepack, false);
        zstring value("hello");
        output.ExchangeHeader(ZToken::Void, ISerializerStream::PT_String);
        output.ExchangeData(value);
        output.ExchangeFooter(ISerializerStream::PT_String);
        output.End();
    }

    MemoryInputStream inputStream(memory.Bytes);
    ZPackedInput input(&inputStream);
    EndInputOnScopeExit endInput{ input };
    RecordingVisitor visitor;

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawString);
    EXPECT_EQ(visitor.StringValue, "hello");

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawEnd);

}

TEST(ZPackedOutput, RoundTripsCStringThroughStringTable)
{
    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ToType(ISerializerStream::TYPE_Tags | ISerializerStream::TYPE_StringTable), ISerializerStream::CONTENT_SimpleRepack, false);
        const char* value = "world";
        output.Exchange(ZToken::Void, value);
        output.End();
    }

    MemoryInputStream inputStream(memory.Bytes);
    ZPackedInput input(&inputStream);
    EndInputOnScopeExit endInput{ input };
    RecordingVisitor visitor;

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawString);
    EXPECT_EQ(visitor.StringValue, "world");

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawEnd);

}
