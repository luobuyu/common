# 查找 fmt 库
find_package(fmt)

add_library(log log.cpp sink.cpp log_event.cpp log_format.cpp)
if(fmt_FOUND)
    message(STATUS "fmt library found")
    # 定义宏以便在代码中使用
    target_compile_definitions(log PUBLIC FMT_FOUND)
    target_link_libraries(log fmt)
else()
    message(WARNING "fmt library not found")
endif()
target_compile_options(log PUBLIC -Wno-format-security)