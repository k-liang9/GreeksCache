add_library(orchestrator
    src/state_orchestrator.cpp
)

cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED)

target_include_directories(orchestrator PUBLIC include ${Boost_INCLUDE_DIRS})

target_link_libraries(orchestrator PUBLIC state common registry publish)