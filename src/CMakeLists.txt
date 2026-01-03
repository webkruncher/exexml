
cmake_minimum_required(VERSION 3.10)

project("exexml" CXX)

# Require C++20 for modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../krunchercore/cmake/CMakeLists.txt)

add_subdirectory( include )
#add_subdirectory( ut )
add_subdirectory( tests )


