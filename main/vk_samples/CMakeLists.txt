
file(GLOB_RECURSE TRIANGLE_SRC src/*.cpp include/*.h data/*.py data/*.vert data/*.frag)
add_executable(vk_samples ${TRIANGLE_SRC})
copydata(${CMAKE_CURRENT_SOURCE_DIR} vk_samples)
checkvkshaders(${CMAKE_CURRENT_SOURCE_DIR} vk_samples)

target_include_directories(vk_samples PRIVATE include/)
target_link_libraries(vk_samples PRIVATE Core CommonVK)
set_target_properties(vk_samples PROPERTIES UNITY_BUILD ON)
set_target_properties (vk_samples PROPERTIES FOLDER Main)
set_target_properties (vk_samples_DATA PROPERTIES FOLDER Main)
set_target_properties (vk_samples_ShadersCheck PROPERTIES FOLDER Main)