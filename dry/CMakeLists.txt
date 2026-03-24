# dry: 胶水层库，提供日志系统的便捷初始化封装
# 依赖 log（logger）和 config
add_library(dry dry_log.cpp)
target_link_libraries(dry PUBLIC log config)