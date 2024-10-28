add_library(game_proto OBJECT 
    game.proto)

target_link_libraries(game_proto PUBLIC protobuf::libprotobuf)

set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/protobuf")

target_include_directories(game_proto PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")

protobuf_generate(
    TARGET game_proto
    IMPORT_DIRS "${CMAKE_CURRENT_LIST_DIR}"
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")
