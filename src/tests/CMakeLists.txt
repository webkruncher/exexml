# GoogleTest configuration for exexml
cmake_minimum_required(VERSION 3.14)

# Ensure C++20 is used (should inherit from parent, but make it explicit)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch GoogleTest from GitHub
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Get include paths from parent
get_filename_component(EXEXML_INCLUDE_PATH ../include/ REALPATH)

# exexml is designed to be standalone - no krunchercore dependency for tests
message(STATUS "Building exexml tests as standalone (no krunchercore dependency)")
set(INCLUDE_PATHS "")
set(LIBRARY_PATHS "")
set(LIBRARIES "")

# Test executable for XML parser
add_executable(
  test_exexml
    test_exexml.cpp
    test_xml_parsing.cpp
    test_xml_nodes.cpp
    test_xml_attributes.cpp
)

target_include_directories(
  test_exexml
    PRIVATE
      ${EXEXML_INCLUDE_PATH}
      ${CMAKE_CURRENT_SOURCE_DIR}
)

# Undefine PROPERTIES_BASE to make exexml truly standalone
target_compile_options(
  test_exexml
    PRIVATE
      -UPROPERTIES_BASE
)

target_link_libraries(
  test_exexml
    PRIVATE
      GTest::gtest_main
      GTest::gmock
)

# Discover tests for CTest integration
include(GoogleTest)
gtest_discover_tests(test_exexml)

# Copy test data files to build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_data/simple.xml
               ${CMAKE_CURRENT_BINARY_DIR}/test_data/simple.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_data/complex.xml
               ${CMAKE_CURRENT_BINARY_DIR}/test_data/complex.xml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_data/with_cdata.xml
               ${CMAKE_CURRENT_BINARY_DIR}/test_data/with_cdata.xml COPYONLY)
