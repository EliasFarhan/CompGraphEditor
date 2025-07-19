
add_subdirectory(imnodes)
add_subdirectory(pybind11)
add_subdirectory(NekoLib)
add_subdirectory(wasm3)
add_subdirectory(wasm3/platforms/cpp/wasm3_cpp)
include_directories(include)

execute_process(COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/build_externals.py ${CMAKE_CURRENT_SOURCE_DIR})


add_library(ktx STATIC IMPORTED GLOBAL)
target_compile_definitions(ktx INTERFACE KHRONOS_STATIC)
add_library(ktx_read STATIC IMPORTED GLOBAL)
target_include_directories(ktx INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/include")
if(MSVC)
    set_target_properties(ktx PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-dbg/Debug/ktx.lib"
            IMPORTED_LOCATION_RELASE "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-rel/Release/ktx.lib"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
    set_target_properties(ktx_read PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-dbg/Debug/ktx_read.lib"
            IMPORTED_LOCATION_RELASE "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-rel/Release/ktx_read.lib"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
else()
    set_target_properties(ktx PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-dbg/libktx.a"
            IMPORTED_LOCATION_RELASE "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-rel/libktx.a"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
    set_target_properties(ktx_read PROPERTIES
            IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-dbg/libktx_read.a"
            IMPORTED_LOCATION_RELASE "${CMAKE_CURRENT_SOURCE_DIR}/KTX-Software/build-rel/libktx_read.a"
            IMPORTED_CONFIGURATIONS "Release;Debug"
    )
endif()
add_library(libktx INTERFACE IMPORTED GLOBAL)
set_target_properties(libktx PROPERTIES INTERFACE_LINK_LIBRARIES "ktx;ktx_read")