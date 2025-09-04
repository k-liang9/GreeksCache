add_library(ingest
    src/gbm_sim.cpp
    src/mailbox.cpp
)

cmake_policy(SET CMP0167 NEW)
find_package(Boost REQUIRED)

target_include_directories(ingest PUBLIC include ${Boost_INCLUDE_DIRS})

target_link_libraries(ingest PUBLIC common)