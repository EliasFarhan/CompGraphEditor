file(GLOB_RECURSE CORE_FILES src/*.cpp include/*.h)
add_library(Core STATIC ${CORE_FILES})
target_include_directories(Core PUBLIC include/ ${STB_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/)

set(FB_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/core_schemas/")
file(GLOB ENGINE_SCHEMAS idl/*.fbs)
flatbuffers_generate_headers(
        TARGET core_schemas
        SCHEMAS ${ENGINE_SCHEMAS}
        INCLUDE_PREFIX "generated/"
        FLAGS --gen-object-api
)

target_include_directories(Core PUBLIC ${FB_OUTDIR})

target_compile_definitions(Core PUBLIC GLM_ENABLE_EXPERIMENTAL)
target_link_libraries(Core PUBLIC libktx)
target_link_libraries(Core PUBLIC glm::glm SDL3::SDL3 imgui::imgui assimp flatbuffers::flatbuffers NekoCore)
add_dependencies(Core GENERATE_core_schemas)

if(ENABLE_PROFILING)
    target_link_libraries(Core PUBLIC Tracy::TracyClient)
    target_compile_definitions(Core PUBLIC TRACY_ENABLE=1)
endif(ENABLE_PROFILING)
set_target_properties (Core PROPERTIES FOLDER Core)

if(MSVC)
    target_compile_definitions(Core PUBLIC WIN32_LEAN_AND_MEAN)
    target_compile_options(Core PUBLIC /Oi /GL /fp:fast)
    target_link_options(Core PUBLIC /LTCG)
else()
    target_compile_options(Core PUBLIC -march=haswell -ffast-math -flto)
    target_link_options(Core PUBLIC -flto)
endif()

target_precompile_headers(Core PRIVATE "<array>" "<vector>" "<string>" "<filesystem>")
