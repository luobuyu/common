# dry_core: 底层纯工具库（header-only）
# 包含 dry_string.h、dry_time.h、dry_random.h，只依赖标准库
add_library(dry_core INTERFACE)
target_include_directories(dry_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)