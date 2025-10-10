add_library(CommonWasm STATIC src/wasm_interface.cpp src/wasm/neko2.cpp include/wasm_interface.h)
target_include_directories(CommonWasm PUBLIC include/)

target_link_libraries(CommonWasm PUBLIC Core libwasm3)
set_target_properties (CommonWasm PROPERTIES FOLDER Core)