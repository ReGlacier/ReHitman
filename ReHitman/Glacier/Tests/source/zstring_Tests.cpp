#include <Glacier/ZSTL/zstring.h>
#include <gtest/gtest.h>

#include <stdexcept>

using namespace Glacier;

namespace
{
    static_assert(sizeof(zstring) == 0xC);
}

TEST(zstring, DefaultConstructorCreatesEmptyString)
{
    zstring str;

    EXPECT_TRUE(str.empty());
    EXPECT_EQ(str.length(), 0u);
    EXPECT_STREQ(str.c_str(), "");
}

TEST(zstring, ConstructsFromCString)
{
    zstring str("hello");

    EXPECT_EQ(str.length(), 5u);
    EXPECT_STREQ(str.c_str(), "hello");
}

TEST(zstring, ConstructsFromCStringAndLength)
{
    zstring str("hello world", 5);

    EXPECT_EQ(str.length(), 5u);
    EXPECT_STREQ(str.c_str(), "hello");
}

TEST(zstring, CopyAndSubstringConstructors)
{
    zstring original("abcdef");
    zstring copy(original);
    zstring substring(original, 2, 3);

    EXPECT_STREQ(copy.c_str(), "abcdef");
    EXPECT_STREQ(substring.c_str(), "cde");
}

TEST(zstring, AssignmentHandlesSelfAndOther)
{
    zstring source("source");
    zstring target("target");

    target = source;
    target = target;

    EXPECT_STREQ(target.c_str(), "source");
}

TEST(zstring, ToUpperConvertsAsciiOnly)
{
    zstring str("aBc-xyz-123");

    str.to_upper();

    EXPECT_STREQ(str.c_str(), "ABC-XYZ-123");
}

TEST(zstring, EqualityAndCompareUseStringContents)
{
    zstring first("abcd");
    zstring second("abce");
    zstring same("abcd");

    EXPECT_TRUE(first == same);
    EXPECT_TRUE(first == "abcd");
    EXPECT_FALSE(first == second);
    EXPECT_TRUE(first.compare(second, 3));
    EXPECT_FALSE(first.compare(second));
    EXPECT_TRUE(first.equal(same));
    EXPECT_TRUE(first.equal(second, 3));
}

TEST(zstring, CountReturnsNumberOfMatchingCharacters)
{
    zstring str("banana");

    EXPECT_EQ(str.count('a'), 3u);
    EXPECT_EQ(str.count('z'), 0u);
}

TEST(zstring, FormatWritesFormattedContent)
{
    zstring str;

    str.format("%s-%d", "value", 42);

    EXPECT_STREQ(str.c_str(), "value-42");
}

TEST(zstring, FormatAssertsWhenOutputDoesNotFit)
{
    zstring str;
    char large[1100]{};
    for (char& ch : large)
        ch = 'x';
    large[sizeof(large) - 1] = '\0';

    EXPECT_THROW(str.format("%s", large), std::runtime_error);
}
