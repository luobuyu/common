cmake_minimum_required(VERSION 3.14)

add_library(thread thread_pool.cpp)
target_link_libraries(thread PUBLIC dry_core)