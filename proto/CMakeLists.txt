find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(PROTOBUF REQUIRED protobuf)

# Generate C++ protobuf and gRPC files
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
    COMMENT "Generating C++ protobuf and gRPC files"
)

# Generate Python protobuf and gRPC files
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/core_pb2.py"
           "${CMAKE_CURRENT_BINARY_DIR}/core_pb2_grpc.py"
    COMMAND python3 -m grpc_tools.protoc
    ARGS -I ${CMAKE_CURRENT_SOURCE_DIR}
         --python_out=${CMAKE_CURRENT_BINARY_DIR}
         --grpc_python_out=${CMAKE_CURRENT_BINARY_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/core.proto
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/core.proto
    COMMENT "Generating Python protobuf and gRPC files"
)

# Create a library for the generated C++ files
add_library(proto_grpc
    ${CMAKE_CURRENT_BINARY_DIR}/core.pb.cc
    ${CMAKE_CURRENT_BINARY_DIR}/core.grpc.pb.cc
)

target_include_directories(proto_grpc 
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ${GRPC_INCLUDE_DIRS}
    PUBLIC ${PROTOBUF_INCLUDE_DIRS}
)

target_link_libraries(proto_grpc 
    PUBLIC ${GRPC_LIBRARIES}
    PUBLIC ${PROTOBUF_LIBRARIES}
)

target_link_directories(proto_grpc
    PUBLIC ${GRPC_LIBRARY_DIRS}
    PUBLIC ${PROTOBUF_LIBRARY_DIRS}
)

# Create a custom target for Python files
add_custom_target(proto_python ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/core_pb2.py"
            "${CMAKE_CURRENT_BINARY_DIR}/core_pb2_grpc.py"
    COMMENT "Building Python protobuf files"
)

# Copy Python files to shared generated directory for use by all services
add_custom_command(TARGET proto_python POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/core_pb2.py"
        "${CMAKE_SOURCE_DIR}/generated/python/"
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/core_pb2_grpc.py"
        "${CMAKE_SOURCE_DIR}/generated/python/"
    COMMENT "Copying Python protobuf files to shared generated directory"
)
