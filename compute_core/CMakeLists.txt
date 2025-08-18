cmake_minimum_required(VERSION 3.26)
project(GreeksComputeCore)

set(CMAKE_CXX_STANDARD 23)

add_executable(main
    src/main.cpp
    src/greeks.cpp
    tests/testing_vars.cpp
    )

target_include_directories(main PUBLIC
    src
    inc
    tests
)
