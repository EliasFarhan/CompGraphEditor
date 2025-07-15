
add_subdirectory(imnodes)
add_subdirectory(pybind11)
add_subdirectory(NekoLib)
MESSAGE("UVWASI SOURCE: ${uvwasi_SOURCE_DIR}/include")
add_subdirectory(wasm3)
add_subdirectory(wasm3/platforms/cpp/wasm3_cpp)
set(KTX_FEATURE_TOOLS ON)
add_subdirectory(KTX-Software)