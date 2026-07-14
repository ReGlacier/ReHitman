#include <Glacier/Serializer/ZPackedInput.h>
#include <Glacier/Serializer/ZSerializerVisitor.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

using namespace Glacier;

namespace
{
    struct MemoryPackedInputStream final : public ZInputStreamBase
    {
        std::vector<char> Bytes;
        size_t Offset{ 0 };

        explicit MemoryPackedInputStream(std::vector<char> bytes)
            : Bytes(std::move(bytes))
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

    template <typename T>
    void Append(std::vector<char>& bytes, const T& value)
    {
        const auto* raw = reinterpret_cast<const char*>(&value);
        bytes.insert(bytes.end(), raw, raw + sizeof(T));
    }

    void AppendHeader(std::vector<char>& bytes, ISerializerStream::EType type = ISerializerStream::TYPE_Tags)
    {
        static constexpr char kSignature[] = "IOPacked v0.1";
        bytes.insert(bytes.end(), std::begin(kSignature), std::end(kSignature));

        Append<uint8_t>(bytes, 0u);
        Append<uint32_t>(bytes, static_cast<uint32_t>(type));
        Append<uint32_t>(bytes, static_cast<uint32_t>(ISerializerStream::CONTENT_SimpleRepack));
        Append<uint32_t>(bytes, 0u);
    }

    struct RecordingVisitor final : public ISerializerVisitor
    {
        ZToken Token{ ZToken::Void };
        uint32_t ReferenceValue{ 0 };
        int32_t Int32Value{ 0 };
        uint32_t BeginArrayCount{ 0 };
        uint32_t RawSize{ 0 };
        std::string StringValue;
        std::string RawData;
        bool SawInt32{ false };
        bool SawBeginArray{ false };
        bool SawReference{ false };
        bool SawSkip{ false };
        bool SawEndObject{ false };
        bool SawEnd{ false };

        void Data(const ZToken token, const char* data) override
        {
            Token = token;
            StringValue = data;
        }

        void Data(const ZToken token, int32_t* data) override
        {
            Token = token;
            Int32Value = *data;
            SawInt32 = true;
        }

        void Data(const ZToken token, void* data, const uint32_t size) override
        {
            Token = token;
            RawSize = size;
            RawData.assign(static_cast<const char*>(data), static_cast<const char*>(data) + size);
        }

        void BeginArray(const ZToken token, const uint32_t capacity) override
        {
            Token = token;
            BeginArrayCount = capacity;
            SawBeginArray = true;
        }

        void Reference(const ZToken token, uint32_t ref) override
        {
            Token = token;
            ReferenceValue = ref;
            SawReference = true;
        }

        void Skip() override
        {
            SawSkip = true;
        }

        void EndObject() override
        {
            SawEndObject = true;
        }

        void End() override
        {
            SawEnd = true;
        }
    };
}

TEST(ZPackedInput, InitializesFromPackedHeader)
{
    std::vector<char> bytes;
    AppendHeader(bytes, ISerializerStream::TYPE_DataOnly);

    MemoryPackedInputStream stream(std::move(bytes));
    ZPackedInput input(&stream);

    EXPECT_EQ(input.m_Input, &stream);
    EXPECT_EQ(input.m_DataStream, &stream);
    EXPECT_EQ(input.m_Type, ISerializerStream::TYPE_DataOnly);
    EXPECT_EQ(input.m_Content, ISerializerStream::CONTENT_SimpleRepack);
    EXPECT_FALSE(input.m_BigEndian);
    EXPECT_FALSE(stream.m_ChangeEndianness);

    input.End();
}

TEST(ZPackedInput, ReadsPayloadFromSeparateDataStreamWhenProvided)
{
    std::vector<char> headerBytes;
    AppendHeader(headerBytes);

    std::vector<char> dataBytes;
    Append<uint32_t>(dataBytes, static_cast<uint32_t>(ZPackedStream::TAG_Int32));
    Append<int32_t>(dataBytes, 314159);

    MemoryPackedInputStream headerStream(std::move(headerBytes));
    MemoryPackedInputStream dataStream(std::move(dataBytes));
    ZPackedInput input(&headerStream, &dataStream);

    int32_t value = 0;
    input.Exchange(ZToken::Void, value);

    EXPECT_EQ(input.m_Input, &headerStream);
    EXPECT_EQ(input.m_DataStream, &dataStream);
    EXPECT_EQ(value, 314159);
    EXPECT_EQ(headerStream.Offset, headerStream.Bytes.size());
    EXPECT_EQ(dataStream.Offset, dataStream.Bytes.size());

    input.End();
}

TEST(ZPackedInput, ExchangeContainerReadsStoredCount)
{
    std::vector<char> bytes;
    AppendHeader(bytes);
    Append<uint32_t>(bytes, static_cast<uint32_t>(ZPackedStream::TAG_Container));
    Append<uint32_t>(bytes, 12u);

    MemoryPackedInputStream stream(std::move(bytes));
    ZPackedInput input(&stream);

    uint32_t count = 0;
    input.ExchangeContainer(ZToken::Void, &count);

    EXPECT_EQ(count, 12u);

    input.End();
}

TEST(ZPackedInput, ExchangeReadsPrimitiveBehindTag)
{
    std::vector<char> bytes;
    AppendHeader(bytes);
    Append<uint32_t>(bytes, static_cast<uint32_t>(ZPackedStream::TAG_Int32));
    Append<int32_t>(bytes, -123456);

    MemoryPackedInputStream stream(std::move(bytes));
    ZPackedInput input(&stream);

    int32_t value = 0;
    input.Exchange(ZToken::Void, value);

    EXPECT_EQ(value, -123456);

    input.End();
}

TEST(ZPackedInput, VisitReadsReferenceAndControlEvents)
{
    std::vector<char> bytes;
    AppendHeader(bytes);
    Append<uint8_t>(bytes, static_cast<uint8_t>(ZPackedStream::TAG_NamedReference));
    Append<uint32_t>(bytes, 88u);
    Append<uint32_t>(bytes, 1234u);
    Append<uint8_t>(bytes, static_cast<uint8_t>(ZPackedStream::TAG_SkipMark));
    Append<uint8_t>(bytes, static_cast<uint8_t>(ZPackedStream::TAG_EndObject));

    MemoryPackedInputStream stream(std::move(bytes));
    ZPackedInput input(&stream);
    RecordingVisitor visitor;

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawReference);
    EXPECT_EQ(visitor.Token, ZToken(88));
    EXPECT_EQ(visitor.ReferenceValue, 1234u);

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawSkip);

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawEndObject);

    input.End();
}

