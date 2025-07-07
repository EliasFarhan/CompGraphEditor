add_executable(wasm_main wasm_main.cpp)
target_link_libraries(wasm_main CommonWasm)
copydata(${CMAKE_CURRENT_SOURCE_DIR} wasm_main)