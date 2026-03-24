cmake_minimum_required(VERSION 3.14)

add_library(config config.cpp)
target_link_libraries(config PUBLIC dry_core)