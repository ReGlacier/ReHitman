#include <Glacier/Serializer/IDictionary.h>
#include <Glacier/Serializer/IDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZFastDictionary_Serializerlib.h>
#include <Glacier/Serializer/ZInputStreamBase.h>
#include <Glacier/Serializer/ZOutputStreamBase.h>
#include <Glacier/Serializer/ZPackedDictionary.h>
#include <Glacier/Serializer/ZTokenTable_Serializerlib.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Glacier;

namespace
{
    struct MemoryOutputStream final : public ZOutputStreamBase
    {
        std::vector<char> Bytes;

        explicit MemoryOutputStream(bool bigEndian = false)
        {
            m_ChangeEndianness = bigEndian;
        }

        uint32_t WriteRaw(char* address, const uint32_t size) override
        {
            Bytes.insert(Bytes.end(), address, address + size);
            return size;
        }

        uint32_t WriteChangeEndianness(char* address, const uint32_t size, const uint32_t mask) override
        {
            for (uint32_t offset = 0; offset < size; ++offset)
                Bytes.push_back(address[offset ^ mask]);

            return size;
        }
    };

    struct MemoryInputStream final : public ZInputStreamBase
    {
        const std::vector<char>& Bytes;
        size_t Offset{ 0 };

        explicit MemoryInputStream(const std::vector<char>& bytes, bool bigEndian = false)
            : Bytes(bytes)
        {
            m_ChangeEndianness = bigEndian;
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

    std::vector<std::string> StressWords()
    {
        std::vector<std::string> words;
        words.reserve(256);

        for (int group = 0; group < 16; ++group)
        {
            for (int index = 0; index < 16; ++index)
            {
                words.push_back("node" + std::to_string(group) + "/leaf" + std::to_string(index));
            }
        }

        return words;
    }
}

TEST(ZDictionary, AssignsStableSequentialTokensAndMaintainsSortedTokenTable)
{
    ZDictionary dictionary;

    EXPECT_EQ(dictionary.GetToken("beta"), ZToken(0));
    EXPECT_EQ(dictionary.GetToken("alpha"), ZToken(1));
    EXPECT_EQ(dictionary.GetToken("gamma"), ZToken(2));
    EXPECT_EQ(dictionary.GetToken("beta"), ZToken(0));

    ASSERT_EQ(dictionary.m_StringTable.GetSize(), 3u);
    EXPECT_STREQ(dictionary.m_StringTable[0].c_str(), "beta");
    EXPECT_STREQ(dictionary.m_StringTable[1].c_str(), "alpha");
    EXPECT_STREQ(dictionary.m_StringTable[2].c_str(), "gamma");

    ASSERT_EQ(dictionary.m_TokenTable.GetSize(), 3u);
    EXPECT_STREQ(dictionary.m_TokenTable[0].m_String.c_str(), "alpha");
    EXPECT_EQ(dictionary.m_TokenTable[0].m_Token, ZToken(1));
    EXPECT_STREQ(dictionary.m_TokenTable[1].m_String.c_str(), "beta");
    EXPECT_EQ(dictionary.m_TokenTable[1].m_Token, ZToken(0));
    EXPECT_STREQ(dictionary.m_TokenTable[2].m_String.c_str(), "gamma");
    EXPECT_EQ(dictionary.m_TokenTable[2].m_Token, ZToken(2));
}

TEST(ZDictionary, NullWordUsesEmptyStringToken)
{
    ZDictionary dictionary;

    EXPECT_EQ(dictionary.GetToken(nullptr), ZToken(0));
    EXPECT_EQ(dictionary.GetToken(""), ZToken(0));
    ASSERT_EQ(dictionary.m_StringTable.GetSize(), 1u);
    EXPECT_STREQ(dictionary.m_StringTable[0].c_str(), "");
}

TEST(ZFastDictionary, HandlesSharedPrefixesBranchesAndSevenBitCharacters)
{
    ZFastDictionary dictionary;

    EXPECT_EQ(dictionary.GetToken("a"), ZToken(0));
    EXPECT_EQ(dictionary.GetToken("ab"), ZToken(1));
    EXPECT_EQ(dictionary.GetToken("abc"), ZToken(2));
    EXPECT_EQ(dictionary.GetToken("ac"), ZToken(3));
    EXPECT_EQ(dictionary.GetToken("b"), ZToken(4));
    EXPECT_EQ(dictionary.GetToken("ab"), ZToken(1));
    EXPECT_EQ(dictionary.GetToken("\xE1"), ZToken(0));

    EXPECT_EQ(dictionary.GetLargestToken(), ZToken(4));
    EXPECT_GT(dictionary.CountNodes(), 1u);
    EXPECT_GE(dictionary.CountWordsLength(true), dictionary.CountWordsLength(false));
}

TEST(ZFastDictionary, TokenTableBuildMapsTokensBackToWords)
{
    ZFastDictionary dictionary;
    const ZToken alpha = dictionary.GetToken("alpha");
    const ZToken alps = dictionary.GetToken("alps");
    const ZToken beta = dictionary.GetToken("beta");

    ZTokenTable table(dictionary);

    EXPECT_TRUE(table.IsValidToken(alpha));
    EXPECT_TRUE(table.IsValidToken(alps));
    EXPECT_TRUE(table.IsValidToken(beta));
    EXPECT_STREQ(table.GetWord(alpha), "alpha");
    EXPECT_STREQ(table.GetWord(alps), "alps");
    EXPECT_STREQ(table.GetWord(beta), "beta");
    EXPECT_FALSE(table.IsValidToken(ZToken::Void));
    EXPECT_FALSE(table.IsValidToken(ZToken(99)));
}

TEST(ZTokenTable, BuildsFromDictionaryAndRejectsInvalidTokens)
{
    ZDictionary dictionary;
    const ZToken beta = dictionary.GetToken("beta");
    const ZToken alpha = dictionary.GetToken("alpha");

    ZTokenTable table(dictionary);

    EXPECT_EQ(table.m_LargestToken, ZToken(1));
    EXPECT_STREQ(table.GetWord(beta), "beta");
    EXPECT_STREQ(table.GetWord(alpha), "alpha");
    EXPECT_FALSE(table.IsValidToken(ZToken::Void));
    EXPECT_FALSE(table.IsValidToken(ZToken(2)));
    EXPECT_THROW(table.GetWord(ZToken(2)), std::runtime_error);
}

TEST(ZTokenTableSerializerlib, SaveLoadRoundTripsDictionaryTokenTable)
{
    ZDictionary dictionary;
    dictionary.GetToken("beta");
    dictionary.GetToken("alpha");

    ZTokenTable_Serializerlib saved(dictionary);
    MemoryOutputStream output;
    saved.Save(output);

    MemoryInputStream input(output.Bytes);
    ZTokenTable_Serializerlib loaded;
    loaded.Load(input);

    EXPECT_EQ(loaded.m_LargestToken, ZToken(1));
    EXPECT_STREQ(loaded.GetWord(ZToken(0)), "beta");
    EXPECT_STREQ(loaded.GetWord(ZToken(1)), "alpha");
    EXPECT_EQ(input.Offset, output.Bytes.size());
}

TEST(ZTokenTableSerializerlib, SaveLoadRoundTripsFastDictionaryTokenTable)
{
    ZFastDictionary dictionary;
    dictionary.GetToken("alpha");
    dictionary.GetToken("alps");
    dictionary.GetToken("beta");

    ZTokenTable_Serializerlib saved(dictionary);
    MemoryOutputStream output;
    saved.Save(output);

    MemoryInputStream input(output.Bytes);
    ZTokenTable_Serializerlib loaded;
    loaded.Load(input);

    EXPECT_STREQ(loaded.GetWord(ZToken(0)), "alpha");
    EXPECT_STREQ(loaded.GetWord(ZToken(1)), "alps");
    EXPECT_STREQ(loaded.GetWord(ZToken(2)), "beta");
    EXPECT_EQ(input.Offset, output.Bytes.size());
}

TEST(ZTokenTableSerializerlib, SaveLoadHonorsBigEndianStreams)
{
    ZDictionary dictionary;
    dictionary.GetToken("one");
    dictionary.GetToken("two");

    ZTokenTable_Serializerlib saved(dictionary);
    MemoryOutputStream output(true);
    saved.Save(output);

    MemoryInputStream input(output.Bytes, true);
    ZTokenTable_Serializerlib loaded;
    loaded.Load(input);

    EXPECT_STREQ(loaded.GetWord(ZToken(0)), "one");
    EXPECT_STREQ(loaded.GetWord(ZToken(1)), "two");
    EXPECT_EQ(input.Offset, output.Bytes.size());
}

TEST(ZPackedDictionary, BuildsFromDictionaryAndFastDictionary)
{
    ZFastDictionary fastDictionary;
    fastDictionary.GetToken("beta");
    fastDictionary.GetToken("alpha");
    fastDictionary.GetToken("alphabet");

    ZPackedDictionary fromFastDictionary(fastDictionary);
    EXPECT_EQ(fromFastDictionary.GetToken("beta"), ZToken(0));
    EXPECT_EQ(fromFastDictionary.GetToken("alpha"), ZToken(1));
    EXPECT_EQ(fromFastDictionary.GetToken("alphabet"), ZToken(2));
    EXPECT_EQ(fromFastDictionary.GetToken("alp"), ZToken::Void);
}

TEST(ZPackedDictionary, BuildsFromSortedDictionary)
{
    ZDictionary dictionary;
    dictionary.GetToken("alpha");
    dictionary.GetToken("alphabet");
    dictionary.GetToken("beta");

    ZPackedDictionary packed(dictionary);

    EXPECT_EQ(packed.GetToken("alpha"), ZToken(0));
    EXPECT_EQ(packed.GetToken("alphabet"), ZToken(1));
    EXPECT_EQ(packed.GetToken("beta"), ZToken(2));
    EXPECT_EQ(packed.GetToken("alp"), ZToken::Unknown);
}

TEST(ZPackedDictionarySerializerlib, SaveLoadRoundTripsPackedDictionary)
{
    ZFastDictionary dictionary;
    dictionary.GetToken("alpha");
    dictionary.GetToken("alps");
    dictionary.GetToken("beta");

    ZPackedDictionary_Serializerlib saved(dictionary);
    MemoryOutputStream output;
    saved.Save(output);

    MemoryInputStream input(output.Bytes);
    ZPackedDictionary_Serializerlib loaded;
    loaded.Load(input);

    EXPECT_EQ(loaded.GetToken("alpha"), ZToken(0));
    EXPECT_EQ(loaded.GetToken("alps"), ZToken(1));
    EXPECT_EQ(loaded.GetToken("beta"), ZToken(2));
    EXPECT_EQ(loaded.GetToken("alp"), ZToken::Void);
    EXPECT_EQ(input.Offset, output.Bytes.size());
}

TEST(ZPackedDictionarySerializerlib, SaveLoadHonorsBigEndianStreams)
{
    ZFastDictionary dictionary;
    dictionary.GetToken("alpha");
    dictionary.GetToken("beta");

    ZPackedDictionary_Serializerlib saved(dictionary);
    MemoryOutputStream output(true);
    saved.Save(output);

    MemoryInputStream input(output.Bytes, true);
    ZPackedDictionary_Serializerlib loaded;
    loaded.Load(input);

    EXPECT_EQ(loaded.GetToken("alpha"), ZToken(0));
    EXPECT_EQ(loaded.GetToken("beta"), ZToken(1));
    EXPECT_EQ(input.Offset, output.Bytes.size());
}

TEST(ZDictionarySerializerlib, VirtualSerializerInterfaceSavesTokenTableAndPackedDictionary)
{
    ZDictionary_Serializerlib dictionary;
    IDictionary* lookup = &dictionary;
    IDictionary_Serializerlib* serializer = &dictionary;

    EXPECT_EQ(lookup->GetToken("alpha"), ZToken(0));
    EXPECT_EQ(lookup->GetToken("beta"), ZToken(1));

    MemoryOutputStream tokenTableOutput;
    serializer->SaveAsTokenTable(tokenTableOutput);
    MemoryInputStream tokenTableInput(tokenTableOutput.Bytes);
    ZTokenTable_Serializerlib tokenTable;
    tokenTable.Load(tokenTableInput);

    EXPECT_STREQ(tokenTable.GetWord(ZToken(0)), "alpha");
    EXPECT_STREQ(tokenTable.GetWord(ZToken(1)), "beta");

    MemoryOutputStream packedOutput;
    serializer->SaveAsPackedDictionary(packedOutput);
    MemoryInputStream packedInput(packedOutput.Bytes);
    ZPackedDictionary_Serializerlib packed;
    packed.Load(packedInput);

    EXPECT_EQ(packed.GetToken("alpha"), ZToken(0));
    EXPECT_EQ(packed.GetToken("beta"), ZToken(1));
}

TEST(ZFastDictionarySerializerlib, VirtualSerializerInterfaceSavesTokenTableAndPackedDictionary)
{
    ZFastDictionary_Serializerlib dictionary;
    IDictionary* lookup = &dictionary;
    IDictionary_Serializerlib* serializer = &dictionary;

    EXPECT_EQ(lookup->GetToken("alpha"), ZToken(0));
    EXPECT_EQ(lookup->GetToken("alps"), ZToken(1));
    EXPECT_EQ(lookup->GetToken("beta"), ZToken(2));

    MemoryOutputStream tokenTableOutput;
    serializer->SaveAsTokenTable(tokenTableOutput);
    MemoryInputStream tokenTableInput(tokenTableOutput.Bytes);
    ZTokenTable_Serializerlib tokenTable;
    tokenTable.Load(tokenTableInput);

    EXPECT_STREQ(tokenTable.GetWord(ZToken(0)), "alpha");
    EXPECT_STREQ(tokenTable.GetWord(ZToken(1)), "alps");
    EXPECT_STREQ(tokenTable.GetWord(ZToken(2)), "beta");

    MemoryOutputStream packedOutput;
    serializer->SaveAsPackedDictionary(packedOutput);
    MemoryInputStream packedInput(packedOutput.Bytes);
    ZPackedDictionary_Serializerlib packed;
    packed.Load(packedInput);

    EXPECT_EQ(packed.GetToken("alpha"), ZToken(0));
    EXPECT_EQ(packed.GetToken("alps"), ZToken(1));
    EXPECT_EQ(packed.GetToken("beta"), ZToken(2));
}

TEST(ZFastDictionary, StressPackedDictionaryAndTokenTableWithManyPrefixes)
{
    ZFastDictionary dictionary;
    const std::vector<std::string> words = StressWords();

    for (size_t i = 0; i < words.size(); ++i)
        EXPECT_EQ(dictionary.GetToken(words[i].c_str()), ZToken(static_cast<int32_t>(i)));

    ZPackedDictionary packed(dictionary);
    ZTokenTable table(dictionary);

    for (size_t i = 0; i < words.size(); ++i)
    {
        const ZToken token(static_cast<int32_t>(i));
        EXPECT_EQ(packed.GetToken(words[i].c_str()), token);
        EXPECT_STREQ(table.GetWord(token), words[i].c_str());
    }
}
