# argparse library
add_library(argparse
    # 基类
    argument.cpp
    argument.h
    
    # 具体实现类 (非模板)
    flag_argument.cpp
    flag_argument.h
    argument_parser.cpp
    argument_parser.h
    argument_parser.inc
    
    # 模板类 (仅头文件,实现在 .inc 中)
    option_argument.h
    option_argument.inc
    positional_argument.h
    positional_argument.inc
)

# 设置头文件搜索路径,让其他项目能找到这些头文件
target_include_directories(argparse PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})