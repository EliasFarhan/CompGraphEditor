
file(GLOB_RECURSE TRIANGLE_SRC src/*.cpp include/*.h)
add_executable(hello_triangle ${TRIANGLE_SRC})
target_include_directories(hello_triangle PRIVATE include/)
target_link_libraries(hello_triangle PRIVATE Common)
set_target_properties(hello_triangle PROPERTIES UNITY_BUILD ON)