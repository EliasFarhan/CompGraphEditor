
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

file(GLOB_RECURSE EDITOR_SRC src/*.cpp include/*.h)
add_executable(editor ${EDITOR_SRC} ${SCRIPT_FILES})

target_include_directories(editor PRIVATE include/)
target_link_libraries(editor PRIVATE CommonGL nlohmann_json::nlohmann_json)

add_dependencies(editor editor_scripts)
#set_target_properties(editor PROPERTIES UNITY_BUILD ON)