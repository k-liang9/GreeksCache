add_library(registry
    src/registry.cpp
)

target_include_directories(registry PUBLIC include)

target_link_libraries(registry PUBLIC common)