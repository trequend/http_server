cmake_minimum_required(VERSION 3.14.0)

add_subdirectory("simple_http")

project(http_server
    VERSION 0.1.0
)

add_executable(
    http_server
    "src/main.cc"
    "src/scope_guard.h"
)

target_compile_features(http_server PUBLIC cxx_std_20)

target_link_libraries(http_server PUBLIC simple_http)
