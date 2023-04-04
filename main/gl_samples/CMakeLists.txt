
file(GLOB_RECURSE TRIANGLE_SRC src/*.cpp include/*.h data/*.py data/*.vert data/*.frag)
add_executable(gl_samples ${TRIANGLE_SRC})
copydata(${CMAKE_CURRENT_SOURCE_DIR} gl_samples)
checkshaders(${CMAKE_CURRENT_SOURCE_DIR} gl_samples)

target_include_directories(gl_samples PRIVATE include/)
target_link_libraries(gl_samples PRIVATE Core CommonGL)
set_target_properties(gl_samples PROPERTIES UNITY_BUILD ON)
set_target_properties (gl_samples PROPERTIES FOLDER Main)
set_target_properties (gl_samples_DATA PROPERTIES FOLDER Main)
set_target_properties (gl_samples_ShadersCheck PROPERTIES FOLDER Main)