find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

# Generate protobuf and gRPC files
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/core.pb.cc"
           "${CMAKE_CURRENT_BINARY_DIR}/core.pb.h"
           "${CMAKE_CURRENT_BINARY_DIR}/core.grpc.pb.cc"
           "${CMAKE_CURRENT_BINARY_DIR}/core.grpc.pb.h"
    COMMAND /opt/homebrew/bin/protoc
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
         --plugin=protoc-gen-grpc=/opt/homebrew/bin/grpc_cpp_plugin
         -I ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/core.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/core.proto
)

add_library(core_grpc
    src/grpc_server.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/core.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/core.grpc.pb.cc
)

target_include_directories(core_grpc 
    PUBLIC include
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PRIVATE ${GRPC_INCLUDE_DIRS}
    PRIVATE ${PROTOBUF_INCLUDE_DIRS}
)

target_link_libraries(core_grpc 
    PUBLIC common
    PUBLIC ${GRPC_LIBRARIES}
    PUBLIC ${PROTOBUF_LIBRARIES}
)

target_link_directories(core_grpc
    PUBLIC ${GRPC_LIBRARY_DIRS}
    PUBLIC ${PROTOBUF_LIBRARY_DIRS}
)