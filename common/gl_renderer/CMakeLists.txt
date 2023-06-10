MESSAGE(GL_COMMON)
find_package(GLEW REQUIRED)

file(GLOB_RECURSE COMMON_GL_FILES src/*.cpp include/*.h)
add_library(CommonGL STATIC ${COMMON_GL_FILES})
target_include_directories(CommonGL PUBLIC include/)
target_link_libraries(CommonGL PUBLIC Core GLEW::GLEW)
set_target_properties(CommonGL PROPERTIES UNITY_BUILD ON)

set_target_properties (CommonGL PROPERTIES FOLDER Core)
target_precompile_headers(CommonGL REUSE_FROM Core)
