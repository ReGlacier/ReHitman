#include <Glacier/System/ZIniFileParserSimple.h>

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

namespace Glacier
{
    TEST(ZIniFileParserSimple, ProcessCmdLineReturnsMainIniForEmptyCommandLine)
    {
        ZIniFileParserSimple parser;

        const MYSTR result = parser.ProcessCmdLine("");

        EXPECT_STREQ(static_cast<const char*>(result), "main.ini");
    }

    TEST(ZIniFileParserSimple, ProcessCmdLineStripsAtPrefix)
    {
        ZIniFileParserSimple parser;

        const MYSTR result = parser.ProcessCmdLine("@profiles/player.ini");

        EXPECT_STREQ(static_cast<const char*>(result), "profiles/player.ini");
    }

    TEST(ZIniFileParserSimple, ProcessCmdLineKeepsPlainIniNameAndTrimsTrailingWhitespace)
    {
        ZIniFileParserSimple parser;

        const MYSTR result = parser.ProcessCmdLine("custom.ini \t\r\n");

        EXPECT_STREQ(static_cast<const char*>(result), "custom.ini");
    }

    TEST(ZIniFileParserSimple, ParseOptionLineSplitsNameAndArgsAtEquals)
    {
        char name[512]{};
        char args[512]{};

        ZIniFileParserSimpleTests::ParseOptionLine("  VideoMode = 1024 768 32   ", name, args);

        EXPECT_STREQ(name, "VideoMode");
        EXPECT_STREQ(args, "1024 768 32");
    }

    TEST(ZIniFileParserSimple, ParseOptionLineUsesFirstWhitespaceWhenEqualsIsAbsent)
    {
        char name[512]{};
        char args[512]{};

        ZIniFileParserSimpleTests::ParseOptionLine("ConsoleCmd stat fps   ", name, args);

        EXPECT_STREQ(name, "ConsoleCmd");
        EXPECT_STREQ(args, "stat fps");
    }

    TEST(ZIniFileParserSimple, CopyNameReturnsPointerToArguments)
    {
        char name[512]{};
        const char* source = "\tOptionName value";

        const char* args = ZIniFileParserSimpleTests::CopyName(name, source);

        EXPECT_STREQ(name, "OptionName");
        EXPECT_EQ(*args, ' ');
    }

    TEST(ZIniFileParserSimple, RemoveTrailingSpacesMatchesOriginalWhitespaceSet)
    {
        char text[] = "value \t\r\n";

        ZIniFileParserSimpleTests::RemoveTrailingSpaces(text);

        EXPECT_STREQ(text, "value");
        EXPECT_TRUE(ZIniFileParserSimpleTests::IsSpace(' '));
        EXPECT_TRUE(ZIniFileParserSimpleTests::IsSpace('\t'));
        EXPECT_TRUE(ZIniFileParserSimpleTests::IsSpace('\r'));
        EXPECT_TRUE(ZIniFileParserSimpleTests::IsSpace('\n'));
        EXPECT_FALSE(ZIniFileParserSimpleTests::IsSpace('\v'));
    }

    TEST(ZIniFileParserSimple, RemoveTrailingSpacesLeavesSingleSpaceLikeOriginalLoop)
    {
        char text[] = " ";

        ZIniFileParserSimpleTests::RemoveTrailingSpaces(text);

        EXPECT_STREQ(text, " ");
    }

    TEST(ZIniFileParserSimple, ParseOptionLineAssertsOnTooLongLine)
    {
        char name[512]{};
        char args[512]{};
        const std::string tooLong(0x1FF, 'a');

        EXPECT_THROW(
            ZIniFileParserSimpleTests::ParseOptionLine(tooLong.c_str(), name, args),
            std::runtime_error);
    }
}
