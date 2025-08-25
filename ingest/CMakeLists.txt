add_library(ingest
    src/gbm_sim.cpp
)

target_include_directories(ingest PUBLIC include)

target_link_libraries(ingest PUBLIC common)