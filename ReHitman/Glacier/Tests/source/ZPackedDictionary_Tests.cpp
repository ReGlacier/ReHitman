#include <Glacier/Serializer/ZPackedInput.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    struct PackedDictionaryFixture
    {
        char Letters[4]{ 0, 'a', 'b', 'c' };
        uint32_t From[4]{ 1, 2, 3, 3 };
        uint32_t To[4]{ 2, 3, 3, 3 };
        ZToken Tokens[4]{ ZToken::Void, ZToken::Void, ZToken(42), ZToken::Void };
        ZPackedDictionary_Serializerlib Dictionary;

        PackedDictionaryFixture()
        {
            Dictionary.m_Size = 4;
            Dictionary.m_Letters = Letters;
            Dictionary.m_From = From;
            Dictionary.m_To = To;
            Dictionary.m_Tokens = Tokens;
        }
    };
}

TEST(ZPackedDictionary, EmptyDictionaryReturnsVoid)
{
    ZPackedDictionary_Serializerlib dictionary;
    dictionary.m_Size = 0;

    ZToken token;
    EXPECT_EQ(*dictionary.GetToken(&token, "anything"), ZToken::Void);
}

TEST(ZPackedDictionary, FindsTokenByPackedTriePath)
{
    PackedDictionaryFixture fixture;

    ZToken token;
    EXPECT_EQ(*fixture.Dictionary.GetToken(&token, "ab"), ZToken(42));
}

TEST(ZPackedDictionary, ReturnsUnknownWhenPathIsMissing)
{
    PackedDictionaryFixture fixture;

    ZToken token;
    EXPECT_EQ(*fixture.Dictionary.GetToken(&token, "ac"), ZToken::Unknown);
    EXPECT_EQ(*fixture.Dictionary.GetToken(&token, "b"), ZToken::Unknown);
}

TEST(ZPackedDictionary, UsesSevenBitDictionaryIndex)
{
    PackedDictionaryFixture fixture;
    fixture.Letters[1] = static_cast<char>(0xE1); // 0xE1 & 0x7F == 'a'

    ZToken token;
    EXPECT_EQ(*fixture.Dictionary.GetToken(&token, "\xE1" "b"), ZToken(42));
}
