add_library(state
    src/expiry_state.cpp
    src/symbol_state.cpp
)

target_include_directories(state PUBLIC include)

target_link_libraries(state PUBLIC common engine)