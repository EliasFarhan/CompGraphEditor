
file(GLOB_RECURSE TRIANGLE_SRC src/*.cpp include/*.h data/*.py data/*.vert data/*.frag)
add_executable(hello_triangle ${TRIANGLE_SRC})
copydata(${CMAKE_CURRENT_SOURCE_DIR} hello_triangle)
checkshaders(${CMAKE_CURRENT_SOURCE_DIR} hello_triangle)

target_include_directories(hello_triangle PRIVATE include/)
target_link_libraries(hello_triangle PRIVATE Common)
set_target_properties(hello_triangle PROPERTIES UNITY_BUILD ON)