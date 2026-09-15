#include <Glacier/ZSTL/MYSTR.h>
#include <gtest/gtest.h>

using namespace Glacier;


bool IsUsingInternalBuffer(const MYSTR& str)
{
    const char* bufferStart = str.m_Buffer;
    const char* bufferEnd = str.m_Buffer + sizeof(str.m_Buffer);
    return (str.String >= bufferStart && str.String < bufferEnd);
}

TEST(MYSTR_Tests, ConstructorsAndSSOBounds)
{
    MYSTR emptyStr;
    EXPECT_EQ(emptyStr.Length(), 0);
    EXPECT_STREQ(emptyStr.String, "");
    EXPECT_TRUE(IsUsingInternalBuffer(emptyStr));

    std::string shortData(123, 'a');
    MYSTR shortStr(shortData.c_str());
    EXPECT_EQ(shortStr.Length(), 123);
    EXPECT_TRUE(IsUsingInternalBuffer(shortStr));

    std::string longData(124, 'b');
    MYSTR longStr(longData.c_str());
    EXPECT_EQ(longStr.Length(), 124);
    EXPECT_FALSE(IsUsingInternalBuffer(longStr));
    EXPECT_STREQ(longStr.String, longData.c_str());

    MYSTR shortCopy(shortStr);
    EXPECT_TRUE(IsUsingInternalBuffer(shortCopy));
    EXPECT_STREQ(shortCopy.String, shortData.c_str());

    MYSTR longCopy(longStr);
    EXPECT_FALSE(IsUsingInternalBuffer(longCopy));
    EXPECT_STREQ(longCopy.String, longData.c_str());
}

TEST(MYSTR_Tests, DestructorBehavior)
{
    {
        std::string hugeData(1000, 'x');
        MYSTR hugeStr(hugeData.c_str());
        EXPECT_FALSE(IsUsingInternalBuffer(hugeStr));
    }
    SUCCEED();
}

TEST(MYSTR_Tests, SetStringBufferSwitching)
{
    MYSTR str;
    EXPECT_TRUE(IsUsingInternalBuffer(str));

    std::string longData(300, 'c');
    str.SetString(longData.c_str());
    EXPECT_FALSE(IsUsingInternalBuffer(str));
    EXPECT_EQ(str.Length(), 300);

    str.SetString("short");
    EXPECT_TRUE(IsUsingInternalBuffer(str));
    EXPECT_STREQ(str.String, "short");

    str.SetString(longData.c_str());
    EXPECT_FALSE(IsUsingInternalBuffer(str));

    str.SetString(nullptr);
    EXPECT_TRUE(IsUsingInternalBuffer(str));
    EXPECT_STREQ(str.String, "");
}

TEST(MYSTR_Tests, AssignmentOperators)
{
    MYSTR s1("Hello");
    MYSTR s2;

    s2 = s1;
    EXPECT_STREQ(s2.String, "Hello");
    EXPECT_TRUE(IsUsingInternalBuffer(s2));

    s2 = s2;
    EXPECT_STREQ(s2.String, "Hello");

    std::string longData(200, 'z');
    MYSTR sLong(longData.c_str());
    s1 = sLong;
    EXPECT_FALSE(IsUsingInternalBuffer(s1));
    EXPECT_STREQ(s1.String, longData.c_str());

    s1 = s2;
    EXPECT_TRUE(IsUsingInternalBuffer(s1));
    EXPECT_STREQ(s1.String, "Hello");
}

TEST(MYSTR_Tests, IntegerAssignment)
{
    MYSTR str;

    str = 123456;
    EXPECT_STREQ(str.String, "123456");
    EXPECT_TRUE(IsUsingInternalBuffer(str));

    str = -98765;
    EXPECT_STREQ(str.String, "-98765");
    EXPECT_TRUE(IsUsingInternalBuffer(str));

    str = 0;
    EXPECT_STREQ(str.String, "0");

    str = 2147483647;
    EXPECT_STREQ(str.String, "2147483647");
    EXPECT_TRUE(IsUsingInternalBuffer(str));
}

TEST(MYSTR_Tests, Concatenation)
{
    MYSTR a("Hit");
    MYSTR b("man");
    a += b;
    EXPECT_STREQ(a.String, "Hitman");
    EXPECT_TRUE(IsUsingInternalBuffer(a));

    MYSTR base(std::string(60, 'a').c_str());
    MYSTR tail(std::string(60, 'b').c_str());
    base += tail;
    EXPECT_EQ(base.Length(), 120);
    EXPECT_TRUE(IsUsingInternalBuffer(base));

    base += MYSTR("break");
    EXPECT_EQ(base.Length(), 125);
    EXPECT_FALSE(IsUsingInternalBuffer(base));

    MYSTR lhs("Blood ");
    MYSTR rhs("Money");
    MYSTR res = lhs + rhs;
    EXPECT_STREQ(res.String, "Blood Money");
    EXPECT_TRUE(IsUsingInternalBuffer(res));
    
    EXPECT_STREQ(lhs.String, "Blood ");
    EXPECT_STREQ(rhs.String, "Money");
}

TEST(MYSTR_Tests, ToLowerMethod)
{
    MYSTR strShort("HiTMaN 47!");
    strShort.ToLower();
    EXPECT_STREQ(strShort.String, "hitman 47!");
    EXPECT_TRUE(IsUsingInternalBuffer(strShort));

    std::string upperLong(150, 'G');
    std::string lowerLong(150, 'g');
    MYSTR strLong(upperLong.c_str());
    
    strLong.ToLower();
    EXPECT_STREQ(strLong.String, lowerLong.c_str());
    EXPECT_FALSE(IsUsingInternalBuffer(strLong));

    MYSTR empty;
    empty.ToLower();
    EXPECT_STREQ(empty.String, "");
}