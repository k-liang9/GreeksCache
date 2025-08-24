add_executable(tests
    src/main.cpp
    src/testing_vars.cpp
)

target_include_directories(tests PUBLIC include)

target_link_libraries(tests PRIVATE state publish)

enable_testing()
add_test(NAME tests COMMAND tests)