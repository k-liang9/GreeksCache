cmake_minimum_required(VERSION 3.26)
project(GreeksComputeCore)

# set(CMAKE_CXX_COMPILER g++-15)
set(CMAKE_CXX_STANDARD 23)

add_executable(main
    src/main.cpp
    src/bs_engine.cpp
    src/utils.cpp
    src/contracts.cpp
    src/market_data.cpp
    tests/testing_vars.cpp
    )

target_include_directories(main PUBLIC
    src
    inc
    tests
)
