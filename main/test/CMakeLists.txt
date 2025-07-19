add_executable(wasm_main wasm_main.cpp)
target_link_libraries(wasm_main CommonWasm)
copydata(${CMAKE_CURRENT_SOURCE_DIR} wasm_main)

add_executable(ktx_main ktx_main.cpp)
target_link_libraries(ktx_main PRIVATE libktx)