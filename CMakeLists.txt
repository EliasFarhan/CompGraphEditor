cmake_minimum_required (VERSION 3.18)
project(GPR5300)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb.h")
find_package(tinyobjloader CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)

add_subdirectory(external/pybind11)

file(GLOB_RECURSE PROTO_FILES proto/*.proto)
add_library(CommonProto STATIC ${PROTO_FILES})
protobuf_generate(TARGET CommonProto LANGUAGE cpp)
target_include_directories(CommonProto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(CommonProto PUBLIC protobuf::libprotobuf-lite protobuf::libprotoc protobuf::libprotobuf)

file(GLOB_RECURSE COMMON_FILES src/*.cpp include/*.h)
add_library(Common STATIC ${COMMON_FILES})
target_include_directories(Common PUBLIC include/ ${STB_INCLUDE_DIRS} ${Python3_INCLUDE_DIRS})
target_link_libraries(Common PUBLIC CommonProto GLEW::GLEW glm::glm SDL2::SDL2 SDL2::SDL2main imgui::imgui
    tinyobjloader::tinyobjloader pybind11::embed ${Python3_LIBRARIES} spdlog::spdlog)
set_target_properties(Common PROPERTIES UNITY_BUILD ON)
if(MSVC)
    target_compile_definitions(Common PUBLIC "_USE_MATH_DEFINES")
    target_compile_options(Common PUBLIC /arch:AVX2 /Oi /GL /fp:fast)
    target_link_options(Common PUBLIC /LTCG)
else()

endif()

file(GLOB MAIN_FOLDERS main/*)
foreach(MAIN_FOLDER ${MAIN_FOLDERS})
    add_subdirectory(${MAIN_FOLDER})
endforeach()
