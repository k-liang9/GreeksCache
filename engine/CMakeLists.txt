add_library(engine
    src/engine.cpp
)

target_include_directories(engine PUBLIC include)

target_link_libraries(engine PUBLIC common)