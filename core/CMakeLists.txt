file(GLOB_RECURSE CORE_FILES src/*.cpp include/*.h)
add_library(Core STATIC ${CORE_FILES})
target_include_directories(Core PUBLIC include/ ${STB_INCLUDE_DIRS} ${Python3_INCLUDE_DIRS})
generate_proto(Core)

set_target_properties (CoreProto PROPERTIES FOLDER Core)
target_link_libraries(Core PUBLIC glm::glm SDL2::SDL2 SDL2::SDL2main imgui::imgui 
    pybind11::embed ${Python3_LIBRARIES} spdlog::spdlog KTX::ktx assimp::assimp)

if(ENABLE_PROFILER)
    target_link_libraries(Core PUBLIC TracyClient)
    target_compile_definitions(Core PUBLIC TRACY_ENABLE=1)
endif()
set_target_properties (Core PROPERTIES FOLDER Core)

if(MSVC)
    target_compile_definitions(Core PUBLIC WIN32_LEAN_AND_MEAN)
    target_compile_options(Core PUBLIC /arch:AVX2 /Oi /GL /fp:fast)
    target_link_options(Core PUBLIC /LTCG)
else()
    target_compile_options(Core PUBLIC -march=haswell -ffast-math -flto)
    target_link_options(Core PUBLIC -flto)
endif()

target_precompile_headers(Core PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/proto/renderer.pb.h" "<array>" "<vector>" "<string>" "<filesystem>")