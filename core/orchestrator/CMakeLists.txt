add_library(orchestrator
    src/state_orchestrator.cpp
)

target_include_directories(orchestrator PUBLIC include)

target_link_libraries(orchestrator PUBLIC state common registry publish)