# 查找 fmt 库
find_package(fmt QUIET)

add_library(logger log.cpp log_manager.cpp sink.cpp log_event.cpp log_format.cpp)

target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(logger PUBLIC dry_core)
if(fmt_FOUND)
    message(STATUS "fmt library found")
    # 定义宏以便在代码中使用
    target_compile_definitions(logger PUBLIC FMT_FOUND)
    target_link_libraries(logger PUBLIC fmt)
else()
    message(STATUS "fmt library not found")
endif()
if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(logger PUBLIC Threads::Threads)
endif()
target_compile_options(logger PUBLIC -Wno-format-security)

# 仅当使用GCC且版本小于9时需要链接stdc++fs
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(logger PUBLIC stdc++fs)
endif()