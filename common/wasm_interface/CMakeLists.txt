add_library(CommonWasm STATIC src/wasm_interface.cpp include/wasm_interface.h)
target_include_directories(CommonWasm PUBLIC include/)

target_link_libraries(CommonWasm PUBLIC Core m3 wasm3_cpp)
set_target_properties (CommonWasm PROPERTIES FOLDER Core)