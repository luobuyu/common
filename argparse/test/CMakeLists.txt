cmake_minimum_required(VERSION 3.10)
project(argparse_test)

# 添加 argparse 库
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/argparse)

# argparse 测试
add_executable(test_argparse test_argparse.cpp)
target_link_libraries(test_argparse argparse)
