add_executable(app
    main.cpp
)

target_link_libraries(app PRIVATE runtime)

add_dependencies(app core_pybind)