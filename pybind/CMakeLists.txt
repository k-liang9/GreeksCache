find_package(pybind11 REQUIRED)

pybind11_add_module(core_pybind src/bindings.cpp)

target_include_directories(core_pybind PRIVATE include)

target_link_libraries(core_pybind PRIVATE orchestrator)

target_compile_definitions(core_pybind
                           PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})