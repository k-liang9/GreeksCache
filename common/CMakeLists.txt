add_library(common
    src/market_data.cpp
    src/utils.cpp
)

target_include_directories(common PUBLIC include)