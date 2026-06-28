cmake_minimum_required(VERSION 3.14)

add_library(dry_thread thread_pool.cpp
  sharded_thread_pool.cpp)
target_link_libraries(dry_thread PUBLIC dry_core)
target_include_directories(dry_thread PUBLIC ${PROJECT_SOURCE_DIR})