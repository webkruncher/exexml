/**
 * XML Node Manipulation Tests
 * Tests for XML node creation, traversal, and manipulation
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

// Test fixture for XML node tests
class XmlNodeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a standard test document
        std::string xml = R"(
            <root id="root1">
                <parent id="parent1">
                    <child name="first">Content 1</child>
                    <child name="second">Content 2</child>
                    <child name="third">Content 3</child>
                </parent>
                <sibling>Sibling content</sibling>
            </root>
        )";

        testDoc.Load(xml);
    }

    void TearDown() override {
        // Cleanup handled by Xml destructor
    }

    Xml testDoc;
};

// Test getting child nodes by name
TEST_F(XmlNodeTest, GetChildNodeByName) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& parent = testDoc.Root->GetNode("parent");
    EXPECT_EQ(parent.Name(), "parent");

    XmlNode& child1 = parent.GetNode("child");
    EXPECT_EQ(child1.Name(), "child");
}

// Test getting non-existent node throws exception
TEST_F(XmlNodeTest, GetNonExistentNode) {
    ASSERT_NE(testDoc.Root, nullptr);

    EXPECT_THROW(testDoc.Root->GetNode("nonexistent"), XmlException);
}

// Test finding node (returns pointer, can be NULL)
TEST_F(XmlNodeTest, FindNodeByName) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode* found = testDoc.Root->FindNode("parent");
    ASSERT_NE(found, nullptr);
    EXPECT_EQ(found->Name(), "parent");

    XmlNode* notFound = testDoc.Root->FindNode("nonexistent");
    EXPECT_EQ(notFound, nullptr);
}

// Test accessing node attributes
TEST_F(XmlNodeTest, AccessNodeAttributes) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlAttributes& attrs = testDoc.Root->Attributes();
    EXPECT_TRUE(attrs.Exists("id"));
    EXPECT_EQ(std::string(attrs["id"]), "root1");
}

// Test checking if attribute exists
TEST_F(XmlNodeTest, AttributeExists) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& parent = testDoc.Root->GetNode("parent");
    XmlAttributes& attrs = parent.Attributes();

    EXPECT_TRUE(attrs.Exists("id"));
    EXPECT_FALSE(attrs.Exists("nonexistent"));
}

// Test node name access
TEST_F(XmlNodeTest, NodeName) {
    ASSERT_NE(testDoc.Root, nullptr);

    EXPECT_EQ(testDoc.Root->Name(), "root");

    XmlNode& parent = testDoc.Root->GetNode("parent");
    EXPECT_EQ(parent.Name(), "parent");
}

// Test getting parent node
TEST_F(XmlNodeTest, GetParentNode) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& parent = testDoc.Root->GetNode("parent");
    XmlNode& child = parent.GetNode("child");

    XmlNode& retrievedParent = child.GetParent();
    EXPECT_EQ(retrievedParent.Name(), "parent");
}

// Test root node has no parent
TEST_F(XmlNodeTest, RootHasNoParent) {
    ASSERT_NE(testDoc.Root, nullptr);

    EXPECT_EQ(testDoc.Root->Parent(), nullptr);
}

// Test getting root from deep node
TEST_F(XmlNodeTest, GetRootFromDeepNode) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& parent = testDoc.Root->GetNode("parent");
    XmlNode& child = parent.GetNode("child");

    const XmlNode* root = child.Root();
    ASSERT_NE(root, nullptr);
    // Now Name() is const-correct!
    EXPECT_EQ(root->Name(), "root");
}

// Test node children collection
TEST_F(XmlNodeTest, NodeChildren) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& parent = testDoc.Root->GetNode("parent");

    // The parent node should have children
    EXPECT_FALSE(parent.children.empty());
    EXPECT_GE(parent.children.size(), 1);
}

// Test node has correct number of children
TEST_F(XmlNodeTest, CountChildren) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& parent = testDoc.Root->GetNode("parent");

    // Parent should have 3 child nodes (all named "child")
    size_t childCount = 0;
    for (auto* elem : parent.children) {
        XmlNode& node = static_cast<XmlNode&>(*elem);
        if (node.Name() == "child") {
            childCount++;
        }
    }

    EXPECT_EQ(childCount, 3);
}

// Test traversing all children
TEST_F(XmlNodeTest, TraverseChildren) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& parent = testDoc.Root->GetNode("parent");

    std::vector<std::string> childNames;
    for (auto* elem : parent.children) {
        XmlNode& node = static_cast<XmlNode&>(*elem);
        childNames.push_back(node.Name());
    }

    EXPECT_FALSE(childNames.empty());
    EXPECT_EQ(childNames[0], "child");
}

// Test node output operator
TEST_F(XmlNodeTest, NodeOutputStream) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& parent = testDoc.Root->GetNode("parent");

    std::ostringstream oss;
    oss << parent;

    std::string output = oss.str();
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("parent"), std::string::npos);
}

// Test tab level (indentation tracking)
TEST_F(XmlNodeTest, TabLevel) {
    ASSERT_NE(testDoc.Root, nullptr);

    int rootLevel = testDoc.Root->TabLevel();
    EXPECT_GE(rootLevel, 0);

    XmlNode& parent = testDoc.Root->GetNode("parent");
    int parentLevel = parent.TabLevel();

    // Child should be indented more than root
    EXPECT_GT(parentLevel, rootLevel);
}

// Test node Generate method (creating new child)
TEST_F(XmlNodeTest, GenerateNewChild) {
    ASSERT_NE(testDoc.Root, nullptr);

    size_t originalChildCount = testDoc.Root->children.size();

    XmlNodeBase* newChild = testDoc.Root->Generate(testDoc.Root, "newchild");
    ASSERT_NE(newChild, nullptr);

    size_t newChildCount = testDoc.Root->children.size();
    EXPECT_EQ(newChildCount, originalChildCount + 1);
}

// Test node copy via assignment
TEST_F(XmlNodeTest, NodeAssignment) {
    std::string xml1 = "<node1>Content1</node1>";
    std::string xml2 = "<node2>Content2</node2>";

    Xml doc1, doc2;
    doc1.Load(xml1);
    doc2.Load(xml2);

    ASSERT_NE(doc1.Root, nullptr);
    ASSERT_NE(doc2.Root, nullptr);

    *doc2.Root = *doc1.Root;

    EXPECT_EQ(doc2.Root->Name(), doc1.Root->Name());
}

// Test clearing a node
TEST_F(XmlNodeTest, ClearNode) {
    std::string xml = "<root><child1/><child2/></root>";
    Xml doc;
    doc.Load(xml);

    ASSERT_NE(doc.Root, nullptr);
    EXPECT_FALSE(doc.Root->children.empty());

    doc.Root->clear();

    EXPECT_TRUE(doc.Root->children.empty());
}

// Test node with no children
TEST_F(XmlNodeTest, NodeWithNoChildren) {
    std::string xml = "<root/>";
    Xml doc;
    doc.Load(xml);

    ASSERT_NE(doc.Root, nullptr);
    EXPECT_TRUE(doc.Root->children.empty());
}

// Test accessing sibling nodes
TEST_F(XmlNodeTest, AccessSiblings) {
    ASSERT_NE(testDoc.Root, nullptr);

    // Get first child
    XmlNode& parent = testDoc.Root->GetNode("parent");
    EXPECT_NO_THROW(parent.Name());

    // Get sibling
    XmlNode& sibling = testDoc.Root->GetNode("sibling");
    EXPECT_EQ(sibling.Name(), "sibling");
}

// Test document root accessor via cast
TEST_F(XmlNodeTest, DocumentRootViaCast) {
    ASSERT_NE(testDoc.Root, nullptr);

    XmlNode& root = static_cast<XmlNode&>(testDoc);
    EXPECT_EQ(root.Name(), "root");
}

// Test operator() returns true for valid nodes
TEST_F(XmlNodeTest, NodeOperatorCall) {
    ASSERT_NE(testDoc.Root, nullptr);

    bool result = (*testDoc.Root)();
    // Should return true for successfully processing all children
    EXPECT_TRUE(result);
}
