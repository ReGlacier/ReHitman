#include <Glacier/ZSTL/SimpleXML.h>
#include <gtest/gtest.h>

#include <cstring>
#include <string>
#include <vector>

using namespace Glacier;

namespace
{
    struct XMLEvent
    {
        std::string Name;
        bool IsStart;
        std::vector<std::pair<std::string, std::string>> Attrs;
    };

    struct CapturingSimpleXML : SimpleXML
    {
        std::vector<XMLEvent> Events;

        void startElement(const char* name, const char** attrs) override
        {
            XMLEvent event;
            event.Name = name ? name : "";
            event.IsStart = true;

            for (int i = 0; attrs && attrs[i]; i += 2)
            {
                event.Attrs.emplace_back(attrs[i], attrs[i + 1] ? attrs[i + 1] : "");
            }

            Events.push_back(event);
        }

        void endElement(const char* name) override
        {
            Events.push_back({name ? name : "", false, {}});
        }
    };

    std::vector<char> MakeBuffer(const char* xml)
    {
        return std::vector<char>(xml, xml + std::strlen(xml));
    }
}

TEST(SimpleXML, ConstructorInitializesEmptyState)
{
    SimpleXML xml;

    EXPECT_EQ(xml.m_pStart, nullptr);
    EXPECT_EQ(xml.m_pEnd, nullptr);
    EXPECT_EQ(xml.m_p, nullptr);
    EXPECT_EQ(xml.m_Len, 0);
    EXPECT_EQ(xml.m_pTag, nullptr);
    EXPECT_EQ(xml.m_nAttributes, 0);
    EXPECT_EQ(xml.m_Attributes[0], nullptr);
}

TEST(SimpleXML, ParseStartAndEndElements)
{
    CapturingSimpleXML xml;
    auto buffer = MakeBuffer("<root><child></child></root>");

    EXPECT_EQ(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), 1);

    ASSERT_EQ(xml.Events.size(), 4u);
    EXPECT_TRUE(xml.Events[0].IsStart);
    EXPECT_EQ(xml.Events[0].Name, "root");
    EXPECT_TRUE(xml.Events[1].IsStart);
    EXPECT_EQ(xml.Events[1].Name, "child");
    EXPECT_FALSE(xml.Events[2].IsStart);
    EXPECT_EQ(xml.Events[2].Name, "child");
    EXPECT_FALSE(xml.Events[3].IsStart);
    EXPECT_EQ(xml.Events[3].Name, "root");
}

TEST(SimpleXML, ParseSelfClosingElementEmitsStartAndEnd)
{
    CapturingSimpleXML xml;
    auto buffer = MakeBuffer("<root><leaf/></root>");

    EXPECT_EQ(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), 1);

    ASSERT_EQ(xml.Events.size(), 4u);
    EXPECT_EQ(xml.Events[0].Name, "root");
    EXPECT_TRUE(xml.Events[0].IsStart);
    EXPECT_EQ(xml.Events[1].Name, "leaf");
    EXPECT_TRUE(xml.Events[1].IsStart);
    EXPECT_EQ(xml.Events[2].Name, "leaf");
    EXPECT_FALSE(xml.Events[2].IsStart);
    EXPECT_EQ(xml.Events[3].Name, "root");
    EXPECT_FALSE(xml.Events[3].IsStart);
}

TEST(SimpleXML, ParseAttributesAndGetAttr)
{
    CapturingSimpleXML xml;
    auto buffer = MakeBuffer("<item id=42 name=agent_47 value=hello-world/>");

    EXPECT_EQ(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), 1);

    ASSERT_GE(xml.Events.size(), 1u);
    ASSERT_EQ(xml.Events[0].Attrs.size(), 3u);

    EXPECT_EQ(xml.Events[0].Attrs[0].first, "id");
    EXPECT_EQ(xml.Events[0].Attrs[0].second, "42");
    EXPECT_EQ(xml.Events[0].Attrs[1].first, "name");
    EXPECT_EQ(xml.Events[0].Attrs[1].second, "agent_47");
    EXPECT_EQ(xml.Events[0].Attrs[2].first, "value");
    EXPECT_EQ(xml.Events[0].Attrs[2].second, "hello-world");

    const char* attrs[] = {"id", "42", "name", "agent_47", nullptr};
    EXPECT_STREQ(xml.GetAttr(attrs, "id"), "42");
    EXPECT_STREQ(xml.GetAttr(attrs, "name"), "agent_47");
    EXPECT_EQ(xml.GetAttr(attrs, "missing"), nullptr);
    EXPECT_EQ(xml.GetAttr(nullptr, "id"), nullptr);
    EXPECT_EQ(xml.GetAttr(attrs, nullptr), nullptr);
}

