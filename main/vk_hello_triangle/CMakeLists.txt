
file(GLOB_RECURSE TRIANGLE_SRC src/*.cpp include/*.h data/*.py data/*.vert data/*.frag)
add_executable(vk_triangle ${TRIANGLE_SRC})
copydata(${CMAKE_CURRENT_SOURCE_DIR} vk_triangle)
#checkshaders(${CMAKE_CURRENT_SOURCE_DIR} vk_triangle)

target_include_directories(vk_triangle PRIVATE include/)
target_link_libraries(vk_triangle PRIVATE Core CommonVK)
set_target_properties(vk_triangle PROPERTIES UNITY_BUILD ON)