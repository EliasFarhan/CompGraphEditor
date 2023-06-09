cmake_minimum_required (VERSION 3.18)
project(Neko2)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set_property( GLOBAL PROPERTY USE_FOLDERS ON)

include(cmake/data.cmake)
include(cmake/proto.cmake)


find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")
find_package(protobuf CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(argh CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)

include_directories(external/include)
option(ENABLE_PROFILER "Enable Tracy Profiling" OFF)


add_subdirectory(core/)
add_subdirectory(common/)
add_subdirectory(external)
add_subdirectory(main/)