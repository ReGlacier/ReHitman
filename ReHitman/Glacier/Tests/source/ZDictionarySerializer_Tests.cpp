#include <Glacier/Serializer/ZOutputStreamBase.h>
#include <Glacier/Serializer/ZDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZTokenTable_Serializerlib.h>
#include <Glacier/Serializer/ZTokenTable.h>
#include <Glacier/Serializer/ZDictionary.h>

#include <gtest/gtest.h>

#include <cstring>
#include <vector>

using namespace Glacier;

namespace
{
    struct MemoryOutputStream final : public ZOutputStreamBase
    {
        std::vector<char> Bytes;

        MemoryOutputStream()
        {
            m_ChangeEndianness = false;
        }

        uint32_t WriteRaw(char* address, const uint32_t size) override
        {
            Bytes.insert(Bytes.end(), address, address + size);
            return size;
        }

        uint32_t WriteChangeEndianness(char* address, const uint32_t size, const uint32_t) override
        {
            return WriteRaw(address, size);
        }

        template <typename T>
        T ReadAt(size_t offset) const
        {
            T value{};
            std::memcpy(&value, Bytes.data() + offset, sizeof(T));
            return value;
        }
    };
}

TEST(ZDictionarySerializerlib, SaveAsTokenTableWritesLargestTokenSizeAndWords)
{
    ZDictionary_Serializerlib dictionary;
    ZToken token;

    token = dictionary.GetToken("beta");
    token = dictionary.GetToken("alpha");

    ASSERT_EQ(dictionary.m_StringTable.GetSize(), 2u);
    ASSERT_STREQ(dictionary.m_StringTable[0].c_str(), "beta");
    ASSERT_STREQ(dictionary.m_StringTable[1].c_str(), "alpha");

    MemoryOutputStream stream;
    ZTokenTable_Serializerlib tokenTable(dictionary);
    tokenTable.Save(stream);

    ASSERT_GE(stream.Bytes.size(), sizeof(uint32_t) * 2);
    EXPECT_EQ(stream.ReadAt<uint32_t>(0), 1u);
    EXPECT_EQ(stream.ReadAt<uint32_t>(4), 11u);
    ASSERT_EQ(stream.Bytes.size(), 19u);
    EXPECT_STREQ(stream.Bytes.data() + 8, "beta");
    EXPECT_STREQ(stream.Bytes.data() + 13, "alpha");
}
