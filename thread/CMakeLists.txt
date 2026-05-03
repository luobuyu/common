cmake_minimum_required(VERSION 3.14)

add_library(dry_thread thread_pool.cpp)
target_link_libraries(dry_thread PUBLIC dry_core)