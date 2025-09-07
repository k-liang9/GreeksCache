add_library(tests
    src/testing_vars.cpp
)

target_include_directories(tests PUBLIC include)

target_link_libraries(tests PRIVATE common)