TEST(ZPackedInput, VisitReadsInlineStringWithoutOverwritingStack)
{
    std::vector<char> bytes;
    AppendHeader(bytes);
    Append<uint8_t>(bytes, static_cast<uint8_t>(ZPackedStream::TAG_NamedString));
    Append<uint32_t>(bytes, 44u);
    Append<uint32_t>(bytes, 5u);
    bytes.insert(bytes.end(), { 'h', 'e', 'l', 'l', 'o' });

    MemoryPackedInputStream stream(std::move(bytes));
    ZPackedInput input(&stream);
    RecordingVisitor visitor;

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_EQ(visitor.Token, ZToken(44));
    EXPECT_EQ(visitor.StringValue, "hello");
    EXPECT_EQ(stream.Offset, stream.Bytes.size());

    input.End();
}

TEST(ZPackedInput, ExchangeRawReadsStoredSizeAndPayload)
{
    std::vector<char> bytes;
    AppendHeader(bytes);
    Append<uint32_t>(bytes, static_cast<uint32_t>(ZPackedStream::TAG_RawData));
    Append<uint32_t>(bytes, 4u);
    bytes.insert(bytes.end(), { 'd', 'a', 't', 'a' });

    MemoryPackedInputStream stream(std::move(bytes));
    ZPackedInput input(&stream);

    char output[4]{};
    input.ExchangeRaw(ZToken::Void, output, sizeof(output));

    EXPECT_EQ(std::memcmp(output, "data", sizeof(output)), 0);

    input.End();
}

TEST(ZPackedInput, ExchangeArrayReadsArrayTagsAndValues)
{
    std::vector<char> bytes;
    AppendHeader(bytes);
    Append<uint32_t>(bytes, static_cast<uint32_t>(ZPackedStream::TAG_Array));
    Append<uint32_t>(bytes, 2u);
    Append<uint32_t>(bytes, static_cast<uint32_t>(ZPackedStream::TAG_Int32));
    Append<uint32_t>(bytes, 10u);
    Append<uint32_t>(bytes, static_cast<uint32_t>(ZPackedStream::TAG_Int32));
    Append<uint32_t>(bytes, 20u);
    Append<uint32_t>(bytes, static_cast<uint32_t>(ZPackedStream::TAG_EndArray));

    MemoryPackedInputStream stream(std::move(bytes));
    ZPackedInput input(&stream);

    uint32_t values[2]{};
    input.ExchangeArray(ZToken::Void, values, 2);

    EXPECT_EQ(values[0], 10u);
    EXPECT_EQ(values[1], 20u);

    input.End();
}

TEST(ZPackedInput, VisitReadsNamedPrimitiveEvent)
{
    std::vector<char> bytes;
    AppendHeader(bytes);
    Append<uint8_t>(bytes, static_cast<uint8_t>(ZPackedStream::TAG_NamedInt32));
    Append<uint32_t>(bytes, 77u);
    Append<int32_t>(bytes, 9001);

    MemoryPackedInputStream stream(std::move(bytes));
    ZPackedInput input(&stream);
    RecordingVisitor visitor;

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawInt32);
    EXPECT_EQ(visitor.Token, ZToken(77));
    EXPECT_EQ(visitor.Int32Value, 9001);

    input.End();
}

TEST(ZPackedInput, VisitReadsArrayRawDataAndEndEvents)
{
    std::vector<char> bytes;
    AppendHeader(bytes);
    Append<uint8_t>(bytes, static_cast<uint8_t>(ZPackedStream::TAG_Array));
    Append<uint32_t>(bytes, 3u);
    Append<uint8_t>(bytes, static_cast<uint8_t>(ZPackedStream::TAG_RawData));
    Append<uint32_t>(bytes, 3u);
    bytes.insert(bytes.end(), { 'r', 'a', 'w' });
    Append<uint8_t>(bytes, static_cast<uint8_t>(ZPackedStream::TAG_EndOfStream));

    MemoryPackedInputStream stream(std::move(bytes));
    ZPackedInput input(&stream);
    RecordingVisitor visitor;

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawBeginArray);
    EXPECT_EQ(visitor.BeginArrayCount, 3u);

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_EQ(visitor.RawSize, 3u);
    EXPECT_EQ(visitor.RawData, "raw");

    EXPECT_TRUE(input.Visit(&visitor));
    EXPECT_TRUE(visitor.SawEnd);

    input.End();
}
