cmake_minimum_required (VERSION 3.14)
project (a0a CXX)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic")
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -s -DNDEBUG -march=native")
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

find_package (Threads REQUIRED)
find_package (GFlags REQUIRED)
find_package (Glog REQUIRED)
find_package (GTest REQUIRED)
find_package (cuDNN REQUIRED)

#
# Version
#
execute_process (
  COMMAND git describe --tags --always
  OUTPUT_VARIABLE VERSION
  ERROR_VARIABLE GIT_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file (
  cmake/version.h.in
  ${CMAKE_BINARY_DIR}/version.h
)

#
# Testing
#
enable_testing ()

#
# Sources
#
add_subdirectory (src)
