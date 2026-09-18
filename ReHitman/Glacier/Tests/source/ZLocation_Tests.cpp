#include <Glacier/PF4/ZLocation.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

using namespace Glacier;

namespace
{
    struct RecordingStream final : ISerializerStream
    {
        std::vector<std::string> Tokens;
        std::vector<EPropertyType> HeaderTypes;
        std::vector<std::string> Events;
        std::vector<int16_t> Int16Values;
        std::vector<float> FloatValues;
        bool Saving = true;

        explicit RecordingStream(bool saving = true)
            : ISerializerStream(TYPE_TagsAndDictionary, CONTENT_SimpleRepack)
            , Saving(saving)
        {
        }

        ~RecordingStream() override
        {
            if (!m_Finished)
                End();
        }

        bool IsSaving() const override { return Saving; }
        void Skip() override {}
        void ExchangeContainer(const ZToken, unsigned int*) override {}
        void ExchangeRaw(const ZToken, void*, const unsigned int) override {}
        void ExchangeREF(const ZToken, IREFConverter*) override {}
        void ExchangeReftab(const ZToken, REFTAB*) override {}
        ZToken GetToken(const char* psToken) override
        {
            Tokens.emplace_back(psToken);
            return ZToken(static_cast<int32_t>(Tokens.size() - 1));
        }
        void BeginArray(const ZToken, const unsigned int count) override
        {
            Events.emplace_back("BeginArray");
            EXPECT_EQ(count, 3u);
        }
        void EndArray() override { Events.emplace_back("EndArray"); }
        void ExchangeHeader(const ZToken, const EPropertyType type) override { HeaderTypes.push_back(type); }
        void ExchangeFooter(const EPropertyType) override {}
        void ExchangeData(ZSerializable*) override {}
        void ExchangeData(ZBitfieldBase*, const ZEnumInfo*) override {}
        void ExchangeData(void*, const ZEnumInfo*) override {}
        void ExchangeData(zstring&) override {}
        void ExchangeData(const char*&) override {}
        void ExchangeData(double&) override {}
        void ExchangeData(float& value) override
        {
            FloatValues.push_back(value);
            if (!Saving)
                value += 10.0f;
        }
        void ExchangeData(uint8_t&) override { Events.emplace_back("U8"); }
        void ExchangeData(int8_t&) override {}
        void ExchangeData(uint16_t&) override {}
        void ExchangeData(int16_t& value) override
        {
            Int16Values.push_back(value);
            if (!Saving)
                value = static_cast<int16_t>(value + 1);
        }
        void ExchangeData(uint32_t&) override {}
        void ExchangeData(int32_t&) override {}
        void ExchangeData(bool&) override { Events.emplace_back("Bool"); }
    };
}

TEST(ZLocation, LoadSaveUsesOriginalFieldTypesAndOrder)
{
    PF4::ZLocation location({ 1.0f, 2.0f, 3.0f }, 7, 5, true);
    RecordingStream stream;

    location.LoadSave(stream, true);

    EXPECT_EQ(stream.Tokens, (std::vector<std::string>{ "m_Component", "m_Graph", "m_Inside", "m_vPos" }));
    EXPECT_EQ(stream.HeaderTypes, (std::vector<ISerializerStream::EPropertyType>{
        ISerializerStream::PT_Int16,
        ISerializerStream::PT_Int16,
        ISerializerStream::PT_Int16,
        ISerializerStream::PT_Float32,
        ISerializerStream::PT_Float32,
        ISerializerStream::PT_Float32,
    }));
    EXPECT_EQ(stream.Int16Values, (std::vector<int16_t>{ 5, 7, 1 }));
    EXPECT_EQ(stream.FloatValues, (std::vector<float>{ 1.0f, 2.0f, 3.0f }));
}

TEST(ZLocation, LoadSaveWritesBackLoadedValues)
{
    PF4::ZLocation location({ 1.0f, 2.0f, 3.0f }, 7, 5, true);
    RecordingStream stream(false);

    location.LoadSave(stream, false);

    EXPECT_EQ(location.Component(), 6);
    EXPECT_EQ(location.Graph(), 8);
    EXPECT_EQ(location.Inside(), 2u);
}
