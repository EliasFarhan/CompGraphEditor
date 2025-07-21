
add_subdirectory(imnodes)
add_subdirectory(pybind11)
add_subdirectory(NekoLib)

execute_process(COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/build_externals.py ${CMAKE_CURRENT_SOURCE_DIR})


add_library(ktx STATIC IMPORTED GLOBAL)
target_compile_definitions(ktx INTERFACE KHRONOS_STATIC)
add_library(ktx_read STATIC IMPORTED GLOBAL)
target_include_directories(ktx INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/include")
if(MSVC)
    set_target_properties(ktx PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-dbg/Debug/ktx.lib"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-rel/Release/ktx.lib"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
    set_target_properties(ktx_read PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-dbg/Debug/ktx_read.lib"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-rel/Release/ktx_read.lib"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
else()
    set_target_properties(ktx PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-dbg/libktx.a"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-rel/libktx.a"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
    set_target_properties(ktx_read PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-dbg/libktx_read.a"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-rel/libktx_read.a"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
endif()
add_library(libktx INTERFACE IMPORTED GLOBAL)
set_target_properties(libktx PROPERTIES INTERFACE_LINK_LIBRARIES "ktx;ktx_read")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/wasm3/platforms/cpp/wasm3_cpp")
add_library(m3 STATIC IMPORTED GLOBAL)
target_include_directories(m3 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/source")
target_compile_definitions(m3 INTERFACE "d_m3HasUVWASI" "d_m3HasTracer")
add_library(uvwasi STATIC IMPORTED GLOBAL)
target_include_directories(uvwasi INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-dbg/_deps/uvwasi-src/include")
add_library(uv STATIC IMPORTED GLOBAL)
if(MSVC)
    set_target_properties(m3 PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-dbg/source/Debug/m3.lib"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-rel/source/Release/m3.lib"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
    set_target_properties(uvwasi PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-dbg/_deps/uvwasi-build/Debug/uvwasi_a.lib"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-rel/_deps/uvwasi-build/Release/uvwasi_a.lib"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
    set_target_properties(uv PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-dbg/_deps/libuv-build/Debug/uv_a.lib"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-rel/_deps/libuv-build/Release/uv_a.lib"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
    target_link_libraries(uv INTERFACE ws2_32 iphlpapi userenv)
else()
    set_target_properties(m3 PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-dbg/source/libm3.a"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-rel/source/libm3.a"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
    set_target_properties(uvwasi PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-dbg/_deps/uvwasi-build/libuvwasi_a.a"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-rel/_deps/uvwasi-build/libuvwasi_a.a"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
    set_target_properties(uv PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-dbg/_deps/libuv-build/libuv_a.a"
            IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/wasm3/build-rel/_deps/libuv-build/libuv_a.a"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
endif()
add_library(libwasm3 INTERFACE IMPORTED GLOBAL)
set_target_properties(libwasm3 PROPERTIES INTERFACE_LINK_LIBRARIES "m3;uvwasi;uv;wasm3_cpp")