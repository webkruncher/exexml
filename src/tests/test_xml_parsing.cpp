/**
 * XML Parsing Tests
 * Tests for basic XML document parsing functionality
 */

#include <gtest/gtest.h>

// exexml.h requires these to be included first
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <queue>
#include <stack>
#include <deque>

using namespace std;

#include <exexml.h>

using namespace XmlFamily;

// Test fixture for XML parsing tests
class XmlParsingTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

// Test basic XML document creation
TEST_F(XmlParsingTest, CreateEmptyDocument) {
    Xml doc;
    EXPECT_EQ(doc.Root, nullptr);
}

// Test loading simple XML from file
TEST_F(XmlParsingTest, LoadSimpleXmlFromFile) {
    std::ifstream file("test_data/simple.xml");
    ASSERT_TRUE(file.is_open()) << "Failed to open test_data/simple.xml";

    Xml doc;
    EXPECT_NO_THROW(doc.Load(file));
    ASSERT_NE(doc.Root, nullptr);
    EXPECT_EQ(doc.Root->Name(), "root");
}

// Test loading XML from string
TEST_F(XmlParsingTest, LoadXmlFromString) {
    std::string xml = "<root><child>Hello</child></root>";
    Xml doc;
    EXPECT_NO_THROW(doc.Load(xml));
    ASSERT_NE(doc.Root, nullptr);
    EXPECT_EQ(doc.Root->Name(), "root");
}

// Test loading XML with attributes
TEST_F(XmlParsingTest, LoadXmlWithAttributes) {
    std::string xml = R"(<root attr1="value1" attr2="value2"><child>Content</child></root>)";
    Xml doc;
    EXPECT_NO_THROW(doc.Load(xml));
    ASSERT_NE(doc.Root, nullptr);

    auto& attrs = doc.Root->Attributes();
    EXPECT_TRUE(attrs.Exists("attr1"));
    EXPECT_TRUE(attrs.Exists("attr2"));
    EXPECT_EQ(std::string(attrs["attr1"]), "value1");
    EXPECT_EQ(std::string(attrs["attr2"]), "value2");
}

// Test loading complex nested XML
TEST_F(XmlParsingTest, LoadComplexXml) {
    std::ifstream file("test_data/complex.xml");
    ASSERT_TRUE(file.is_open()) << "Failed to open test_data/complex.xml";

    Xml doc;
    EXPECT_NO_THROW(doc.Load(file));
    ASSERT_NE(doc.Root, nullptr);
    EXPECT_EQ(doc.Root->Name(), "root");
}

// Test loading XML with CDATA sections
TEST_F(XmlParsingTest, LoadXmlWithCDATA) {
    std::ifstream file("test_data/with_cdata.xml");
    ASSERT_TRUE(file.is_open()) << "Failed to open test_data/with_cdata.xml";

    Xml doc;
    EXPECT_NO_THROW(doc.Load(file));
    ASSERT_NE(doc.Root, nullptr);
}

// Test loading XML with comments (comments should be stripped)
TEST_F(XmlParsingTest, LoadXmlWithComments) {
    std::string xml = R"(
        <root>
            <!-- This is a comment -->
            <child>Content</child>
            <!-- Another comment -->
        </root>
    )";

    Xml doc;
    EXPECT_NO_THROW(doc.Load(xml));
    ASSERT_NE(doc.Root, nullptr);
}

// Test loading XML with XML declaration header
TEST_F(XmlParsingTest, LoadXmlWithDeclaration) {
    std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
        <root>
            <child>Content</child>
        </root>
    )";

    Xml doc;
    EXPECT_NO_THROW(doc.Load(xml));
    ASSERT_NE(doc.Root, nullptr);
    EXPECT_FALSE(doc.Headers.empty()) << "XML declaration should be captured in Headers";
}

// Test loading malformed XML - empty string
TEST_F(XmlParsingTest, LoadEmptyString) {
    std::string xml = "";
    Xml doc;
    // Empty string should throw or handle gracefully
    EXPECT_ANY_THROW(doc.Load(xml));
}

// Test loading malformed XML - unclosed tags
TEST_F(XmlParsingTest, LoadUnclosedTags) {
    std::string xml = "<root><child>Content</root>";
    Xml doc;
    // Malformed XML should be handled - may throw or parse incorrectly
    // This documents current behavior
    EXPECT_ANY_THROW(doc.Load(xml));
}

// Test XML document copy constructor
TEST_F(XmlParsingTest, CopyConstructor) {
    std::string xml = "<root><child>Content</child></root>";
    Xml doc1;
    doc1.Load(xml);

    Xml doc2(doc1);
    ASSERT_NE(doc2.Root, nullptr);
    EXPECT_EQ(doc2.Root->Name(), doc1.Root->Name());
}

// Test XML document assignment operator
TEST_F(XmlParsingTest, AssignmentOperator) {
    std::string xml = "<root><child>Content</child></root>";
    Xml doc1;
    doc1.Load(xml);

    Xml doc2;
    doc2 = doc1;
    ASSERT_NE(doc2.Root, nullptr);
    EXPECT_EQ(doc2.Root->Name(), doc1.Root->Name());
}

// Test writing XML document back to stream
TEST_F(XmlParsingTest, WriteXmlToStream) {
    std::string xml = "<root><child>Content</child></root>";
    Xml doc;
    doc.Load(xml);

    std::ostringstream output;
    output << doc;
    std::string result = output.str();

    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("root"), std::string::npos);
    EXPECT_NE(result.find("child"), std::string::npos);
}

// Test XML with multiple siblings at same level
TEST_F(XmlParsingTest, MultipleSiblings) {
    std::string xml = R"(
        <root>
            <child1>Content1</child1>
            <child2>Content2</child2>
            <child3>Content3</child3>
        </root>
    )";

    Xml doc;
    EXPECT_NO_THROW(doc.Load(xml));
    ASSERT_NE(doc.Root, nullptr);

    // Verify we can find all children
    EXPECT_NO_THROW(doc.Root->GetNode("child1"));
    EXPECT_NO_THROW(doc.Root->GetNode("child2"));
    EXPECT_NO_THROW(doc.Root->GetNode("child3"));
}

// Test self-closing tags
TEST_F(XmlParsingTest, SelfClosingTags) {
    std::string xml = R"(<root><empty/><another/></root>)";

    Xml doc;
    EXPECT_NO_THROW(doc.Load(xml));
    ASSERT_NE(doc.Root, nullptr);
}

// Test XML with mixed content (text and elements)
TEST_F(XmlParsingTest, MixedContent) {
    std::string xml = R"(
        <root>
            Text before
            <child>Inner</child>
            Text after
        </root>
    )";

    Xml doc;
    EXPECT_NO_THROW(doc.Load(xml));
    ASSERT_NE(doc.Root, nullptr);
}

// Test deeply nested XML
TEST_F(XmlParsingTest, DeeplyNestedXml) {
    std::string xml = R"(
        <level1>
            <level2>
                <level3>
                    <level4>
                        <level5>Deep content</level5>
                    </level4>
                </level3>
            </level2>
        </level1>
    )";

    Xml doc;
    EXPECT_NO_THROW(doc.Load(xml));
    ASSERT_NE(doc.Root, nullptr);
    EXPECT_EQ(doc.Root->Name(), "level1");
}
