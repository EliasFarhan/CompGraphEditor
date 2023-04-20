
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
target_link_libraries(editor PRIVATE CommonGL nlohmann_json::nlohmann_json CommonPy)

add_dependencies(editor editor_scripts)

set_target_properties (editor PROPERTIES FOLDER Main)
set_target_properties (editor_scripts PROPERTIES FOLDER Main)
#set_target_properties(editor PROPERTIES UNITY_BUILD ON)

#gl player

file(GLOB_RECURSE PLAYER_SRC src/gl_player/*.cpp include/gl_player/*.h)
find_package(PhysFS REQUIRED)
add_executable(gl_player ${PLAYER_SRC} main/gl_player_main.cpp)
target_include_directories(gl_player PRIVATE include/gl_player/ ${PHYSFS_INCLUDE_DIR})
target_link_libraries(gl_player PRIVATE CommonGL ${PHYSFS_LIBRARY}  CommonPy argh)

set_target_properties(gl_player PROPERTIES UNITY_BUILD ON)
set_target_properties (gl_player PROPERTIES FOLDER Main)