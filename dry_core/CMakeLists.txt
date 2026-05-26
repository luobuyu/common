# dry_core: 底层纯工具库
# 包含 dry_string.h、dry_time.h、dry_random.h、dry_url.h、dry_mime.h、dry_pool.h
# 大部分 header-only，dry_url.cpp 有实现文件
add_library(dry_core dry_url.cpp)
target_include_directories(dry_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)