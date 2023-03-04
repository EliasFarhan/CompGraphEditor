
function(generate_proto lib_name)

set(proto_lib_name "${lib_name}Proto")
file(GLOB_RECURSE PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")
add_library("${proto_lib_name}" STATIC ${PROTO_FILES})
protobuf_generate(TARGET "${proto_lib_name}" LANGUAGE cpp)
target_include_directories("${proto_lib_name}" PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries("${proto_lib_name}" PUBLIC protobuf::libprotobuf-lite protobuf::libprotoc protobuf::libprotobuf)
target_link_libraries("${lib_name}" PUBLIC "${proto_lib_name}")

endfunction()