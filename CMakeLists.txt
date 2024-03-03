cmake_minimum_required(VERSION 3.14.0)

add_subdirectory("simple_http")

project(http_server
    VERSION 0.1.0
)

add_executable(
    http_server
    "src/main.cc"
)

target_compile_features(http_server PUBLIC cxx_std_20)

target_link_libraries(http_server PUBLIC simple_http)

add_custom_command(
    TARGET http_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -Rf
            "$<TARGET_FILE_DIR:http_server>/www"
)

add_custom_command(
    TARGET http_server POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/www" "$<TARGET_FILE_DIR:http_server>/www"
)
