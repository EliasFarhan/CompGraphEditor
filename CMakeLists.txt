cmake_minimum_required (VERSION 3.18)
project(CompGraphEditor)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/data.cmake)
include(cmake/proto.cmake)

find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")
find_package(tinyobjloader CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(Ktx CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_subdirectory(external/pybind11)
option(ENABLE_PROFILER "Enable Tracy Profiling" OFF)
if(ENABLE_PROFILER)
    add_subdirectory(external/tracy)
endif()

add_subdirectory(core/)
add_subdirectory(common/)


add_subdirectory(main/)
