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

TEST(zstring, OperatorPlusZstringAndCStringConcatenates)
{
    zstring lhs("foo");

    zstring result = lhs + "bar";

    EXPECT_STREQ(result.c_str(), "foobar");
    EXPECT_EQ(result.length(), 6u);
    EXPECT_EQ(result.m_iCapacity, 6u);
}

TEST(zstring, OperatorPlusZstringAndCStringHandlesEmptyParts)
{
    zstring empty;
    zstring str("foo");

    EXPECT_STREQ((empty + "bar").c_str(), "bar");
    EXPECT_STREQ((str + "").c_str(), "foo");
    EXPECT_STREQ((str + static_cast<const char*>(nullptr)).c_str(), "foo");
}

TEST(zstring, OperatorPlusZstringAndCStringChains)
{
    zstring result = zstring("a") + "b" + "c";

    EXPECT_STREQ(result.c_str(), "abc");
    EXPECT_EQ(result.length(), 3u);
}

TEST(zstring, OperatorPlusZstringAndCStringDoesNotModifySource)
{
    zstring lhs("foo");

    zstring result = lhs + "bar";
    result.to_upper();

    EXPECT_STREQ(lhs.c_str(), "foo");
    EXPECT_STREQ(result.c_str(), "FOOBAR");
}

TEST(zstring, OperatorPlusCStringAndZstringConcatenates)
{
    zstring rhs("bar");

    zstring result = "foo" + rhs;

    EXPECT_STREQ(result.c_str(), "foobar");
    EXPECT_EQ(result.length(), 6u);
    EXPECT_EQ(result.m_iCapacity, 6u);
}

TEST(zstring, OperatorPlusCStringAndZstringHandlesEmptyParts)
{
    zstring rhs("bar");
    zstring empty;

    EXPECT_STREQ(("" + rhs).c_str(), "bar");
    EXPECT_STREQ((static_cast<const char*>(nullptr) + rhs).c_str(), "bar");
    EXPECT_STREQ(("foo" + empty).c_str(), "foo");
}

TEST(zstring, OperatorPlusCStringAndZstringDoesNotModifySource)
{
    zstring rhs("bar");

    zstring result = "foo" + rhs;
    result.to_upper();

    EXPECT_STREQ(rhs.c_str(), "bar");
    EXPECT_STREQ(result.c_str(), "FOOBAR");
}
