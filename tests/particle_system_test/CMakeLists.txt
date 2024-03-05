cmake_minimum_required(VERSION 3.14.0)

project(particle_system_test
    VERSION 0.1.0
)

add_executable(
    particle_system_test
    "src/main.cc"
)

target_compile_features(particle_system_test PUBLIC cxx_std_20)

target_link_libraries(particle_system_test PUBLIC simple_http)

add_custom_command(
    TARGET particle_system_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -Rf
            "$<TARGET_FILE_DIR:particle_system_test>/www"
)

add_custom_command(
    TARGET particle_system_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/www" "$<TARGET_FILE_DIR:particle_system_test>/www"
)