TEST(SimpleXML, ParseQuotedAttributeKeepsLeadingQuoteLikeOriginalParser)
{
    CapturingSimpleXML xml;
    auto buffer = MakeBuffer("<item name=\"agent\"/>");

    EXPECT_EQ(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), 1);

    ASSERT_GE(xml.Events.size(), 1u);
    ASSERT_EQ(xml.Events[0].Attrs.size(), 1u);
    EXPECT_EQ(xml.Events[0].Attrs[0].first, "name");
    EXPECT_EQ(xml.Events[0].Attrs[0].second, "\"agent");
}

TEST(SimpleXML, CommentsAreSkippedDuringParse)
{
    CapturingSimpleXML xml;
    auto buffer = MakeBuffer("<!--ignored--><root/>");

    EXPECT_EQ(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), 1);

    ASSERT_EQ(xml.Events.size(), 2u);
    EXPECT_EQ(xml.Events[0].Name, "root");
    EXPECT_TRUE(xml.Events[0].IsStart);
    EXPECT_EQ(xml.Events[1].Name, "root");
    EXPECT_FALSE(xml.Events[1].IsStart);
}

TEST(SimpleXML, GetElementCanBeUsedManually)
{
    SimpleXML xml;
    auto buffer = MakeBuffer("<root/><tail></tail>");

    xml.m_pStart = buffer.data();
    xml.m_p = buffer.data();
    xml.m_pEnd = buffer.data() + buffer.size();

    EXPECT_EQ(xml.getElement(), SimpleXML::XML_STARTEND);
    ASSERT_NE(xml.m_pTag, nullptr);
    EXPECT_STREQ(xml.m_pTag, "root");

    EXPECT_EQ(xml.getElement(), SimpleXML::XML_START);
    ASSERT_NE(xml.m_pTag, nullptr);
    EXPECT_STREQ(xml.m_pTag, "tail");

    EXPECT_EQ(xml.getElement(), SimpleXML::XML_END);
    ASSERT_NE(xml.m_pTag, nullptr);
    EXPECT_STREQ(xml.m_pTag, "tail");

    EXPECT_EQ(xml.getElement(), SimpleXML::XML_ENDOFDATA);
}

TEST(SimpleXML, MissingCommentEndThrows)
{
    CapturingSimpleXML xml;
    auto buffer = MakeBuffer("<!--unterminated");

    EXPECT_THROW(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), std::runtime_error);
}

TEST(SimpleXML, MissingEndTagCloseThrows)
{
    CapturingSimpleXML xml;
    auto buffer = MakeBuffer("</root");

    EXPECT_THROW(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), std::runtime_error);
}

TEST(SimpleXML, MissingAttributeEqualsThrows)
{
    CapturingSimpleXML xml;
    auto buffer = MakeBuffer("<root attr value>");

    EXPECT_THROW(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), std::runtime_error);
}

TEST(SimpleXML, MissingStartTagCloseThrows)
{
    CapturingSimpleXML xml;
    auto buffer = MakeBuffer("<root attr=value");

    EXPECT_THROW(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), std::runtime_error);
}

TEST(SimpleXML, InvalidNonTagDataThrows)
{
    CapturingSimpleXML xml;
    auto buffer = MakeBuffer("plain text");

    EXPECT_THROW(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), std::runtime_error);
}

TEST(SimpleXML, AttributeOverflowThrows)
{
    CapturingSimpleXML xml;
    std::string source = "<root";

    for (int i = 0; i < SimpleXML::NATTRIBUTES / 2; ++i)
    {
        source += " a" + std::to_string(i) + "=v";
    }

    source += ">";
    auto buffer = std::vector<char>(source.begin(), source.end());

    EXPECT_THROW(xml.XML_Parse(buffer.data(), static_cast<int>(buffer.size()), 1), std::runtime_error);
}
