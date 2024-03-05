
file(GLOB_RECURSE TRIANGLE_SRC src/*.cpp include/*.h data/*.py data/*.vert data/*.frag data/*.rgen data/*.rchit data/*.rmiss data/*.rahit data/*.rcall data/*.glsl)
add_executable(vk_samples ${TRIANGLE_SRC})
copydata(${CMAKE_CURRENT_SOURCE_DIR} vk_samples)
checkvkshaders(${CMAKE_CURRENT_SOURCE_DIR} vk_samples)

target_include_directories(vk_samples PRIVATE include/)
target_link_libraries(vk_samples PRIVATE Core CommonVK CommonPy argh)
set_target_properties (vk_samples PROPERTIES FOLDER Main)
set_target_properties (vk_samples_DATA PROPERTIES FOLDER Main)
set_target_properties (vk_samples_ShadersCheck PROPERTIES FOLDER Main)

target_precompile_headers(vk_samples PRIVATE 
        "<vk/engine.h>"
        "<py_interface.h>"
        "<argh.h>")