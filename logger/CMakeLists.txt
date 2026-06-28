add_library(logger log.cpp log_manager.cpp sink.cpp log_event.cpp log_format.cpp)

target_include_directories(logger PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(logger PUBLIC dry_core)
# fmt：由顶层 CMakeLists 提供 fmt::fmt target（本地已装 or third_party fallback）
if(TARGET fmt::fmt)
    message(STATUS "fmt library found (FMT_FOUND enabled, LOGF_* available)")
    target_compile_definitions(logger PUBLIC FMT_FOUND)
    target_link_libraries(logger PUBLIC fmt::fmt)
else()
    message(STATUS "fmt library not found (LOGF_* disabled, LOG_* falls back to snprintf)")
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