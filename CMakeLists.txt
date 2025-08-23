cmake_minimum_required(VERSION 3.26)
project(GreeksComputeCore)

# set(CMAKE_CXX_COMPILER g++-15) # for experimental/simd
set(CMAKE_CXX_STANDARD 23)

add_subdirectory(common)
add_subdirectory(engine)
add_subdirectory(publish)
add_subdirectory(registry)
add_subdirectory(state)
add_subdirectory(tests)