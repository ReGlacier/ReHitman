#include <gtest/gtest.h>

#include <Glacier/GUI/ZSIMPLEHTML.h>
#include <Glacier/RTP/VirtualTables.h>

using namespace Glacier;

TEST(ZSIMPLEHTMLParser, RecognizesPcTagCodes)
{
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("COLO"), 0);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/COL"), 1);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("AUTO"), 2);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/AUT"), 3);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("WIDT"), 4);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/WID"), 5);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("INDE"), 6);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/IND"), 7);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("FONT"), 8);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/FON"), 9);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("LINE"), 10);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/LIN"), 11);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("KERN"), 12);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/KER"), 13);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("JUST"), 14);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/JUS"), 15);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("GUTT"), 16);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/GUT"), 17);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("!--"), 18);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("SPAC"), 19);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("CAPS"), 20);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/CAP"), 21);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("TABS"), 22);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/TAB"), 23);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("BEFO"), 24);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/BEF"), 25);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("IMG"), 27);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/IMG"), 26);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("BR"), 28);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("LT"), 29);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("GT"), 30);
}

TEST(ZSIMPLEHTMLParser, IgnoresCharactersAfterPcTagCode)
{
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("COLO=FF00FF"), 0);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("/FON extra"), 9);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType("unknown"), -1);
    EXPECT_EQ(ZSIMPLEHTML::GetTagType(nullptr), -1);
}

TEST(ZSIMPLEHTMLParser, CharacterClassification)
{
    // These helpers are stateless in the PC implementation.
    auto* html = reinterpret_cast<ZSIMPLEHTML*>(static_cast<uintptr_t>(0x1));
    EXPECT_TRUE(html->IsWhiteSpace(' '));
    EXPECT_TRUE(html->IsWhiteSpace('\t'));
    EXPECT_TRUE(html->IsWhiteSpace('\n'));
    EXPECT_TRUE(html->IsNonWord('='));
    EXPECT_TRUE(html->IsPunctuation('.'));
    EXPECT_FALSE(html->IsWhiteSpace('A'));
    EXPECT_FALSE(html->IsPunctuation('A'));
}

TEST(ZSIMPLEHTMLParser, PropertyTableMatchesPcDescriptor)
{
    const RTP::ZPropertyInfo& info = ZSIMPLEHTML::Info;
    ASSERT_NE(info.First, nullptr);
    EXPECT_EQ(info.Super, &ZCHAROBJ::Info);
    EXPECT_STREQ(info.Name, "ZSIMPLEHTML");
    EXPECT_EQ(info.First->m_Next, nullptr);
    EXPECT_STREQ(info.First->m_Name, "m_pvFontsList");
    EXPECT_EQ(info.First->m_Filter, 1u);

    const auto* property = reinterpret_cast<const RTP::ZVirtualProperty<REFTAB32>*>(info.First);
    EXPECT_EQ(property->m_VirtualTable, &RTP::VirtualTables::Virtual_REFTAB32);
}
