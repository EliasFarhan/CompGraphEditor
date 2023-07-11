
file(GLOB_RECURSE SCRIPT_FILES
        "scripts/*.py"
         "shaders/*.vert"
         "shaders/*.frag"
         "shaders/*.geom"
         "shaders/*.comp"
        )

foreach(SCRIPT ${SCRIPT_FILES})
    get_filename_component(FILE_NAME ${SCRIPT} NAME)
    get_filename_component(PATH_NAME ${SCRIPT} DIRECTORY)
    get_filename_component(EXTENSION ${SCRIPT} EXT)
    file(RELATIVE_PATH PATH_NAME "${CMAKE_CURRENT_SOURCE_DIR}" ${PATH_NAME})
    #MESSAGE("Data PATH: ${PATH_NAME} NAME: ${FILE_NAME}")
    set(SCRIPT_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
    #MESSAGE("Data OUT PATH: ${DATA_OUTPUT}")
    add_custom_command(
            OUTPUT ${SCRIPT_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E copy
            ${SCRIPT}
            ${SCRIPT_OUTPUT}
            DEPENDS ${SCRIPT})
    list(APPEND SCRIPT_OUTPUT_FILES ${SCRIPT_OUTPUT})
endforeach(SCRIPT)

add_custom_target(editor_scripts
        DEPENDS ${SCRIPT_OUTPUT_FILES}
)


find_package(nlohmann_json CONFIG REQUIRED)

file(GLOB_RECURSE EDITOR_SRC src/editor/*.cpp include/editor/*.h)
add_executable(editor ${EDITOR_SRC} ${SCRIPT_FILES} main/editor_main.cpp)

target_include_directories(editor PRIVATE include/editor/)
target_link_libraries(editor PRIVATE CommonGL nlohmann_json::nlohmann_json CommonPy argh imnodes)

add_dependencies(editor editor_scripts)

set_target_properties (editor PROPERTIES FOLDER Main)
set_target_properties (editor_scripts PROPERTIES FOLDER Main)
set_target_properties(editor PROPERTIES UNITY_BUILD ON)
target_precompile_headers(editor PRIVATE 
        "<filesystem>" 
        "<fmt/format.h>"
        "<nlohmann/json.hpp>" 
        "<proto/renderer.pb.h>" 
        "<proto/editor.pb.h>"
        "<pybind11/pybind11.h>"
        "<gl/pipeline.h>"
        "<engine/engine.h>")

add_custom_command(TARGET editor PRE_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                $<TARGET_FILE:basisu>
                $<TARGET_FILE_DIR:editor>)
add_dependencies(editor basisu)



#gl player

file(GLOB_RECURSE GL_PLAYER_SRC src/gl_player/*.cpp include/gl_player/*.h)

add_executable(gl_player ${GL_PLAYER_SRC} main/gl_player_main.cpp)
target_include_directories(gl_player PRIVATE include/gl_player/)
target_link_libraries(gl_player PRIVATE CommonGL CommonPy argh PhysFilesystem)

set_target_properties(gl_player PROPERTIES UNITY_BUILD ON)
set_target_properties (gl_player PROPERTIES FOLDER Main)
add_dependencies(editor gl_player)
target_precompile_headers(gl_player PRIVATE 
        "<engine/engine.h>"
        "<py_interface.h>")

#vk player

file(GLOB_RECURSE VK_PLAYER_SRC src/vk_player/*.cpp include/vk_player/*.h)

add_executable(vk_player ${VK_PLAYER_SRC} main/vk_player_main.cpp)
target_include_directories(vk_player PRIVATE include/vk_player/)
target_link_libraries(vk_player PRIVATE CommonVK CommonPy argh PhysFilesystem)
set_target_properties(vk_player PROPERTIES UNITY_BUILD ON)
set_target_properties (vk_player PROPERTIES FOLDER Main)
target_precompile_headers(vk_player PRIVATE
        "<engine/engine.h>"
        "<py_interface.h>")


#vk shader analyzer
add_executable(vk_shader_analyze main/vk_shader_analyze.cpp)
target_link_libraries(vk_shader_analyze argh nlohmann_json::nlohmann_json CommonVK spirv-cross-core spirv-cross-reflect)
set_target_properties (vk_shader_analyze PROPERTIES FOLDER Main/Utils)
add_dependencies(editor vk_shader_analyze)

#add_executable(pbr_utils main/pbr_utils_main.cpp ${EDITOR_SRC})
#target_include_directories(pbr_utils PRIVATE include/editor)
#target_link_libraries(pbr_utils argh CommonGL CommonPy nlohmann_json::nlohmann_json CommonPy imnodes)
#set_target_properties (pbr_utils PROPERTIES FOLDER Main/Utils)
#set_target_properties(pbr_utils PROPERTIES UNITY_BUILD ON)