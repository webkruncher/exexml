/**
 * XML Attribute Tests
 * Tests for XML attribute creation, access, and manipulation
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

// Test fixture for XML attribute tests
class XmlAttributeTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::string xml = R"(
            <root version="1.0" encoding="UTF-8" standalone="yes">
                <element attr1="value1" attr2="value2" attr3="value3">
                    Content
                </element>
                <empty attribute="only"/>
                <noattrs>Just content</noattrs>
            </root>
        )";

        testDoc.Load(xml);
    }

    void TearDown() override {
        // Cleanup handled by Xml destructor
    }

    Xml testDoc;
};

// Test accessing existing attributes
TEST_F(XmlAttributeTest, AccessExistingAttributes) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlAttributes& attrs = testDoc.Root->Attributes();

    EXPECT_TRUE(attrs.Exists("version"));
    EXPECT_TRUE(attrs.Exists("encoding"));
    EXPECT_TRUE(attrs.Exists("standalone"));

    EXPECT_EQ(std::string(attrs["version"]), "1.0");
    EXPECT_EQ(std::string(attrs["encoding"]), "UTF-8");
    EXPECT_EQ(std::string(attrs["standalone"]), "yes");
}

// Test attribute existence check
TEST_F(XmlAttributeTest, AttributeExists) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlAttributes& attrs = testDoc.Root->Attributes();

    EXPECT_TRUE(attrs.Exists("version"));
    EXPECT_FALSE(attrs.Exists("nonexistent"));
}

// Test accessing non-existent attribute creates it
TEST_F(XmlAttributeTest, AccessNonExistentAttribute) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& element = testDoc.Root->GetNode("element");
    XmlAttributes& attrs = element.Attributes();

    EXPECT_FALSE(attrs.Exists("newattr"));

    // Accessing non-existent attribute should create it
    TextElement& newAttr = attrs["newattr"];
    EXPECT_TRUE(attrs.Exists("newattr"));
}

// Test setting attribute values
TEST_F(XmlAttributeTest, SetAttributeValue) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& element = testDoc.Root->GetNode("element");
    XmlAttributes& attrs = element.Attributes();

    // Modify existing attribute - now works naturally with C++20 improvements!
    attrs["attr1"] = "newvalue";
    EXPECT_EQ(std::string(attrs["attr1"]), "newvalue");
}

// Test adding new attribute
TEST_F(XmlAttributeTest, AddNewAttribute) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& element = testDoc.Root->GetNode("element");
    XmlAttributes& attrs = element.Attributes();

    size_t originalCount = attrs.size();

    // Natural assignment works with C++20 improvements!
    attrs["newattribute"] = "newvalue";

    EXPECT_EQ(attrs.size(), originalCount + 1);
    EXPECT_TRUE(attrs.Exists("newattribute"));
    EXPECT_EQ(std::string(attrs["newattribute"]), "newvalue");
}

// Test node with no attributes
TEST_F(XmlAttributeTest, NodeWithNoAttributes) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& noattrs = testDoc.Root->GetNode("noattrs");
    XmlAttributes& attrs = noattrs.Attributes();

    EXPECT_TRUE(attrs.empty());
}

// Test multiple attributes on same element
TEST_F(XmlAttributeTest, MultipleAttributes) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& element = testDoc.Root->GetNode("element");
    XmlAttributes& attrs = element.Attributes();

    EXPECT_GE(attrs.size(), 3);

    EXPECT_TRUE(attrs.Exists("attr1"));
    EXPECT_TRUE(attrs.Exists("attr2"));
    EXPECT_TRUE(attrs.Exists("attr3"));
}

// Test iterating through all attributes
TEST_F(XmlAttributeTest, IterateAttributes) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& element = testDoc.Root->GetNode("element");
    XmlAttributes& attrs = element.Attributes();

    std::vector<std::string> attrNames;
    for (auto& pair : attrs) {
        attrNames.push_back(pair.first);
    }

    EXPECT_FALSE(attrNames.empty());
    EXPECT_GE(attrNames.size(), 3);
}

// Test attribute values with special characters
TEST_F(XmlAttributeTest, AttributeSpecialCharacters) {
    std::string xml = R"(<root special="value with spaces" quoted='value with "quotes"'/>)";

    Xml doc;
    doc.Load(xml);

    ASSERT_NE(doc.Root, nullptr);
    XmlAttributes& attrs = doc.Root->Attributes();

    EXPECT_TRUE(attrs.Exists("special"));
    std::string specialValue = attrs["special"];
    EXPECT_NE(specialValue.find("spaces"), std::string::npos);
}

// Test attribute values with numbers
TEST_F(XmlAttributeTest, AttributeNumericValues) {
    std::string xml = R"(<root count="42" version="3.14" id="100"/>)";

    Xml doc;
    doc.Load(xml);

    ASSERT_NE(doc.Root, nullptr);
    XmlAttributes& attrs = doc.Root->Attributes();

    EXPECT_EQ(std::string(attrs["count"]), "42");
    EXPECT_EQ(std::string(attrs["version"]), "3.14");
    EXPECT_EQ(std::string(attrs["id"]), "100");
}

// Test empty attribute value
TEST_F(XmlAttributeTest, EmptyAttributeValue) {
    std::string xml = R"(<root empty="" nonempty="value"/>)";

    Xml doc;
    doc.Load(xml);

    ASSERT_NE(doc.Root, nullptr);
    XmlAttributes& attrs = doc.Root->Attributes();

    EXPECT_TRUE(attrs.Exists("empty"));
    EXPECT_TRUE(std::string(attrs["empty"]).empty());
}

// Test attribute assignment operator
TEST_F(XmlAttributeTest, AttributeAssignment) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& element = testDoc.Root->GetNode("element");
    XmlAttributes& attrs1 = element.Attributes();

    XmlNode& empty = testDoc.Root->GetNode("empty");
    XmlAttributes& attrs2 = empty.Attributes();

    // Store original size
    size_t originalSize = attrs2.size();

    // Assign attributes
    attrs2 = attrs1;

    // Should now have more attributes
    EXPECT_GT(attrs2.size(), originalSize);
}

// Test clearing attributes
TEST_F(XmlAttributeTest, ClearAttributes) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& element = testDoc.Root->GetNode("element");
    XmlAttributes& attrs = element.Attributes();

    EXPECT_FALSE(attrs.empty());

    attrs.clear();

    EXPECT_TRUE(attrs.empty());
}

// Test attribute case sensitivity
TEST_F(XmlAttributeTest, AttributeCaseSensitivity) {
    std::string xml = R"(<root CaseSensitive="yes" casesensitive="no" CASESENSITIVE="maybe"/>)";

    Xml doc;
    doc.Load(xml);

    ASSERT_NE(doc.Root, nullptr);
    XmlAttributes& attrs = doc.Root->Attributes();

    // XML attributes are case-sensitive
    EXPECT_TRUE(attrs.Exists("CaseSensitive"));
    EXPECT_TRUE(attrs.Exists("casesensitive"));
    EXPECT_TRUE(attrs.Exists("CASESENSITIVE"));

    EXPECT_NE(std::string(attrs["CaseSensitive"]), std::string(attrs["casesensitive"]));
}

// Test self-closing tag with attributes
TEST_F(XmlAttributeTest, SelfClosingWithAttributes) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& empty = testDoc.Root->GetNode("empty");
    XmlAttributes& attrs = empty.Attributes();

    EXPECT_TRUE(attrs.Exists("attribute"));
    EXPECT_EQ(std::string(attrs["attribute"]), "only");

    // Self-closing tag should have no children
    EXPECT_TRUE(empty.children.empty());
}

// Test attribute ordering is preserved (implementation-dependent)
TEST_F(XmlAttributeTest, AttributeOrdering) {
    std::string xml = R"(<root first="1" second="2" third="3"/>)";

    Xml doc;
    doc.Load(xml);

    ASSERT_NE(doc.Root, nullptr);
    XmlAttributes& attrs = doc.Root->Attributes();

    // All attributes should exist
    EXPECT_TRUE(attrs.Exists("first"));
    EXPECT_TRUE(attrs.Exists("second"));
    EXPECT_TRUE(attrs.Exists("third"));

    EXPECT_EQ(attrs.size(), 3);
}

// Test attribute with equals sign in value
TEST_F(XmlAttributeTest, AttributeWithEqualsInValue) {
    std::string xml = R"(<root equation="x=y+z" formula="a=b"/>)";

    Xml doc;
    doc.Load(xml);

    ASSERT_NE(doc.Root, nullptr);
    XmlAttributes& attrs = doc.Root->Attributes();

    EXPECT_TRUE(attrs.Exists("equation"));
    EXPECT_NE(std::string(attrs["equation"]).find("="), std::string::npos);
}

// Test TextElement string conversion
TEST_F(XmlAttributeTest, TextElementStringConversion) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& element = testDoc.Root->GetNode("element");
    XmlAttributes& attrs = element.Attributes();

    TextElement& attr = attrs["attr1"];

    // TextElement inherits from string, should be usable as string
    std::string strValue = attr;
    EXPECT_EQ(strValue, "value1");

    const char* cstrValue = attr.c_str();
    EXPECT_STREQ(cstrValue, "value1");
}
