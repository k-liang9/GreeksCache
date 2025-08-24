add_library(publish
    src/cache.cpp
)

target_include_directories(publish PUBLIC include)

target_link_libraries(publish PUBLIC common)