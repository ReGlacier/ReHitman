#include <Glacier/Serializer/ZPackedInput.h>
#include <gtest/gtest.h>

using namespace Glacier;

namespace
{
    struct PackedDictionaryFixture
    {
        ZPackedDictionary_Serializerlib Dictionary;

        PackedDictionaryFixture()
        {
            Dictionary.m_Size = 4;
            Dictionary.Setup();

            Dictionary.m_Letters[0] = 0;
            Dictionary.m_Letters[1] = 'a';
            Dictionary.m_Letters[2] = 'b';
            Dictionary.m_Letters[3] = 'c';

            Dictionary.m_From[0] = 1;
            Dictionary.m_From[1] = 2;
            Dictionary.m_From[2] = 3;
            Dictionary.m_From[3] = 3;

            Dictionary.m_To[0] = 2;
            Dictionary.m_To[1] = 3;
            Dictionary.m_To[2] = 3;
            Dictionary.m_To[3] = 3;

            Dictionary.m_Tokens[0] = ZToken::Void;
            Dictionary.m_Tokens[1] = ZToken::Void;
            Dictionary.m_Tokens[2] = ZToken(42);
            Dictionary.m_Tokens[3] = ZToken::Void;
        }
    };
}

TEST(ZPackedDictionary, EmptyDictionaryReturnsVoid)
{
    ZPackedDictionary_Serializerlib dictionary;
    dictionary.m_Size = 0;

    EXPECT_EQ(dictionary.GetToken("anything"), ZToken::Void);
}

TEST(ZPackedDictionary, FindsTokenByPackedTriePath)
{
    PackedDictionaryFixture fixture;

    EXPECT_EQ(fixture.Dictionary.GetToken("ab"), ZToken(42));
}

TEST(ZPackedDictionary, ReturnsUnknownWhenPathIsMissing)
{
    PackedDictionaryFixture fixture;

    EXPECT_EQ(fixture.Dictionary.GetToken("ac"), ZToken::Unknown);
    EXPECT_EQ(fixture.Dictionary.GetToken("b"), ZToken::Unknown);
}

TEST(ZPackedDictionary, UsesSevenBitDictionaryIndex)
{
    PackedDictionaryFixture fixture;
    fixture.Dictionary.m_Letters[1] = static_cast<char>(0xE1); // 0xE1 & 0x7F == 'a'

    EXPECT_EQ(fixture.Dictionary.GetToken("\xE1" "b"), ZToken(42));
}
