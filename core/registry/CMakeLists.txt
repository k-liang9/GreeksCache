add_library(registry
    src/registry.cpp
)

cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED)

target_include_directories(registry PUBLIC include ${Boost_INCLUDE_DIRS})

target_link_libraries(registry PUBLIC common)