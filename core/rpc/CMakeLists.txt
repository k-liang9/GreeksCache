add_library(core_grpc
    src/grpc_server.cpp
)

target_include_directories(core_grpc 
    PUBLIC include
)

target_link_libraries(core_grpc 
    PUBLIC common
    PUBLIC proto_grpc
)