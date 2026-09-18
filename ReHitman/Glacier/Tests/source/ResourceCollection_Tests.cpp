#include <Glacier/ResourceCollection.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

using namespace Glacier;

namespace
{
    using Bytes = std::vector<uint8_t>;

    struct Entry
    {
        std::string Name;
        uint8_t Flags = 0;
        Bytes Payload;
    };

    void Append(Bytes& bytes, const Bytes& source)
    {
        bytes.insert(bytes.end(), source.begin(), source.end());
    }

    void AppendString(Bytes& bytes, const char* value)
    {
        while (*value != '\0')
            bytes.push_back(static_cast<uint8_t>(*value++));

        bytes.push_back(0);
    }

    void AppendU32(Bytes& bytes, const uint32_t value)
    {
        bytes.push_back(static_cast<uint8_t>(value & 0xFF));
        bytes.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        bytes.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        bytes.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    }

    void WriteOffset(Bytes& bytes, const size_t index, const uint32_t offset)
    {
        const auto offsetIndex = 4 * index - 3;
        bytes[offsetIndex + 0] = static_cast<uint8_t>(offset & 0xFF);
        bytes[offsetIndex + 1] = static_cast<uint8_t>((offset >> 8) & 0xFF);
        bytes[offsetIndex + 2] = static_cast<uint8_t>((offset >> 16) & 0xFF);
        bytes[offsetIndex + 3] = static_cast<uint8_t>((offset >> 24) & 0xFF);
    }

    Bytes TextPayload(const char* text)
    {
        Bytes bytes;
        AppendString(bytes, text);
        return bytes;
    }

    Bytes TextAndSoundPayload(const char* text, const uint32_t soundResourceId)
    {
        auto bytes = TextPayload(text);
        AppendU32(bytes, soundResourceId);
        return bytes;
    }

    Bytes Table(const std::vector<Entry>& entries)
    {
        Bytes bytes;
        bytes.resize(1 + entries.size() > 0 ? 4 * entries.size() - 3 : 1, 0);
        bytes[0] = static_cast<uint8_t>(entries.size());

        uint32_t offset = 0;

        for (size_t i = 0; i < entries.size(); ++i)
        {
            if (i != 0)
                WriteOffset(bytes, i, offset);

            AppendString(bytes, entries[i].Name.c_str());
            bytes.push_back(entries[i].Flags);
            Append(bytes, entries[i].Payload);
            offset = static_cast<uint32_t>(bytes.size() - (4 * entries.size() - 3));
        }

        return bytes;
    }

    Bytes LocaleBuffer()
    {
        auto folder = Table({
            { "item", 0x01, TextPayload("Nested") },
        });

        auto choices = Table({
            { "first", 0x01, TextPayload("First") },
            { "second", 0x01, TextPayload("Second") },
        });

        return Table({
            { "choices", 0x10, choices },
            { "folder", 0x00, folder },
            { "hello", 0x21, TextAndSoundPayload("Hello", 0x12345678) },
        });
    }
}

TEST(Resource, SetKeyNormalizesSlashesAndBuildsFilename)
{
    Resource resource;

    resource.SetKey("///folder//item");

    EXPECT_STREQ(resource.GetKey(), "folder/item");
    EXPECT_STREQ(resource.m_sFilename, "folder/item.wav");
}

TEST(Resource, GetTextReturnsTextOnlyWhenFlagIsPresent)
{
    char textData[] = { 0x01, 'H', 'e', 'l', 'l', 'o', 0 };
    char emptyData[] = { 0x00, 'I', 'g', 'n', 'o', 'r', 'e', 'd', 0 };
    Resource resource;

    resource.SetData(textData);
    EXPECT_STREQ(resource.GetText(), "Hello");

    resource.SetData(emptyData);
    EXPECT_STREQ(resource.GetText(), "");
}

TEST(Resource, GetSoundResourceIdSkipsOptionalStrings)
{
    char data[] = { 0x23, 'T', 'e', 'x', 't', 0, 'M', 'e', 't', 'a', 0, 0x78, 0x56, 0x34, 0x12 };
    Resource resource;

    resource.SetData(data);

    EXPECT_EQ(resource.GetSoundResourceId(), 0x12345678u);
}

TEST(ResourceCollection, LooksUpNestedResourcesByNormalizedCaseInsensitivePath)
{
    auto bytes = LocaleBuffer();
    ResourceCollection collection;
    collection.m_pBuffer = bytes.data();
    Resource resource;

    ASSERT_EQ(collection.GetResource("///FOLDER//ITEM", &resource), &resource);

    EXPECT_STREQ(resource.GetKey(), "FOLDER/ITEM");
    EXPECT_STREQ(resource.GetText(), "Nested");
    EXPECT_TRUE(collection.HasResource("folder/item"));
    EXPECT_FALSE(collection.HasResource("folder/missing"));
}

TEST(ResourceCollection, ResourceTextWithBaseAndLastFallsBackToLast)
{
    auto bytes = LocaleBuffer();
    ResourceCollection collection;
    collection.m_pBuffer = bytes.data();

    EXPECT_STREQ(collection.GetResourceText("folder", "item"), "Nested");
    EXPECT_STREQ(collection.GetResourceText("folder", "missing"), "missing");
}

TEST(ResourceCollection, FillsResourceAndReadsSoundResourceId)
{
    auto bytes = LocaleBuffer();
    ResourceCollection collection;
    collection.m_pBuffer = bytes.data();
    Resource resource;

    ASSERT_EQ(collection.GetResource("hello", &resource), &resource);

    EXPECT_STREQ(resource.GetText(), "Hello");
    EXPECT_EQ(resource.GetSoundResourceId(), 0x12345678u);
}

TEST(ResourceCollection, ResolvesElementResourcesByIndex)
{
    auto bytes = LocaleBuffer();
    ResourceCollection collection;
    collection.m_pBuffer = bytes.data();
    Resource resource;

    ASSERT_EQ(collection.GetElementResource("choices", 1, &resource), &resource);

    EXPECT_EQ(collection.GetNumElements("choices"), 2);
    EXPECT_STREQ(resource.GetKey(), "choices/second");
    EXPECT_STREQ(resource.GetText(), "Second");
    EXPECT_STREQ(collection.GetElementText("choices", 0), "First");
    EXPECT_EQ(collection.GetElementResource("choices", 2, &resource), nullptr);
    EXPECT_EQ(collection.GetNumElements("hello"), -1);
}
