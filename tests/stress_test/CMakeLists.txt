cmake_minimum_required(VERSION 3.14.0)

project(stress_test
    VERSION 0.1.0
)

add_executable(
    stress_test
    "src/main.cc"
)

target_compile_features(stress_test PUBLIC cxx_std_20)

target_link_libraries(stress_test PUBLIC simple_http)
