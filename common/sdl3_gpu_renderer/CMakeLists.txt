MESSAGE(NOVUS_COMMON)

file(GLOB_RECURSE COMMON_NOVUS_FILES src/*.cpp include/*.h)
add_library(CommonNovus STATIC ${COMMON_GL_FILES})
target_include_directories(CommonNovus PUBLIC include/)
set_target_properties (CommonNovus PROPERTIES FOLDER Core)
target_precompile_headers(CommonNovus REUSE_FROM Core)