cmake_minimum_required(VERSION 3.14.0)

project(spa_test
    VERSION 0.1.0
)

add_executable(
    spa_test
    "src/main.cc"
)

target_compile_features(spa_test PUBLIC cxx_std_20)

target_link_libraries(spa_test PUBLIC simple_http)

add_custom_command(
    TARGET spa_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -Rf
            "$<TARGET_FILE_DIR:spa_test>/www"
)

add_custom_command(
    TARGET spa_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/www" "$<TARGET_FILE_DIR:spa_test>/www"
)
