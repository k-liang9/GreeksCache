cmake_minimum_required(VERSION 3.26)
project(GreeksCache LANGUAGES CXX)

# set(CMAKE_CXX_COMPILER g++-15) # for experimental/simd
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)
enable_testing()

add_subdirectory(proto)
add_subdirectory(core)

# Build everything (C++ and Python) with a single build command
add_custom_target(build_all ALL
    DEPENDS app proto_python
    COMMENT "Building all C++ and Python components"
)