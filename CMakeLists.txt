cmake_minimum_required(VERSION 3.1)

project(argcpp17 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
include(GoogleTest)

set(ARGCPP17_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ARGCPP17_HEADERS
    ${ARGCPP17_INCLUDE_DIR}/argcpp17.h
)

find_package(GTest REQUIRED)

add_library(argcpp17 INTERFACE)
target_include_directories(argcpp17 INTERFACE
    $<BUILD_INTERFACE:${ARGCPP17_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:include>)

add_executable(argcpp17_example example/main.cpp)
target_link_libraries(argcpp17_example argcpp17)

add_executable(argcpp17_test test/test.cpp)
target_link_libraries(argcpp17_test gtest_main argcpp17)
gtest_add_tests(TARGET argcpp17_test TEST_SUFFIX .noArgs TEST_LIST noArgsTests)