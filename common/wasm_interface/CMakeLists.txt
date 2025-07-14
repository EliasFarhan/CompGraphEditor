add_library(CommonWasm STATIC src/wasm_interface.cpp src/wasm/neko2.cpp include/wasm_interface.h)
target_include_directories(CommonWasm PUBLIC include/)

target_link_libraries(CommonWasm PUBLIC Core m3 wasm3_cpp uv_a uvwasi_a)
set_target_properties (CommonWasm PROPERTIES FOLDER Core)