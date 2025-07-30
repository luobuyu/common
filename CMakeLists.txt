cmake_minimum_required(VERSION 3.30)

# 只有在作为顶级项目时才声明project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(common VERSION 1.0.0 LANGUAGES CXX)
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(config)
add_subdirectory(logger)
