/**
 * Main GoogleTest runner for exexml library
 *
 * This file serves as the entry point for all exexml tests.
 * Individual test suites are organized in separate files:
 * - test_xml_parsing.cpp: XML document parsing tests
 * - test_xml_nodes.cpp: XML node manipulation tests
 * - test_xml_attributes.cpp: XML attribute handling tests
 */

#include <gtest/gtest.h>

// GoogleTest will automatically run all tests when main() calls RUN_ALL_TESTS()
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
