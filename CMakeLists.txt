cmake_minimum_required(VERSION 3.30)

# 只有在作为顶级项目时才声明project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(common VERSION 1.0.0 LANGUAGES CXX)
endif()

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 生成 compile_commands.json 用于 IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(config)
add_subdirectory(logger)
add_subdirectory(argparse)
