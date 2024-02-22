cmake_minimum_required(VERSION 3.14.0)

project(simple_http
    VERSION 0.1.0
)

add_library(
    simple_http
    STATIC
    "include/simple_http.h"
    "lib/init_library.h"
    "lib/init_library.cc"
    "lib/init_socket_library.h"
    "lib/init_socket_library.cc"
    "lib/http_server.h"
    "lib/http_server.cc"
    "lib/server.h"
    "lib/server.cc"
    "lib/socket.h"
    "lib/socket.cc"
    "lib/socket_reader.h"
    "lib/socket_reader.cc"
)

target_compile_features(simple_http PUBLIC cxx_std_20)

target_include_directories(simple_http INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
