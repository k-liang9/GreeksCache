add_library(runtime
    src/runtime.cpp
)

cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED)

target_include_directories(runtime PUBLIC include ${Boost_INCLUDE_DIRS})

target_link_libraries(runtime PUBLIC common orchestrator publish core_grpc ingest tests)