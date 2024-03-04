cmake_minimum_required(VERSION 3.14.0)

project(simple_http
    VERSION 0.1.0
)

set(
    PROJECT_FILES
    "include/simple_http.h"
    "lib/init_library.h"
    "lib/init_library.cc"
    "lib/init_socket_library.h"
    "lib/init_socket_library.cc"
    "lib/socket_descriptor.h"
    "lib/server.h"
    "lib/server.cc"
    "lib/socket.h"
    "lib/socket.cc"
    "lib/socket_reader.h"
    "lib/socket_reader.cc"
    "lib/socket_writer.h"
    "lib/socket_writer.cc"
    "lib/http_headers.h"
    "lib/http_headers.cc"
    "lib/base_parser.cc"
    "lib/base_parser.h"
    "lib/http_parser.h"
    "lib/http_parser.cc"
    "lib/http_uri_parser.h"
    "lib/http_uri_parser.cc"
    "lib/http_version.h"
    "lib/http_method.h"
    "lib/http_connection_handler.h"
    "lib/http_connection.h"
    "lib/http_connection.cc"
    "lib/message_body.h"
    "lib/zero_message_body.h"
    "lib/zero_message_body.cc"
    "lib/content_length_message_body.h"
    "lib/content_length_message_body.cc"
    "lib/http_request_data.h"
    "lib/incoming_message.h"
    "lib/incoming_message.cc"
    "lib/outgoing_message.h"
    "lib/outgoing_message.cc"
    "lib/utils.h"
    "lib/utils.cc"
    "lib/thread_pool.h"
    "lib/http_server.h"
    "lib/http_server.cc"
)

# find_package(
#     Doxygen
#     OPTIONAL_COMPONENTS dot mscgen dia
# )

# if (${DOXYGEN_FOUND})

# doxygen_add_docs(
#     doxygen
#     ${PROJECT_FILES}
#     USE_STAMP_FILE
# )

# endif()

add_library(
    simple_http
    STATIC
    ${PROJECT_FILES}
)

target_compile_features(simple_http PUBLIC cxx_std_20)

target_include_directories(simple_http INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
