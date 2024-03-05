cmake_minimum_required(VERSION 3.14.0)

project(fancywork_test
    VERSION 0.1.0
)

add_executable(
    fancywork_test
    "src/main.cc"
)

target_compile_features(fancywork_test PUBLIC cxx_std_20)

target_link_libraries(fancywork_test PUBLIC simple_http)

add_custom_command(
    TARGET fancywork_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -Rf
            "$<TARGET_FILE_DIR:fancywork_test>/www"
)

add_custom_command(
    TARGET fancywork_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/www" "$<TARGET_FILE_DIR:fancywork_test>/www"
)
